#ifndef PARETO_FRONT_H
#define PARETO_FRONT_H

#include <vector>

class ParetoFront {
 public:
  struct ParetoPair {
    int h;
    int d;
    ParetoPair(int h, int d)
    : h(h), d(d) {
    }

    bool operator < (const ParetoPair& other) const
    {
      return h < other.h || (h == other.h && d < other.d);
    }
  };
  
 private:
  std::vector< ParetoPair > pareto_front;
  bool sorted;
  
public:
  ParetoFront();
  ~ParetoFront() = default;

  // For efficiency, pairs should be inserted in decreasing order of d.
  // Otherwise, sort MUST be called prior to using the getters.
  void insert_pair(ParetoPair p);
  void insert_pair(int h, int d);
  

  void sort();

  // Returns the (h, d) which minimises d.
  // Note that the pareto front may have been constructed with a cost-bound,
  // in which case the actual (h, d*) pair may have never been inserted.
  ParetoPair get_min_d_pair() const;
  
  // Returns the pair which minimises d subject to g + h <= bound
  ParetoPair get_min_d_pair(const int g, const int bound) const;
  
  // Returns h*
  ParetoPair get_min_h_pair() const;

  // Returns the pair which minimises (g + h) * d, subject to g + h <= bound
  ParetoPair get_min_f_weighted_pair(const int g, const int bound) const;

  bool empty() const {
    return pareto_front.empty();
  }

  bool is_sorted() const {
    return sorted;
  }
  
};


#endif
