#ifndef DIJKSTRA_SEARCH_PARETO_FRONT_H
#define DIJKSTRA_SEARCH_PARETO_FRONT_H

//#include <map>
#include <functional>
#include <cmath>
#include <vector>
#include <list>
#include <utility>

class ParetoFront {
  
 public:
  struct ParetoPair {
    int h;
    int d;
    ParetoPair(int h, int d) : h(h), d(d) {};
    
    friend bool operator< (const ParetoPair& lhs, const ParetoPair& rhs) {
       return lhs.h < rhs.h || (lhs.h == rhs.h && lhs.d < rhs.d); // Lex compare
    }
    friend bool dominates (const ParetoPair& lhs, const ParetoPair& rhs) {
       return lhs.h <= rhs.h && lhs.d <= rhs.d &&
	 (lhs.h < rhs.h || lhs.d < rhs.d); // Pareto domination compare
    }
    friend bool operator== (const ParetoPair& lhs, const ParetoPair& rhs) {
       return lhs.h == rhs.h && lhs.d == rhs.d;
    }
    ParetoPair& operator+= (const ParetoPair& other) {
      h += other.h;
      d += other.d;
      return *this;
    }
    friend ParetoPair operator+ (ParetoPair lhs, const ParetoPair& rhs) {
      lhs += rhs;
      return lhs;
    }
    double potential(const int g, const int bound) const {
      return (h > bound - g) ?
	0 :
	(h == 0) ?
	1 :
        1 - (h / (double) (bound + 1 - g));
    }
    
  };
  
  
 private:
  std::vector<ParetoPair> pareto_front;

public:
  ParetoFront();

  ParetoFront(const std::vector<ParetoFront*>& pfs, const int bound);
  
  ~ParetoFront() = default;

  void prune_with_bound(const int bound);

  // Appends and returns true if pair is non-dominated by the min-d pair
  bool append_pair(const ParetoPair p);
  bool is_appendable(const ParetoPair p);

  // Returns the (h, d) which minimises d.
  // Note that the pareto front may have been constructed with a cost-bound,
  // in which case the actual (h, d*) pair may have never been inserted.
  ParetoPair get_min_d_pair() const;
  
  // Returns h*
  ParetoPair get_min_h_pair() const;

  // Returns the minimum pair according to the comparison function
  ParetoPair get_min_pair(const std::function<double(const int h, const int d)>& objective) const;

  bool empty() const {
    return pareto_front.empty();
  }

  void merge_additive(const ParetoFront& other, const int bound);

};


#endif
