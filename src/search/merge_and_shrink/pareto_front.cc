#include "pareto_front.h"

#include <algorithm>
#include <cassert>

ParetoFront::ParetoFront()
  : sorted(true) {
}

void ParetoFront::sort() {
  std::sort(pareto_front.begin(), pareto_front.end());
  sorted = true;
}

void ParetoFront::insert_pair(ParetoPair p) {
  if(!pareto_front.empty() &&
     p < pareto_front.back()) {
    sorted = false;
  }
  pareto_front.push_back(p);
}

void ParetoFront::insert_pair(int h, int d) {
  insert_pair(ParetoPair(h, d));
}

ParetoFront::ParetoPair ParetoFront::get_min_d_pair() const {
  if(pareto_front.empty())
    return ParetoPair(-1,-1);

  assert(is_sorted());

  return pareto_front.back();
}

ParetoFront::ParetoPair ParetoFront::get_min_d_pair(const int g, const int bound) const {
  if(pareto_front.empty() ||
     g + get_min_h_pair().h > bound)
    return ParetoPair(-1,-1);
  
  assert(is_sorted());

  int i = pareto_front.size() - 1;
  while(i > 0 && g + pareto_front[i].h > bound)
    i--;
  return pareto_front[i];
}


ParetoFront::ParetoPair ParetoFront::get_min_h_pair() const {
  if(pareto_front.empty())
    return ParetoPair(-1,-1);
  
  assert(is_sorted());

  return pareto_front.front();
}


ParetoFront::ParetoPair ParetoFront::get_min_f_weighted_pair(const int g, const int bound) const {
  if(pareto_front.empty() ||
     g + get_min_h_pair().h > bound)
    return ParetoPair(-1,-1);
  
  assert(is_sorted());

  int min_i = 0;
  int min_fd = (g + pareto_front[min_i].h) * pareto_front[min_i].d;
  for(int i = 0;
      i < (int) pareto_front.size() &&
	g + pareto_front[i].h <= bound;
      i++) {
    int fd = (g + pareto_front[i].h) * pareto_front[i].d;
    if(fd < min_fd ||
       (fd == min_fd &&
	pareto_front[i].d < pareto_front[min_i].d)) {
      min_i = i;
      min_fd = fd;
    }
  }
  
  return pareto_front[min_i];
}
