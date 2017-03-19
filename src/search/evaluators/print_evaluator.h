#ifndef EVALUATORS_PRINT_EVALUATOR_H
#define EVALUATORS_PRINT_EVALUATOR_H

#include "combining_evaluator.h"
#include <iostream>
#include <fstream>

#include <vector>

namespace options {
class Options;
}

namespace print_evaluator {
class PrintEvaluator : public combining_evaluator::CombiningEvaluator {
  std::ofstream file;
protected:
    virtual int combine_values(const std::vector<int> &values) override;
public:
    explicit PrintEvaluator(const options::Options &opts);
    explicit PrintEvaluator(const std::vector<ScalarEvaluator *> &evals,
			    const std::string filename);
    virtual ~PrintEvaluator() override;
};
}

#endif
