#include "pts_evaluator.h"

#include "../option_parser.h"
#include "../plugin.h"

#include <cassert>
#include <limits>
#include <cmath>
#include <algorithm>

using namespace std;

namespace pts_evaluator {
PTSEvaluator::PTSEvaluator(const Options &opts)
    : CombiningEvaluator(opts.get_list<ScalarEvaluator *>("evals")) {
}

PTSEvaluator::PTSEvaluator(const vector<ScalarEvaluator *> &evals)
    : CombiningEvaluator(evals) {
}

PTSEvaluator::~PTSEvaluator() {
}

int PTSEvaluator::combine_values(const vector<int> &values) {
  int g = values[0];
  int h = values[1];
  int C = values[2];
  double potential = (1 - (h / (double)(C + 1 - g)));
  double pts = 1.0 / potential;
  if (pts == std::numeric_limits<double>::infinity())
    return std::numeric_limits<int>::max();
  
  return min(std::numeric_limits<int>::max() - 1, (int) round(pts));
}

static ScalarEvaluator *_parse(OptionParser &parser) {
    parser.document_synopsis("PTS evaluator",
                             "Calculates the potential. Sub-evals must be [g(), h, C]");

    parser.add_list_option<ScalarEvaluator *>("evals",
                                              "three scalar evaluators");
    Options opts = parser.parse();

    opts.verify_list_non_empty<ScalarEvaluator *>("evals");

    if (parser.dry_run())
        return 0;
    else
        return new PTSEvaluator(opts);
}

static Plugin<ScalarEvaluator> _plugin("pts", _parse);
}
