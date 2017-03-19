#include "canonical_pdbs.h"

#include "dominance_pruning.h"
#include "pattern_database.h"


#include <cassert>
#include <iostream>
#include <limits>

using namespace std;

namespace pdbs {

  vector<int>  CanonicalPDBs::expanded = vector<int>();
  
  CanonicalPDBs::CanonicalPDBs(
			       const shared_ptr<PDBCollection> &pattern_databases,
			       const shared_ptr<MaxAdditivePDBSubsets> &max_additive_subsets_,
			       bool dominance_pruning,
			       bool pareto,
			       std::string objective_name,
			       std::string aggregate_name)
    : pattern_databases(pattern_databases),
      max_additive_subsets(max_additive_subsets_),
      pareto(pareto),
      objective_name(objective_name),
      aggregate_name(aggregate_name),
      compute_b(false){
  
    assert(max_additive_subsets);
    if (dominance_pruning) {
      max_additive_subsets = prune_dominated_subsets(
						     *pattern_databases, *max_additive_subsets);
    }

    if(objective_name.compare("d") == 0) {
      objective = [](const int h, const int d, const int g, const int bound, const int b) {
	return (double) d;
	(void)h;
	(void)g;
	(void)bound;
	(void)b;
      };
    } else if (objective_name.compare("ework") == 0) {
      objective = [](const int h, const int d, const int g, const int bound, const int b) {
	double potential =
	(h > bound - g) ? 0 :
	(h == 0) ? 1 :
	1 - (h / (double) (bound + 1 - g));
	
	return pow(d, b) / potential;
      };
      compute_b = true;
      
    } else if (objective_name.compare("h") == 0) {
      objective = [](const int h, const int d, const int g, const int bound, const int b) {
	return (double) h;
	(void)d;
	(void)g;
	(void)bound;
	(void)b;
      };
    } else if  (objective_name.compare("pts") == 0) {
      objective = [](const int h, const int d, const int g, const int bound, const int b) {
	double potential =
	(h > bound - g) ? 0 :
	(h == 0) ? 1 :
	1 - (h / (double) (bound + 1 - g));
	
	return 1 / potential;
	(void)d;
	(void)b;
      };
    } else {
      cout << "ERROR: " << objective_name << " is not a known objective function.";
      assert(false);
    }

    if(aggregate_name.compare("sum") == 0) {
      aggregate = [](const vector<double>& values) {
	double total = 0;
	for(auto v : values)
	  total += v;
	return total;
      };
    } else if (aggregate_name.compare("max") == 0) {
      aggregate = [](const vector<double>& values) {
	return *max_element(values.begin(), values.end());
      };
    } else if (aggregate_name.compare("min") == 0) {
      aggregate = [](const vector<double>& values) {
	return *min_element(values.begin(), values.end());
      };
    } else {
      cout << "ERROR: " << aggregate_name << " is not a known aggregator function.";
      assert(false);
    }
  }

  int CanonicalPDBs::get_value(const State &state) const {
    // If we have an empty collection, then max_additive_subsets = { \emptyset }.
    assert(!max_additive_subsets->empty());

    std::vector<double> values;
    for (const auto &subset : *max_additive_subsets) {
      int subset_h = 0;
      for (const shared_ptr<PatternDatabase> &pdb : subset) {
	/* Experiments showed that it is faster to recompute the
	   h values than to cache them in an unordered_map. */
	int h = pdb->get_value(state);
	if (h == numeric_limits<int>::max())
	  return numeric_limits<int>::max();
	subset_h += h;
      }
      values.push_back(subset_h);
    }
    
    // Aggregate and avoid integer overflows.
    // INF is reserved for out-of-bounds,
    // so all values exceeding that amount are returned as
    // INF - 1
    double agg_value = round(aggregate(values));
    return (agg_value < (double) DijkstraSearch::INF) ?
      (int) agg_value :
      DijkstraSearch::INF - 1;
  }

  int CanonicalPDBs::get_value(const State &state,
			       const int g, const int bound,
			       const int u) const {
    // If we have an empty collection, then max_additive_subsets = { \emptyset }.
    assert(!max_additive_subsets->empty());

    double b = 0;
    if(compute_b) {
      // Compute depth stats for the expected work heuristic
      while(expanded.size() <= (size_t) u)
	expanded.push_back(0);
      expanded[u]++;

      int max_u = expanded.size() - 2;

      int pre_jump = 0;
      for(int i = 0; i <= max_u; i++)
	pre_jump += expanded[i];
    
      b = (pre_jump < 10000) ? 1 : pow(pre_jump, 1.0 / max_u);
    }

    // Build the objective function with respect to the evaluation context
    auto obj =  [this, g, bound, b] (const int h, const int d)
      {
	return objective(h, d, g, bound, b);
      };
    
    // Compute the set of min objective values for each additive subset
    std::vector<double> values;
    for (const auto &subset : *max_additive_subsets) {
      if(subset.empty())
	continue;

      // Perform an additive summation of the pareto fronts
      ParetoFront subset_pf = subset[0]->get_backward_pareto_front(state);
      subset_pf.prune_with_bound(bound - g);
      if(subset_pf.empty())
	return DijkstraSearch::INF;
      for(size_t i = 1; i < subset.size(); i++) {
	subset_pf.merge_additive(subset[i]->get_backward_pareto_front(state),
				 bound - g);
	if(subset_pf.empty())
	  return DijkstraSearch::INF;
      }
      
      ParetoFront::ParetoPair min_pair = subset_pf.get_min_pair(obj);
      const double min_objective = obj(min_pair.h, min_pair.d);

      // Immediately prune on an infinite objective value
      if(min_objective == std::numeric_limits<double>::infinity())
	return DijkstraSearch::INF;

      values.push_back(min_objective);
    }

    // Aggregate and avoid integer overflows.
    // INF is reserved for pruning nodes,
    // but all nodes that reach this point are valid.
    // INF = int max, so all values geq to INF are returned as
    // INF - 1
    double agg_value = round(aggregate(values));
    return (agg_value < (double) DijkstraSearch::INF) ?
      (int) agg_value :
      DijkstraSearch::INF - 1;
  }
}
