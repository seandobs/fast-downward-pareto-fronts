#ifndef MERGE_AND_SHRINK_FTS_FACTORY_H
#define MERGE_AND_SHRINK_FTS_FACTORY_H

/*
  Factory for factored transition systems.

  Takes a planning task and produces a factored transition system that
  represents the planning task. This provides the main bridge from
  planning tasks to the concepts on which merge-and-shrink abstractions
  are based (transition systems, labels, etc.). The "internal" classes of
  merge-and-shrink should not need to know about planning task concepts.
*/

class TaskProxy;

namespace merge_and_shrink {
class FactoredTransitionSystem;
enum class Verbosity;

extern FactoredTransitionSystem create_factored_transition_system(
    const TaskProxy &task_proxy,
    Verbosity verbosity,
    bool finalize_if_unsolvable = true);

extern FactoredTransitionSystem create_factored_transition_system(
    const TaskProxy &task_proxy,
    Verbosity verbosity,
    int bound,
    bool finalize_if_unsolvable = true);
}

#endif