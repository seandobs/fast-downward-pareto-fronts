#ifndef HEURISTICS_CB_FF_HEURISTIC_H
#define HEURISTICS_CB_FF_HEURISTIC_H

#include "../priority_queue.h"
#include "additive_heuristic.h"

#include <vector>

namespace cb_ff_heuristic {
using Proposition = relaxation_heuristic::Proposition;
using UnaryOperator = relaxation_heuristic::UnaryOperator;

enum RelaxedPlanType {SHORT = 0,
		      CHEAP = 1,
		      SHORT_THEN_CHEAP = 2,
		      MAX_RELAXED_PLAN_TYPE};

enum ImprovementType {NONE = 0,
		      ONCE = 1,
		      TOP_DOWN = 2,
		      BOTTOM_UP = 3,
		      MAX_IMPROVEMENT_TYPE};

class CostBoundedFFHeuristic : public Heuristic
{
 public:
  static const unsigned int MAX_SLOTS = 2;

 private:

  static const int MAX_COST_VALUE = 100000000;
  static bool did_write_overflow_warning;
  void write_overflow_warning();

  void increase_cost(int& cost, int amount)
  {
    assert(cost >= 0);
    assert(amount >= 0);
    cost += amount;
    if (cost > MAX_COST_VALUE) {
      write_overflow_warning();
      cost = MAX_COST_VALUE;
    }
  };

  class MaxCombinator {
    bool unit_cost;
  public:
    MaxCombinator(bool _unit_cost) : unit_cost(_unit_cost) { };
    void update_op_cost(UnaryOperator& op, unsigned int slot, int pre_cost)
    {
      if (unit_cost)
	op.cost[slot] = std::max(op.cost[slot], pre_cost + 1);
      else
	op.cost[slot] = std::max(op.cost[slot], pre_cost + op.base_cost);
    };
  };

  class SumCombinator {
    CostBoundedFFHeuristic& h;
  public:
    SumCombinator(CostBoundedFFHeuristic& _h) : h(_h) { };
    void update_op_cost(UnaryOperator& op, unsigned int slot, int pre_cost)
    {
      h.increase_cost(op.cost[slot], pre_cost);
    };
  };

  void enqueue_if_necessary
    (Proposition *prop, unsigned int slot, unsigned int tb_slot,
     int cost, UnaryOperator *op)
  {
    assert(cost >= 0);
    if (prop->cost[slot] == -1 || prop->cost[slot] > cost) {
      prop->cost[slot] = cost;
      prop->reached_by[slot] = op;
      queue.push(cost, prop);
    }
    else if (tb_slot < MAX_SLOTS) { // tie-breaking
      if ((prop->cost[slot] == cost) &&
    	  (prop->cost[tb_slot] > op->cost[tb_slot])) {
    	prop->cost[slot] = cost;
    	prop->reached_by[slot] = op;
    	queue.push(cost, prop);
      }
    }
    assert(prop->cost[slot] != -1 && prop->cost[slot] <= cost);
  }

  AdaptiveQueue<Proposition *> queue;

  std::vector<UnaryOperator> unary_operators;
  std::vector<std::vector<Proposition> > propositions;
  std::vector<Proposition *> goal_propositions;

  void build_unary_operators(const Operator &op, int operator_no);
  void simplify();

  void setup_exploration_queue(unsigned int slot, const State &state, bool unit_cost);

  template<class CCC>
  void relaxed_exploration(unsigned int slot, unsigned int tb_slot, CCC& co)
  {
    int unsolved_goals = goal_propositions.size();
    while (!queue.empty()) {
      std::pair<int, Proposition *> top_pair = queue.pop();
      int distance = top_pair.first;
      Proposition *prop = top_pair.second;
      int prop_cost = prop->cost[slot];
      assert(prop_cost <= distance);
      if (prop_cost < distance)
	continue;
      if (prop->is_goal && --unsolved_goals == 0)
	return;
      const std::vector<UnaryOperator *> &triggered_operators =
	prop->precondition_of;
      for (int i = 0; i < triggered_operators.size(); i++) {
	UnaryOperator *unary_op = triggered_operators[i];
	unary_op->unsatisfied_preconditions--;
	co.update_op_cost(*unary_op, slot, prop_cost);
	assert(unary_op->unsatisfied_preconditions >= 0);
	if (unary_op->unsatisfied_preconditions == 0)
	  enqueue_if_necessary(unary_op->effect, slot, tb_slot,
			       unary_op->cost[slot], unary_op);
      }
    }
  }

  // Relaxed plans are represented as a set of operators implemented
  // as a bit vector.
  typedef std::vector<bool> RelaxedPlan;
  RelaxedPlan rp;

  RelaxedPlanType initial_plan_type;
  ImprovementType improvement_type;
  int penalty_factor;

  void unmark_propositions();
  void reset_switches();
  void extract_relaxed_plan
    (unsigned int slot, Proposition *goal, RelaxedPlan& relaxed_plan);
  int relaxed_plan_cost(RelaxedPlan& relaxed_plan);
  int relaxed_plan_size(RelaxedPlan& relaxed_plan);
  void collect_preferred_operators
    (const State &state, const RelaxedPlan& relaxed_plan);

 protected:
  virtual void initialize();
  virtual int compute_heuristic(const GlobalState &state);
  virtual int compute_heuristic(const GlobalState &state, int g, int bound);

 public:
  CostBoundedFFHeuristic(const Options &options);
  ~CostBoundedFFHeuristic();
};

}

#endif
