#ifndef DIJKSTRA_SEARCH_DIJKSTRA_SEARCH_H
#define DIJKSTRA_SEARCH_DIJKSTRA_SEARCH_H

#include "pareto_front.h"

#include <functional>
#include <vector>
#include <limits>
#include <map>

using namespace std;

class DijkstraSearch {
 public:
  struct Successor {
    size_t id;
    int transition_cost;
  Successor(const size_t id, const int transition_cost)
  : id(id), transition_cost(transition_cost) {}
  };
  
  enum Direction {
    FORWARD,
    BACKWARD
  };

  enum Algorithm {
    ORDINARY,
    PARETO
  };

  static const int INF = numeric_limits<int>::max();

 private:
  
  int bound;
  vector<size_t> queue_init[2];
  function<vector<Successor>(const size_t)> transition_func[2];
  vector<ParetoFront> pareto[2];

  bool inits[2];
  bool computed[2][2];

  void expand(size_t state,
	      ParetoFront::ParetoPair node_pair,
	      multimap<ParetoFront::ParetoPair, size_t>& lex_queue,
	      function<vector<Successor>(const size_t)>& transitions,
	      std::vector<ParetoFront>& pareto_fronts);
  void expand(size_t state,
	      ParetoFront::ParetoPair node_pair,
	      multimap<ParetoFront::ParetoPair, size_t>& lex_queue,
	      function<vector<Successor>(const size_t)>& transitions,
	      std::vector<ParetoFront>& pareto_fronts,
	      std::vector<ParetoFront>& oppoiste_pareto_fronts);

 public:
  
  DijkstraSearch(int bound = INF);
  
  bool is_bounded() const;
  bool is_init(Direction dir) const;
  bool is_computed(Direction dir, Algorithm alg) const;

  void init(Direction dir,
	    function<vector<Successor>(const size_t)> transition_func,
	    vector<size_t> queue_init,
	    size_t max_size);

  void compute(Direction dir, Algorithm alg);

  void clear(Direction dir);
 
  int get_value(Direction dir, const size_t state) const;
  ParetoFront& get_pareto_front(Direction dir, const size_t state);

  void set_values(Direction dir, vector<ParetoFront>& new_distances);
  
};

#endif
