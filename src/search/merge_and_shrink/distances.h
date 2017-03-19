#ifndef MERGE_AND_SHRINK_DISTANCES_H
#define MERGE_AND_SHRINK_DISTANCES_H

#include "types.h"
#include "../dijkstra_search/dijkstra_search.h"
#include "../dijkstra_search/pareto_front.h"

#include <vector>

/*
  TODO: Possible interface improvements for this class:

  - Rather than have compute_distances() return a vector<bool>,
    store it inside as an attribute (just like init_distances and
    goal_distances).
  - Check TODOs in implementation file.

  (Many of these would need performance tests, as distance computation
  can be one of the bottlenecks in our code.)

  TODO: Possible performance improvements:
  - Make the interface more fine-grained, so that it becomes possible
    to only compute the things that we actually want to use. For
    example, in many configurations we do not really care about g
    values (although we might still want to prune unreachable
    states... we might want to make this configurable).
  - Get rid of the vector<bool> of prunable states; this can be
    deduced from init_distances and goal_distances anyway. Not clear
    which of these options would be better.
  - We currently try to keep the distance information after shrinking
    (going quite far to avoid recomputation). Does this really serve a
    purpose? *After* shrinking, why are we interested in the distances
    in the first place? The main point of the distances is to inform
    the shrink strategies. (OK, I guess some merge strategies care
    about them, too -- but should *all* merge strategies pay for that?
    And if these merge strategies only care about h values, which we
    usually preserve, wouldn't it be better to invalidate g values
    then?)

    TODO: This class is functionally very similar to a pattern database.
              Should Distances and PatternDatabase inherit from a more
	      general class which implements the common functionality?
*/

namespace merge_and_shrink {
class TransitionSystem;
 
 class Distances {
    static const int DISTANCE_UNKNOWN = -1;

    const TransitionSystem &transition_system;

    DijkstraSearch dijkstra_search;

    int max_f;
    int max_g;
    int max_h;

    void clear_distances();
    void clear_pareto_fronts();
    
    size_t get_num_states() const;

    function<vector< DijkstraSearch::Successor>(const size_t state) >  get_successors() const;
    function<vector< DijkstraSearch::Successor>(const size_t state) >  get_predecessors() const;
    
public:
    explicit Distances(const TransitionSystem &transition_system, const int bound = INF);
    ~Distances();

    bool are_distances_computed() const;
    bool are_backward_pareto_fronts_computed() const;
    
    std::vector<bool> compute_distances(Verbosity verbosity);

    /*
      Update distances according to the given abstraction. If the abstraction
      is not f-preserving, distances are directly recomputed.

      It is OK for the abstraction to drop states, but then all
      dropped states must be unreachable or irrelevant. (Otherwise,
      the method might fail to detect that the distance information is
      out of date.)
    */
    void apply_abstraction(
        const StateEquivalenceRelation &state_equivalence_relation,
        Verbosity verbosity);

    int get_max_f() const { // used by shrink_fh
        return max_f;
    }
    int get_max_g() const { // unused
        return max_g;
    }
    int get_max_h() const { // used by shrink strategies
        return max_h;
    }
    int get_init_distance(int state) const { // used by shrink_fh
      return dijkstra_search.get_value(DijkstraSearch::FORWARD, state);
    }
    int get_goal_distance(int state) const { // used by shrink strategies and DFP
      return dijkstra_search.get_value(DijkstraSearch::BACKWARD, state);
    }

    void compute_backward_pareto_fronts(Verbosity verbosity);
    
    ParetoFront& get_backward_pareto_front(const size_t state);
    
    void dump() const;
    void statistics() const;
};
}

#endif
