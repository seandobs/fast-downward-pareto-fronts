#ifndef PDBS_CANONICAL_PDBS_H
#define PDBS_CANONICAL_PDBS_H

#include "types.h"
#include "../dijkstra_search/pareto_front.h"

#include <memory>
#include <algorithm>

class State;

namespace pdbs {
class CanonicalPDBs {
  static std::vector<int> expanded;
 
  std::shared_ptr<PDBCollection> pattern_databases;
  std::shared_ptr<MaxAdditivePDBSubsets> max_additive_subsets;

  bool pareto;
  std::string objective_name;
  std::string aggregate_name;

  std::function<double (std::vector<double>& values)> aggregate;
  std::function<double (const int h, const int d, const int g, const int bound, const int b)> objective;
  
  bool compute_b;
  
public:
    CanonicalPDBs(const std::shared_ptr<PDBCollection> &pattern_databases,
                  const std::shared_ptr<MaxAdditivePDBSubsets> &max_additive_subsets,
                  bool dominance_pruning,
		  bool pareto = false,
		  std::string objective_name = "h",
		  std::string aggregate = "max");
    ~CanonicalPDBs() = default;

    int get_value(const State &state) const;
    int get_value(const State &state, const int g,  const int bound, const int u) const;

};
}

#endif
