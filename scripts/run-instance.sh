#!/bin/bash
# Description:
# Executes Fast Downward on the given problem instance.
# Planner behaviour follows some presets depending on the supplied args

# Command line args

# Heuristic for guiding the greedy search: -h (ff-d, ff-h, ipdb-h,
#   ipdb-pts, ipb-astar, ipfpdb-h, ipfpdb-d, ipfpdb-pts, ipfpdb-ework)
heuristic="ipdb-astar"

# Output folder: -o (name)
outputs="outputs"

# Whether we print statistics: -s (true, false)
stats="false"

# Time limit for search: -T (time)
overall_time_limit="10m"

# Memory limit for search: -M (size)
overall_memory_limit="1G"

# Time limit for hill climbing in seconds: -H (number)
max_hill_climbing_time="120"

# Cost boundary: -C (number)
bound="99999999"

# Competition: -c (competition name)
competition="ipc6"

# Problem domain: -d (domain name)
domain="elevators-strips"

# Problem instance: -p (problem name)
problem="p01"

# Planner: -P (planner location)
planner="./fast-downward.py"

# Parse command line args
while [[ $# -gt 1 ]]
do
key="$1"

case $key in
    -h|--heuristic)
	heuristic="$2"
	shift
	;;
    -o|--outputs)
	outputs="$2"
	shift
	;;
    -s|--stats)
	stats="$2"
	shift
	;;
    -T|--time-limit)
	overall_time_limit="$2"
	shift
	;;
    -M|--memory-limit)
	overall_memory_limit="$2"
	shift
	;;
    -H|--hill-climbing-limit)
	max_hill_climbing_time="$2"
	shift
	;;
    -C|--cost-bound)
	bound="$2"
	shift
	;;
    -c|--competition)
	competition="$2"
	shift
	;;
    -d|--domain)
	domain="$2"
	shift
	;;
    -p|--problem)
	problem="$2"
	shift
	;;
    -P|--planner)
	planner="$2"
	shift
	;;
    *)
        echo "Unknown option: $key"
	exit
	;;
esac
shift
done

# Default heuristic settings (will be set depending on heuristic arg)
objective="h"
aggregate="max"
pareto="false"
ff_transform="no_transform()"
f_eval="sum([g(), ipfpdb])"
pts_eval="pts([g(), ipfpdb, bound_const])"
unsafe_pruning="false"

# Parse heuristic, build open list
case $heuristic in
    "ff-"*)
	open="tiebreaking([ffx, g()], unsafe_pruning=$unsafe_pruning)"
	if [ $(echo "$heuristic" | cut -d "-" -f 2-) == "d" ]
	then
	    ff_transform="adapt_costs(cost_type=ONE)"
	fi
	;;
    "ipdb-pts")
	open="tiebreaking([$pts_eval, g()], unsafe_pruning=$unsafe_pruning)"
	;;
    "ipdb-h")
	open="tiebreaking([ipfpdb, g()], unsafe_pruning=$unsafe_pruning)"
	;;
    "ipdb-astar")
	open="tiebreaking([$f_eval, g()], unsafe_pruning=$unsafe_pruning)"
	;;
    "ipfpdb-"*"-"*)
	pareto="true"
	objective=$(echo "$heuristic" | cut -d "-" -f 2)
	aggregate=$(echo "$heuristic" | cut -d "-" -f 3)
	f_eval="sum([g()])"
	open="tiebreaking([ipfpdb, g()], unsafe_pruning=$unsafe_pruning)"
	;;
    "ipdb-"*"-"*)
	pareto="false"
	objective=$(echo "$heuristic" | cut -d "-" -f 2)
	aggregate=$(echo "$heuristic" | cut -d "-" -f 3)
	f_eval="sum([g()])"
	open="tiebreaking([ipfpdb, g()], unsafe_pruning=$unsafe_pruning)"
	;;
    *)
	echo "Unknown heuristic: $heuristic"
	exit
	;;
esac

# Construct heuristics depending on args
ipfpdb="
ipfpdb=cpdbs(
    patterns=hillclimbing(
    max_time=$max_hill_climbing_time,
    bound=$bound),
  cache_estimates=false,
  pareto=$pareto,
  objective=$objective,
  aggregate=$aggregate)"

ff="
ffx=ff(transform=$ff_transform)"

bound_const="
bound_const=const(value=$bound)"

# If needed, print stats for every node expanded
if [ "$stats" == "true" ]
then
    print_eval="
print(evals=[g(), u(), ipfpdb],
  file=./$outputs/$heuristic/$domain/$problem.stat)"
    search="eager($open,
  f_eval=$f_eval, 
  print_eval=$print_eval,
  bound=$bound,
  reopen_closed=true)"
else
    search="
eager($open,
  f_eval=$f_eval,
  bound=$bound,
  reopen_closed=true)"
fi

# Points to the pre-processor output file for this problem
ofile="./inputs/problems/$competition/seq-sat/$domain/$problem.out"
if [ ! -e "$ofile" ]
then
    echo "ERROR: Pre-processor output file missing: $ofile"
else
    # Make output directory if necessary
    if [ ! -d "./$outputs/$heuristic/$domain" ]
    then
	mkdir "./$outputs/$heuristic/$domain" -p
    fi

    # Execute the planner on the problem
    start=`date +%s.%N`
    $planner --overall-time-limit "$overall_time_limit" \
	     --overall-memory-limit "$overall_memory_limit" \
	     --plan-file "./$outputs/$heuristic/$domain/$problem.sas_plan" \
	     "$ofile" \
	     --heuristic "$ipfpdb" \
	     --heuristic "$ff" \
	     --heuristic "$bound_const" \
	     --search "$search" > "./$outputs/$heuristic/$domain/$problem.fdlog"
    exitcode="$?"
    end=`date +%s.%N`

    # Record the running time and exit code for the given problem
    runtime=$( echo "$end - $start" | bc -l )
    echo "exitcode runtime" > "./$outputs/$heuristic/$domain/$problem.run"
    echo "$exitcode $runtime" >> "./$outputs/$heuristic/$domain/$problem.run"
fi
