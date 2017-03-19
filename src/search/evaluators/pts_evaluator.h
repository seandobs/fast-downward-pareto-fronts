#ifndef EVALUATORS_SUM_EVALUATOR_H
#define EVALUATORS_SUM_EVALUATOR_H

#include "combining_evaluator.h"

#include <vector>

namespace options {
class Options;
}

namespace pts_evaluator {
class PTSEvaluator : public combining_evaluator::CombiningEvaluator {
protected:
    virtual int combine_values(const std::vector<int> &values) override;
public:
    explicit PTSEvaluator(const options::Options &opts);
    explicit PTSEvaluator(const std::vector<ScalarEvaluator *> &evals);
    virtual ~PTSEvaluator() override;
};
}

#endif
