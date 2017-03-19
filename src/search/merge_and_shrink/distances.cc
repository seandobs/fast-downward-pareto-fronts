#include "distances.h"

#include "label_equivalence_relation.h"
#include "transition_system.h"

#include "../priority_queue.h"
#include "../dijkstra_search/dijkstra_search.h"
#include "../dijkstra_search/pareto_front.h"

#include <cassert>
#include <deque>
#include <functional>

using namespace std;

namespace merge_and_shrink {
  const int Distances::DISTANCE_UNKNOWN;
  
  Distances::Distances(const TransitionSystem &transition_system, const int bound)
    : transition_system(transition_system),
      dijkstra_search(DijkstraSearch(bound)) {
  }

  Distances::~Distances() {}
  
  void Distances::clear_distances() {
    max_f = DISTANCE_UNKNOWN;
    max_g = DISTANCE_UNKNOWN;
    max_h = DISTANCE_UNKNOWN;
    dijkstra_search.clear(DijkstraSearch::FORWARD);
    dijkstra_search.clear(DijkstraSearch::BACKWARD);
  }

  void Distances::clear_pareto_fronts() {
    dijkstra_search.clear(DijkstraSearch::FORWARD);
    dijkstra_search.clear(DijkstraSearch::BACKWARD);
  }

  size_t Distances::get_num_states() const {
    return transition_system.get_size();
  }

  function<vector< DijkstraSearch::Successor>(const size_t state) > Distances::get_successors() const {
    vector<vector<DijkstraSearch::Successor>> forward_graph(get_num_states());
    for (const GroupAndTransitions &gat : transition_system) {
        const LabelGroup &label_group = gat.label_group;
        const vector<Transition> &transitions = gat.transitions;
        int cost = label_group.get_cost();
        for (const Transition &transition : transitions) {
             forward_graph[transition.src].push_back(
                DijkstraSearch::Successor(transition.target, cost));
        }
    }
    return [forward_graph] (const size_t state) {
      return forward_graph[state];
    };
  }
  function<vector< DijkstraSearch::Successor>(const size_t state) > Distances::get_predecessors() const {
    vector<vector<DijkstraSearch::Successor>> backward_graph(get_num_states());
    for (const GroupAndTransitions &gat : transition_system) {
        const LabelGroup &label_group = gat.label_group;
        const vector<Transition> &transitions = gat.transitions;
        int cost = label_group.get_cost();
        for (const Transition &transition : transitions) {
	     backward_graph[transition.target].push_back(
                DijkstraSearch::Successor(transition.src, cost));
        }
    }
    return [backward_graph] (const size_t state) {
      return backward_graph[state];
    };
  }

  ParetoFront&  Distances::get_backward_pareto_front(const size_t state) {
    return dijkstra_search.get_pareto_front(DijkstraSearch::BACKWARD, state);
  }

  bool Distances::are_distances_computed() const {
    return dijkstra_search.is_computed(DijkstraSearch::FORWARD, DijkstraSearch::ORDINARY) &&
      dijkstra_search.is_computed(DijkstraSearch::BACKWARD, DijkstraSearch::ORDINARY);
  }

  bool Distances::are_backward_pareto_fronts_computed() const {
    return dijkstra_search.is_computed(DijkstraSearch::BACKWARD, DijkstraSearch::PARETO);
  }

  void Distances::compute_backward_pareto_fronts(Verbosity verbosity) {
    if (verbosity >= Verbosity::VERBOSE) {
      cout << transition_system.tag();
    }
    assert(!are_pareto_fronts_computed());
    
    vector<size_t> backward_init;
    for (size_t state = 0; state < get_num_states(); ++state) {
      if (transition_system.is_goal_state(state)) {
	backward_init.push_back(state);
        }
    }

    dijkstra_search.init(DijkstraSearch::BACKWARD, get_predecessors(), backward_init, get_num_states());

    dijkstra_search.compute(DijkstraSearch::BACKWARD, DijkstraSearch::PARETO);

    assert(are_pareto_fronts_computed());
  }

