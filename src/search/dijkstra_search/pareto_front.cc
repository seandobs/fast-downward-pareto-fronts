#include "pareto_front.h"

#include "dijkstra_search.h"

#include <algorithm>
#include <cassert>
#include <iostream>

ParetoFront::ParetoFront()
{}

ParetoFront::ParetoFront(const std::vector<ParetoFront*>& pfs, const int bound)
{
  pareto_front = pfs[0]->pareto_front;
  prune_with_bound(bound);
  
  for(size_t i = 1; i < pfs.size(); i++) {
    if(pareto_front.empty())
      return;
    
    const auto& other = *(pfs[i]);
    merge_additive(other, bound);
  }
}

void ParetoFront::prune_with_bound(const int bound) {
  for(auto p = pareto_front.begin();
      p != pareto_front.end();
      p++) {
    if(p->h > bound) {
      pareto_front.erase(p, pareto_front.end());
      return;
    }
  }
}

bool ParetoFront::append_pair(const ParetoPair p) {
  if(is_appendable(p)) {
    if(!pareto_front.empty() &&
       pareto_front.back().h == p.h) {
      pareto_front.back().d = p.d;
    } else {
      pareto_front.push_back(p);
    }
    return true;
  }
  return false;
}
bool ParetoFront::is_appendable(const ParetoPair p) {
  if (pareto_front.empty())
    return true;
  
  const ParetoPair min_d = get_min_d_pair();
  return min_d.d > p.d && min_d.h <= p.h;
}

ParetoFront::ParetoPair ParetoFront::get_min_d_pair() const {
  return (pareto_front.empty()) ?
    ParetoPair(DijkstraSearch::INF,DijkstraSearch::INF) :
    *(pareto_front.rbegin());
}

ParetoFront::ParetoPair ParetoFront::get_min_h_pair() const {
  return (pareto_front.empty()) ?
    ParetoPair(DijkstraSearch::INF,DijkstraSearch::INF) :
    *(pareto_front.begin());
}


ParetoFront::ParetoPair ParetoFront::get_min_pair(
        const std::function<double(const int h, const int d)>& objective) const
{
  auto min_pair = pareto_front.begin();
  double min_obj = objective(min_pair->h, min_pair->d);
  for(auto p = std::next(min_pair);
      p != pareto_front.end();
      p++) {
    double obj = objective(p->h, p->d);
    if(obj < min_obj) {
      min_obj = obj;
      min_pair = p;
    }
  }
  return *min_pair;
}

void ParetoFront::merge_additive(const ParetoFront& other, const int bound) {
  if(pareto_front.empty() || other.pareto_front.empty()) {
    pareto_front.clear();
    return;
  }

  // Optimised additive merge for the singleton fronts
  const ParetoPair min_h_pair = get_min_h_pair();
  if(pareto_front.size() == 1) {
    pareto_front.clear();
    for(auto p : other.pareto_front) {
      p += min_h_pair;
      if(p.h > bound) break;
      pareto_front.push_back(p);
    }
    return;
  }
  
  const ParetoPair other_min_h_pair = other.get_min_h_pair();
  if(other.pareto_front.size() == 1) {
    for(auto a = pareto_front.begin();
	a != pareto_front.end();
	a++) {
      *a += other_min_h_pair;
      if(a->h > bound) {
	pareto_front.erase(a, pareto_front.end());
	break;
      }
    }
    return;
  }

  //Perform bucketed additive merge 
  const int mind = get_min_d_pair().d + other.get_min_d_pair().d;
  const int maxd = get_min_h_pair().d + other_min_h_pair.d;
  const size_t minh_size = ((maxd - mind)+1);
  int *minh = (int *) alloca(sizeof(int) * minh_size);
  std::fill(minh, minh + minh_size, bound+1);

  int newh, newd;
  for(const auto a : pareto_front) {
    if(a.h + other_min_h_pair.h > bound) break;
    for(const auto b : other.pareto_front) {
      newh = a.h + b.h;
      newd = a.d + b.d;
      
      if(newh > bound) break;
      
      if(newh < minh[newd-mind])
  	minh[newd-mind] = newh;
    }
  }

  pareto_front.clear();
  int curminh = bound+1;
  for(size_t d = 0; d < minh_size; d++) {
    if(minh[d] < curminh) {
      curminh = minh[d];
      pareto_front.push_back(ParetoPair(minh[d], d+mind));
    }
  }
  std::reverse(pareto_front.begin(), pareto_front.end());
}
