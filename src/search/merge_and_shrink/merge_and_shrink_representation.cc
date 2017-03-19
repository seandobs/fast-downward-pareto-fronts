#include "merge_and_shrink_representation.h"


#include "../dijkstra_search/pareto_front.h"
#include "distances.h"
#include "types.h"

#include "../task_proxy.h"

#include <algorithm>
#include <iostream>
#include <numeric>

using namespace std;

namespace merge_and_shrink {
  MergeAndShrinkRepresentation::MergeAndShrinkRepresentation(int domain_size)
    : domain_size(domain_size){
  }

  MergeAndShrinkRepresentation::~MergeAndShrinkRepresentation() {
  }

  int MergeAndShrinkRepresentation::get_domain_size() const {
    return domain_size;
  }

  MergeAndShrinkRepresentationLeaf::MergeAndShrinkRepresentationLeaf(
        int var_id, int domain_size)
    : MergeAndShrinkRepresentation(domain_size),
      var_id(var_id),
      lookup_table(domain_size) {
    iota(lookup_table.begin(), lookup_table.end(), 0);
  }

  void MergeAndShrinkRepresentationLeaf::set_distances(
        Distances &distances) {
    if(distances.are_backward_pareto_fronts_computed()) {
      pareto_fronts = vector< ParetoFront >(lookup_table.size());
      for(int i = 0; i < (int) lookup_table.size(); i++) {
	if(lookup_table[i] != PRUNED_STATE) {
	  pareto_fronts[i] = distances.get_backward_pareto_front(lookup_table[i]);
	  lookup_table[i] = distances.get_goal_distance(lookup_table[i]);
	}
      } 
    } else {
      for (int &entry : lookup_table) {
	if (entry != PRUNED_STATE) {
	  entry = distances.get_goal_distance(entry);
	}
      }
    }
  }

void MergeAndShrinkRepresentationLeaf::apply_abstraction_to_lookup_table(
        const vector<int> &abstraction_mapping) {
  int new_domain_size = 0;
  for (int &entry : lookup_table) {
    if (entry != PRUNED_STATE) {
      entry = abstraction_mapping[entry];
      new_domain_size = max(new_domain_size, entry + 1);
    }
  }
  domain_size = new_domain_size;
}

int MergeAndShrinkRepresentationLeaf::get_value(const State &state) const {
  int value = state[var_id].get_value();
  return lookup_table[value];
}

int MergeAndShrinkRepresentationLeaf::get_value(
        const State &state, const int g, const int bound) const {
  if(pareto_fronts.empty())
    return get_value(state);
  
  int value = state[var_id].get_value();

  ParetoFront pf = pareto_fronts[value];

  auto objective = [g, bound](const int h, const int d)
    {
      if(h > bound - g)
	return DijkstraSearch::INF;
      return d;
    };

  ParetoFront::ParetoPair p = pf.get_min_pair(objective);
  
  if(p.d == -1)
    return PRUNED_STATE;

  return objective(p.h, p.d);
}

void MergeAndShrinkRepresentationLeaf::dump() const {
  for (const auto &value : lookup_table) {
    cout << value << ", ";
  }
  cout << endl;
}


MergeAndShrinkRepresentationMerge::MergeAndShrinkRepresentationMerge(
        unique_ptr<MergeAndShrinkRepresentation> left_child_,
        unique_ptr<MergeAndShrinkRepresentation> right_child_)
  : MergeAndShrinkRepresentation(left_child_->get_domain_size() *
				 right_child_->get_domain_size()),
    left_child(move(left_child_)),
    right_child(move(right_child_)),
    lookup_table(left_child->get_domain_size(),
		 vector<int>(right_child->get_domain_size())) {
  int counter = 0;
  for (vector<int> &row : lookup_table) {
    for (int &entry : row) {
      entry = counter;
      ++counter;
    }
  }
}

void MergeAndShrinkRepresentationMerge::set_distances(
        Distances &distances) {
    if(distances.are_backward_pareto_fronts_computed()) {
      pareto_fronts =
	vector< vector< ParetoFront > >(
	   lookup_table.size(),
	   vector< ParetoFront >(lookup_table[0].size()));
      
      for(int i = 0; i < (int) lookup_table.size(); i++) {
	for(int j = 0; j < (int) lookup_table[i].size(); j++) {
	  if(lookup_table[i][j] != PRUNED_STATE) {
	    pareto_fronts[i][j] = distances.get_backward_pareto_front(lookup_table[i][j]);
	    lookup_table[i][j] = distances.get_goal_distance(lookup_table[i][j]);
	  }
	}
      } 
    } else {
      for (vector<int> &row : lookup_table) {
	for (int &entry : row) {
	  if (entry != PRUNED_STATE) {
	    entry = distances.get_goal_distance(entry);
	  }
	}
      }
    }
  }
  

void MergeAndShrinkRepresentationMerge::apply_abstraction_to_lookup_table(
        const vector<int> &abstraction_mapping) {
  int new_domain_size = 0;
  for (vector<int> &row : lookup_table) {
    for (int &entry : row) {
      if (entry != PRUNED_STATE) {
	entry = abstraction_mapping[entry];
	new_domain_size = max(new_domain_size, entry + 1);
      }
    }
  }
  domain_size = new_domain_size;
}

int MergeAndShrinkRepresentationMerge::get_value(const State &state) const {
  int state1 = left_child->get_value(state);
  int state2 = right_child->get_value(state);
  if (state1 == PRUNED_STATE ||
      state2 == PRUNED_STATE)
    return PRUNED_STATE;
  return lookup_table[state1][state2];
}

int MergeAndShrinkRepresentationMerge::get_value(
        const State &state, const int g, const int bound) const {

  if(pareto_fronts.empty()) {
    return get_value(state);
  }

  int state1 = left_child->get_value(state);
  int state2 = right_child->get_value(state);
  
  
  if (state1 == PRUNED_STATE ||
      state2 == PRUNED_STATE ||
      lookup_table[state1][state2] == PRUNED_STATE) {
    return PRUNED_STATE;
  }

  auto objective = [g, bound](const int h, const int d)
    {
      if(h > bound - g)
	return DijkstraSearch::INF;
      return d;
    };

  ParetoFront::ParetoPair p = pareto_fronts[state1][state2].get_min_pair(objective);
  
  if(p.d == -1)
    return PRUNED_STATE;

  return objective(p.h, p.d);
}

void MergeAndShrinkRepresentationMerge::dump() const {
  for (const auto &row : lookup_table) {
    for (const auto &value : row) {
      cout << value << ", ";
    }
    cout << endl;
  }
  cout << "dump left child:" << endl;
  left_child->dump();
  cout << "dump right child:" << endl;
  right_child->dump();
}
}