  vector<bool> Distances::compute_distances(Verbosity verbosity) {
    /*
      This method does the following:
      - Computes the distances of abstract states from the abstract
      initial state ("abstract g") and from the abstract goal states
      ("abstract h").
      - Set max_f, max_g and max_h.
      - Return a vector<bool> that indicates which states can be pruned
      because they are unreachable (abstract g is infinite) or
      irrelevant (abstract h is infinite).
    */

    if (verbosity >= Verbosity::VERBOSE) {
      cout << transition_system.tag();
    }
    assert(!are_distances_computed());

    size_t num_states = get_num_states();

    if (num_states == 0) {
      if (verbosity >= Verbosity::VERBOSE) {
	cout << "empty transition system, no distances to compute" << endl;
      }
      max_f = max_g = max_h = INF;
      return vector<bool>();
    }
    
    vector<size_t> forward_init;
    forward_init.push_back(transition_system.get_init_state());

    vector<size_t> backward_init;
    for (size_t state = 0; state < get_num_states(); ++state) {
      if (transition_system.is_goal_state(state)) {
	backward_init.push_back(state);
        }
    }

    dijkstra_search.init(DijkstraSearch::FORWARD, get_successors(), forward_init, get_num_states());
    dijkstra_search.init(DijkstraSearch::BACKWARD, get_predecessors(), backward_init, get_num_states());
    
    dijkstra_search.compute(DijkstraSearch::FORWARD, DijkstraSearch::ORDINARY);
    dijkstra_search.compute(DijkstraSearch::BACKWARD, DijkstraSearch::ORDINARY);

    max_f = 0;
    max_g = 0;
    max_h = 0;

    int unreachable_count = 0, irrelevant_count = 0;
    vector<bool> prunable_states(num_states, false);
    for (size_t state = 0; state < num_states; ++state) {
      int g = dijkstra_search.get_value(DijkstraSearch::FORWARD, state);
      int h = dijkstra_search.get_value(DijkstraSearch::BACKWARD, state);
      
      // States that are both unreachable and irrelevant are counted
      // as unreachable, not irrelevant. (Doesn't really matter, of
      // course.)
      if (g == INF) {
	++unreachable_count;
	prunable_states[state] = true;
      } else if (h == INF) {
	++irrelevant_count;
	prunable_states[state] = true;
      } else {
	max_f = max(max_f, g + h);
	max_g = max(max_g, g);
	max_h = max(max_h, h);
      }
    }
    if (verbosity >= Verbosity::VERBOSE &&
	(unreachable_count || irrelevant_count)) {
      cout << transition_system.tag()
	   << "unreachable: " << unreachable_count << " states, "
	   << "irrelevant: " << irrelevant_count << " states" << endl;
    }
    assert(are_distances_computed());
    return prunable_states;
  }

  void Distances::apply_abstraction(
        const StateEquivalenceRelation &state_equivalence_relation,
	Verbosity verbosity) {
    assert(are_distances_computed());

    int new_num_states = state_equivalence_relation.size();
    vector<ParetoFront> new_init_distances(new_num_states);
    vector<ParetoFront> new_goal_distances(new_num_states);

    bool must_recompute = false;
    for (int new_state = 0; new_state < new_num_states; ++new_state) {
      const StateEquivalenceClass &state_equivalence_class =
	state_equivalence_relation[new_state];
      assert(!state_equivalence_class.empty());
      
      size_t ref_state = *state_equivalence_class.begin();
      ParetoFront new_init_front = dijkstra_search.get_pareto_front(DijkstraSearch::FORWARD, ref_state);
      ParetoFront new_goal_front = dijkstra_search.get_pareto_front(DijkstraSearch::BACKWARD, ref_state);
      
      for(size_t old_state : state_equivalence_class) {
	if(dijkstra_search.get_value(DijkstraSearch::FORWARD, old_state) != new_init_front.get_min_h_pair().h ||
	   dijkstra_search.get_value(DijkstraSearch::BACKWARD, old_state) != new_goal_front.get_min_h_pair().h) {
	  must_recompute = true;
	  break;
	}
      }

      if (must_recompute)
	break;

      new_init_distances[new_state] = new_init_front;
      new_goal_distances[new_state] = new_goal_front;
    }

    if (must_recompute) {
      if (verbosity >= Verbosity::VERBOSE) {
	cout << transition_system.tag()
	     << "simplification was not f-preserving!" << endl;
      }
      clear_distances();
      compute_distances(verbosity);
    } else {
      dijkstra_search.set_values(DijkstraSearch::FORWARD, new_init_distances);
      dijkstra_search.set_values(DijkstraSearch::BACKWARD, new_goal_distances);
    }
  }

  void Distances::dump() const {
    cout << "Distances: ";
    for (size_t state = 0; state < get_num_states(); ++state) {
      cout << state << ": " << dijkstra_search.get_value(DijkstraSearch::BACKWARD, state) << ", ";
    }
    cout << endl;
  }

  void Distances::statistics() const {
    cout << transition_system.tag();
    if (!are_distances_computed()) {
      cout << "distances not computed";
    } else if (transition_system.is_solvable()) {
      cout << "init h=" << dijkstra_search.get_value(DijkstraSearch::BACKWARD, transition_system.get_init_state())
	   << ", max f=" << get_max_f()
	   << ", max g=" << get_max_g()
	   << ", max h=" << get_max_h();
    } else {
      cout << "transition system is unsolvable";
    }
    cout << endl;
  }
}
