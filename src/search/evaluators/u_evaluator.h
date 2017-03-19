#ifndef EVALUATORS_U_EVALUATOR_H
#define EVALUATORS_U_EVALUATOR_H

#include "../scalar_evaluator.h"

class Heuristic;

namespace u_evaluator {
class UEvaluator : public ScalarEvaluator {
public:
    UEvaluator() = default;
    virtual ~UEvaluator() override = default;

    virtual EvaluationResult compute_result(
        EvaluationContext &eval_context) override;

    virtual void get_involved_heuristics(std::set<Heuristic *> &) override {}
};
}

#endif
