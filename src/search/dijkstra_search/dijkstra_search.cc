#include "dijkstra_search.h"

#include "../priority_queue.h"

#include <cassert>


DijkstraSearch::DijkstraSearch(int bound)
  : bound(bound) {
  inits[FORWARD] = false;
  inits[BACKWARD] = false;

  computed[FORWARD][ORDINARY] = false;
  computed[BACKWARD][ORDINARY] = false;

  computed[FORWARD][PARETO] = false;
  computed[BACKWARD][PARETO] = false;
}
  
bool DijkstraSearch::is_bounded() const {
  return bound < INF;
}
bool DijkstraSearch::is_init(Direction dir) const {
  return inits[dir];
}
bool DijkstraSearch::is_computed(Direction dir, Algorithm alg) const {
  return computed[dir][alg];
}
void DijkstraSearch::init(Direction dir,
			  function<vector<Successor>(const size_t)> transition_func,
			  vector<size_t> queue_init,
			  size_t max_size) {
  this->transition_func[dir] = transition_func;
  this->queue_init[dir] = queue_init;
  inits[dir] = true;
  pareto[dir].resize(max_size);
}


void DijkstraSearch::compute(Direction dir, Algorithm alg) {
  assert(is_init(dir));
  if(is_computed(dir, alg))
    return;

  string dir_s = (dir == FORWARD) ? "Forward " : "Backward ";
  string alg_s = (alg == ORDINARY) ? "Ordinary " : "Pareto ";
  
  cout << "Performing "
       << dir_s
       << alg_s
       << "Dijkstra Search with Cost-Bound: "
       << bound << endl;

  // Inform the bounds with opposite direction cost if computed
  Direction opposite_dir = (dir == FORWARD) ? BACKWARD : FORWARD;
  bool inform = is_computed(opposite_dir, ORDINARY);
  
  auto& transitions = transition_func[dir];
  auto& pareto_fronts = pareto[dir];
  auto& opposite_pareto_fronts = pareto[opposite_dir];

  // Multimap ordered on pareto pair operator<
  // will act as a lexicographical queue with
  // O(log(n)) insertion and O(1) pop
  multimap<ParetoFront::ParetoPair, size_t> lex_queue;

  if(is_computed(dir, ORDINARY)) {
    // Reconstruct the lex_queue from the ordinary search
    // rather than re-exploring the cheapest nodes
    for(size_t i = 0; i < pareto_fronts.size(); i++) {
      if(!pareto_fronts[i].empty()) {
	const ParetoFront::ParetoPair node_pair = pareto_fronts[i].get_min_h_pair();
	const size_t state = i;
	if(inform) {
	  expand(state, node_pair,
		 lex_queue, transitions,
		 pareto_fronts, opposite_pareto_fronts);
	} else {
	  expand(state, node_pair,
		 lex_queue, transitions,
		 pareto_fronts);
	}
      }
    }
  } else {
    // Initialise the lex queue
    for(size_t init_state : queue_init[dir]) {
      lex_queue.emplace(ParetoFront::ParetoPair(0,0), init_state);
    }
  }
  
  while(!lex_queue.empty()) {
    auto pop = lex_queue.begin();
    const ParetoFront::ParetoPair node_pair = pop->first;
    const size_t state = pop->second;
    lex_queue.erase(pop);

    ParetoFront &state_frontier = pareto_fronts[state];
    if((state_frontier.empty() || alg == PARETO) &&
       state_frontier.append_pair(node_pair)) {
      if(inform) {
	expand(state, node_pair,
	       lex_queue, transitions,
	       pareto_fronts, opposite_pareto_fronts);
      } else {
	expand(state, node_pair,
	       lex_queue, transitions,
	       pareto_fronts);
      }
    }

    computed[dir][alg] = true;
    if(alg == PARETO)
      computed[dir][ORDINARY] = true;
  }
}

void DijkstraSearch::expand(size_t state,
			    ParetoFront::ParetoPair node_pair,
			    multimap<ParetoFront::ParetoPair, size_t>& lex_queue,
			    function<vector<Successor>(const size_t)>& transitions,
			    std::vector<ParetoFront>& pareto_fronts) {
  for(auto successor : transitions(state)) {
    const ParetoFront::ParetoPair successor_pair =
      ParetoFront::ParetoPair(node_pair.h + successor.transition_cost,
			      node_pair.d + 1);
    ParetoFront& successor_front = pareto_fronts[successor.id];
	
    // Only enque if the node is within the cost boundary and non-dominated
    if (successor_pair.h <= bound &&
	successor_front.is_appendable(successor_pair)) {
      lex_queue.emplace(successor_pair, successor.id);
    }
  }
}

void DijkstraSearch::expand(size_t state,
			    ParetoFront::ParetoPair node_pair,
			    multimap<ParetoFront::ParetoPair, size_t>& lex_queue,
			    function<vector<Successor>(const size_t)>& transitions,
			    std::vector<ParetoFront>& pareto_fronts,
			    std::vector<ParetoFront>& opposite_pareto_fronts) {
  for(auto successor : transitions(state)) {
    const ParetoFront::ParetoPair successor_pair =
      ParetoFront::ParetoPair(node_pair.h + successor.transition_cost,
			      node_pair.d + 1);
    ParetoFront& successor_front = pareto_fronts[successor.id];
    int opposite_cost = opposite_pareto_fronts[successor.id].get_min_h_pair().h;
	
    // Only enque if the node is within the cost boundary and non-dominated
    if (successor_pair.h <= bound - opposite_cost &&
	successor_front.is_appendable(successor_pair)) {
      lex_queue.emplace(successor_pair, successor.id);
    }
  }
}

void DijkstraSearch::clear(Direction dir) {
  pareto[dir].clear();

  computed[dir][ORDINARY] = false;
  computed[dir][PARETO] = false;
}

int DijkstraSearch::get_value(Direction dir, const size_t state) const {
  return pareto[dir].at(state).get_min_h_pair().h;
}
ParetoFront& DijkstraSearch::get_pareto_front(Direction dir, const size_t state) {
  return pareto[dir][state];
}


void DijkstraSearch::set_values(Direction dir, vector<ParetoFront>& new_distances) {
  clear(dir);
  for(size_t i = 0; i < new_distances.size(); i++) {
    if(!new_distances[i].empty()) {
      pareto[dir][i] = new_distances[i];
    }
  }
}
