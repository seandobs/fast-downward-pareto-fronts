#include "print_evaluator.h"

#include "../option_parser.h"
#include "../plugin.h"

#include <cassert>
#include <limits>

using namespace std;

namespace print_evaluator {
PrintEvaluator::PrintEvaluator(const Options &opts)
  : CombiningEvaluator(opts.get_list<ScalarEvaluator *>("evals")),
    file(ofstream(opts.get<string>("file"), std::ios::out | std::ios::app)) {
}

PrintEvaluator::PrintEvaluator(const vector<ScalarEvaluator *> &evals,
			       const string filename)
  : CombiningEvaluator(evals),
    file(ofstream(filename, std::ios::out | std::ios::app)){
}

PrintEvaluator::~PrintEvaluator() {
}

int PrintEvaluator::combine_values(const vector<int> &values) {
    for (int value : values) {
      file << value << " ";
    }
    file << endl;
    return 0;
}

static ScalarEvaluator *_parse(OptionParser &parser) {
    parser.document_synopsis("Print evaluator",
                             "Prints the sub-evaluators.");

    parser.add_list_option<ScalarEvaluator *>("evals",
                                              "at least one scalar evaluator");
    parser.add_option<string>("file",
			      "print statistics file name");
    Options opts = parser.parse();

    opts.verify_list_non_empty<ScalarEvaluator *>("evals");

    if (parser.dry_run())
        return 0;
    else
        return new PrintEvaluator(opts);
}

static Plugin<ScalarEvaluator> _plugin("print", _parse);
}
