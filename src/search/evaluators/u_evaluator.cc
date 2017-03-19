#include "u_evaluator.h"

#include "../evaluation_context.h"
#include "../evaluation_result.h"
#include "../option_parser.h"
#include "../plugin.h"

namespace u_evaluator {
EvaluationResult UEvaluator::compute_result(EvaluationContext &eval_context) {
    EvaluationResult result;
    result.set_h_value(eval_context.get_u_value());
    return result;
}

static ScalarEvaluator *_parse(OptionParser &parser) {
    parser.document_synopsis(
        "u-value evaluator",
        "Returns the u-value (depth) of the search node.");
    parser.parse();
    if (parser.dry_run())
        return 0;
    else
        return new UEvaluator;
}

static Plugin<ScalarEvaluator> _plugin("u", _parse);
}
