#include "cb_ff_heuristic.h"

#include "../global_state.h"
#include "../option_parser.h"
#include "../plugin.h"
#include "../task_tools.h"

#include <cassert>

using namespace std;

namespace cb_ff_heuristic {

// construction and destruction
CostBoundedFFHeuristic::CostBoundedFFHeuristic(const Options &opts)
  : Heuristic(opts),
    initial_plan_type(RelaxedPlanType(opts.get_enum("initial_plan"))),
    improvement_type(ImprovementType(opts.get_enum("improve"))),
    penalty_factor(opts.get<int>("penalty_factor"))
{
  if ((initial_plan_type == CHEAP) && (improvement_type != NONE)) {
    cout << "invalid option combination: intial_plan = CHEAP and improve != NONE" << endl;
    exit(1);
  }
  assert(penalty_factor >= 1);
}

CostBoundedFFHeuristic::~CostBoundedFFHeuristic()
{
  // ??
}

bool CostBoundedFFHeuristic::did_write_overflow_warning = false;

void CostBoundedFFHeuristic::write_overflow_warning()
{
  if (!did_write_overflow_warning) {
    cout << "WARNING: overflow in SumCombinator! Costs clamped to "
	 << MAX_COST_VALUE << endl;
    cerr << "WARNING: overflow in SumCombinator! Costs clamped to "
	 << MAX_COST_VALUE << endl;
    did_write_overflow_warning = true;
  }
}

void CostBoundedFFHeuristic::setup_exploration_queue
(unsigned int slot, const State &state, bool unit_cost)
{
  queue.clear();
  for (int var = 0; var < propositions.size(); var++) {
    for (int value = 0; value < propositions[var].size(); value++) {
      Proposition &prop = propositions[var][value];
      prop.cost[slot] = -1;
    }
  }
  // Deal with operators and axioms without preconditions.
  for (int i = 0; i < unary_operators.size(); i++) {
    UnaryOperator &op = unary_operators[i];
    op.unsatisfied_preconditions = op.precondition.size();
    if (unit_cost)
      op.cost[slot] = 1;
    else
      op.cost[slot] = op.base_cost;
    if (op.unsatisfied_preconditions == 0)
      enqueue_if_necessary(op.effect, slot, MAX_SLOTS, op.cost[slot], &op);
  }
  // add the state
  for (int var = 0; var < propositions.size(); var++) {
    Proposition *init_prop = &propositions[var][state[var]];
    enqueue_if_necessary(init_prop, slot, MAX_SLOTS, 0, 0);
  }
}

void CostBoundedFFHeuristic::unmark_propositions()
{
  for (int var = 0; var < propositions.size(); var++) {
    for (int value = 0; value < propositions[var].size(); value++) {
      Proposition &prop = propositions[var][value];
      prop.marked = false;
    }
  }
}

void CostBoundedFFHeuristic::reset_switches()
{
  for (int var = 0; var < propositions.size(); var++) {
    for (int value = 0; value < propositions[var].size(); value++) {
      Proposition &prop = propositions[var][value];
      prop.switch_to_slot = MAX_SLOTS;
    }
  }
}

void CostBoundedFFHeuristic::extract_relaxed_plan
(unsigned int slot, Proposition *goal, RelaxedPlan& relaxed_plan)
{
  if (!goal->marked) {
    goal->marked = true;
    if (goal->switch_to_slot < MAX_SLOTS)
      slot = goal->switch_to_slot;
    UnaryOperator *unary_op = goal->reached_by[slot];
    if (unary_op) { // We have not yet chained back to a start node.
      if (unary_op->operator_no != -1) {
	relaxed_plan[unary_op->operator_no] = true;
	for (int i = 0; i < unary_op->precondition.size(); i++)
	  extract_relaxed_plan(slot, unary_op->precondition[i], relaxed_plan);
      }
    }
  }
}

void CostBoundedFFHeuristic::collect_preferred_operators
(const State &state, const RelaxedPlan& relaxed_plan)
{
  for (unsigned int i = 0; i < g_operators.size(); i++)
    if (relaxed_plan[i]) {
      const Operator& op = g_operators[i];
      if (op.is_applicable(state))
	set_preferred(&op);
    }
}

int CostBoundedFFHeuristic::relaxed_plan_cost(RelaxedPlan& relaxed_plan)
{
  int total_cost = 0;
  for (int op_no = 0; op_no < relaxed_plan.size(); op_no++)
    if (rp[op_no])
      increase_cost(total_cost, g_operators[op_no].get_cost());
  return total_cost;
}

int CostBoundedFFHeuristic::relaxed_plan_size(RelaxedPlan& relaxed_plan)
{
  int total_size = 0;
  for (int op_no = 0; op_no < relaxed_plan.size(); op_no++)
    if (rp[op_no])
      total_size += 1;
  return total_size;
}

int CostBoundedFFHeuristic::compute_heuristic(const GlobalState &)
{
  cout << "wrong version of CostBoundedFFHeuristic::compute_heuristic called"
       << endl;
  assert(0);
  return DEAD_END;
}

static const unsigned int unit_add_slot = 0;
static const unsigned int cost_add_slot = 1;

static bool compare_min_depth(const CostBoundedFFHeuristic::Proposition *p1,
			      const CostBoundedFFHeuristic::Proposition *p2)
{
  return p1->cost[unit_add_slot] < p2->cost[unit_add_slot];
}

static bool compare_max_cost(const CostBoundedFFHeuristic::Proposition *p1,
			     const CostBoundedFFHeuristic::Proposition *p2)
{
  return p1->cost[cost_add_slot] > p2->cost[cost_add_slot];
}

int CostBoundedFFHeuristic::compute_heuristic
(const GlobalState &global_state, int g, int bound)
{
  State state = convert_global_state(global_state);
  
  int room = (bound - g);

  SumCombinator sc(*this);
  switch (initial_plan_type) {
  case CHEAP:
    setup_exploration_queue(unit_add_slot, state, true);
    relaxed_exploration<SumCombinator>(unit_add_slot, MAX_SLOTS, sc);
    setup_exploration_queue(cost_add_slot, state, false);
    relaxed_exploration<SumCombinator>(cost_add_slot, unit_add_slot, sc);
    break;
  case SHORT_THEN_CHEAP:
    setup_exploration_queue(cost_add_slot, state, false);
    relaxed_exploration<SumCombinator>(cost_add_slot, MAX_SLOTS, sc);
    setup_exploration_queue(unit_add_slot, state, true);
    relaxed_exploration<SumCombinator>(unit_add_slot, cost_add_slot, sc);
    break;
  default:
    setup_exploration_queue(unit_add_slot, state, true);
    relaxed_exploration<SumCombinator>(unit_add_slot, MAX_SLOTS, sc);
  }

  for (int i = 0; i < goal_propositions.size(); i++) {
    int prop_cost = goal_propositions[i]->cost[unit_add_slot];
    if (prop_cost == -1)
      return DEAD_END;
  }

  unmark_propositions();
  rp.assign(rp.size(), false);
  for (int i = 0; i < goal_propositions.size(); i++) {
    switch (initial_plan_type) {
    case CHEAP:
      extract_relaxed_plan(cost_add_slot, goal_propositions[i], rp);
      break;
    default:
      extract_relaxed_plan(unit_add_slot, goal_propositions[i], rp);
      break;
    }
  }

  int rp_cost = relaxed_plan_cost(rp);
  int initial_rp_cost = rp_cost;
  int initial_h = 0;

  // try to optimise the relaxed plan a bit...

  if ((improvement_type == ONCE) && (rp_cost >= room)) {
    initial_h = relaxed_plan_size(rp);

    if (initial_plan_type == SHORT) {
      setup_exploration_queue(cost_add_slot, state, false);
      relaxed_exploration<SumCombinator>(cost_add_slot, unit_add_slot, sc);
    }

    /// single-shot version:
    reset_switches();
    unmark_propositions();
    rp.assign(rp.size(), false);
    for (int i = 0; i < goal_propositions.size(); i++)
      extract_relaxed_plan(cost_add_slot, goal_propositions[i], rp);
    rp_cost = relaxed_plan_cost(rp);
  }

  if ((improvement_type == TOP_DOWN) && (rp_cost >= room)) {
    initial_h = relaxed_plan_size(rp);

    if (initial_plan_type == SHORT) {
      setup_exploration_queue(cost_add_slot, state, false);
      relaxed_exploration<SumCombinator>(cost_add_slot, unit_add_slot, sc);
    }

    std::vector<Proposition*> sorted_goals(goal_propositions);
    sort(sorted_goals.begin(), sorted_goals.end(), compare_max_cost);

    unsigned int sg_no = 0;
    while ((sg_no < sorted_goals.size()) && (rp_cost >= room)) {
      unmark_propositions();
      rp.assign(rp.size(), false);
      for (int i = 0; i < sorted_goals.size(); i++)
	if (i <= sg_no)
	  extract_relaxed_plan(cost_add_slot, sorted_goals[i], rp);
	else
	  extract_relaxed_plan(unit_add_slot, sorted_goals[i], rp);
      rp_cost = relaxed_plan_cost(rp);
      sg_no += 1;
    }
  }

  if ((improvement_type == BOTTOM_UP) && (rp_cost >= room)) {
    initial_h = relaxed_plan_size(rp);

    if (initial_plan_type == SHORT) {
      setup_exploration_queue(cost_add_slot, state, false);
      relaxed_exploration<SumCombinator>(cost_add_slot, unit_add_slot, sc);
    }

    std::vector<Proposition*> subgoals;
    for (int var = 0; var < propositions.size(); var++)
      for (int value = 0; value < propositions[var].size(); value++)
    	if (propositions[var][value].marked)
    	  subgoals.push_back(&propositions[var][value]);
    sort(subgoals.begin(), subgoals.end(), compare_min_depth);
    reset_switches();
    unsigned int sg_no = 0;
    while ((sg_no < subgoals.size()) && (rp_cost >= room)) {
      subgoals[sg_no]->switch_to_slot = cost_add_slot;
      unmark_propositions();
      rp.assign(rp.size(), false);
      for (int i = 0; i < goal_propositions.size(); i++)
    	extract_relaxed_plan(unit_add_slot, goal_propositions[i], rp);
      int new_rp_cost = relaxed_plan_cost(rp);
      if (new_rp_cost > rp_cost) {
    	subgoals[sg_no]->switch_to_slot = MAX_SLOTS;
      }
      else {
    	rp_cost = new_rp_cost;
      }
      sg_no += 1;
    }
    // need to redo the extraction at the end, since the best rp
    // may not be the last one computed.
    unmark_propositions();
    rp.assign(rp.size(), false);
    for (int i = 0; i < goal_propositions.size(); i++)
      extract_relaxed_plan(unit_add_slot, goal_propositions[i], rp);
  }

  int h_ff = relaxed_plan_size(rp);

  // if ((initial_rp_cost >= room) && (rp_cost < room)) {
  //   cout << "improve: " << initial_h << " -> " << h_ff << endl;
  // }

  if (rp_cost >= room) h_ff *= penalty_factor;
  collect_preferred_operators(state, rp);

  return h_ff;
}


static ScalarEvaluator *_parse(OptionParser &parser) {
  // Heuristic::add_options_to_parser(parser);

  vector<string> initial_types;
  initial_types.push_back("SHORT");
  initial_types.push_back("CHEAP");
  initial_types.push_back("SHORT_THEN_CHEAP");
  parser.add_enum_option("initial_plan",
			 initial_types,
			 "SHORT",
			 "initial relaxed plan");

  vector<string> improve_types;
  improve_types.push_back("NONE");
  improve_types.push_back("ONCE");
  improve_types.push_back("TOP_DOWN");
  improve_types.push_back("BOTTOM_UP");
  parser.add_enum_option("improve",
			 improve_types,
			 "NONE",
			 "relaxed plan improvement method");

  parser.add_option<int>("penalty_factor", 2, "penalty factor for over-cost relaxed plans");
  Options opts = parser.parse();
  opts.set<int>("cost_type", 1);
  if (parser.dry_run())
    return 0;
  else
    return new CostBoundedFFHeuristic(opts);
}

static Plugin<ScalarEvaluator> _plugin("cbff", _parse);

}
