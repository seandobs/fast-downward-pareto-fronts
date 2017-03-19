#!/bin/bash
# Description:
# Prints (but does not run) a list of calls to run-instance.sh for the given problem set.

# Command line args

# Heuristic for guiding the greedy search: -h [heuristic]
# Where [heuristic] is one of:
#  ff-d, ff-h, ipb-astar,
#  ipdb-[objective]-[aggregator],
#  ipfpdb-[objective]-[aggregator]
# [objective] is one of:
#  h, d, pts, ework
# and [aggregator] is one of:
#  min, max, sum
heuristic="ipdb-astar"

# Output folder: -o [folder name]
outputs="outputs"

# Whether we print statistics: -s [true, false]
stats="false"

# Time limit for search: -T [time]
overall_time_limit="10m"

# Memory limit for search: -M [size]
overall_memory_limit="1G"

# Time limit for hill climbing in seconds: -H [number]
max_hill_climbing_time="120"

# Problem set: -p [pset file]
problemset="./inputs/ipc6.pset"

# Planner: -P [planner script]
planner="./fast-downward.py"

# Parse command line args
while [[ $# -gt 0 ]]
do
    key="$1"

    case $key in
	-h|--heuristic)
	    heuristic="$2"
	    shift
	    ;;
	-p|--problemset)
	    problemset="$2"
	    shift
	    ;;
	-t|--tightness)
	    tightness="$2"
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

# Read the problems from the problem set file
cat "$problemset" | tail -n +2 | while read -r line
do
    # Parse the collumns
    line=$(echo $line | tr -s ' ')
    competition=$(echo $line | cut -d " " -f 1)
    domain=$(echo $line | cut -d " " -f 2)
    problem=$(echo $line | cut -d " " -f 3)
    hlb=$(echo $line | cut -d " " -f 4)
    sol=$(echo $line | cut -d " " -f 5)
    src=$(echo $line | cut -d " " -f 6)
    best=$(echo $line | cut -d " " -f 7)
    sbest=$(echo $line | cut -d " " -f 8)
    newbest=$(echo $line | cut -d " " -f 9)
    
    # Set cost bound depending on tightness
    bound=99999999
    if [ "$tightness" == "tight" ]
    then
	if [ ! "$best" == "Inf" ]
	then
	    let bound=$best
	fi
    else
	if [ ! "$sbest" == "Inf" ]
	then
	    let bound=$sbest-1
	fi
    fi
    
    echo ./scripts/run-instance.sh \
	 "-h $heuristic" \
	 "-o $outputs" \
	 "-s $stats" \
	 "-T $overall_time_limit" \
	 "-M $overall_memory_limit" \
	 "-H $max_hill_climbing_time" \
	 "-C $bound" \
	 "-c $competition" \
	 "-d $domain" \
	 "-p $problem" \
	 "-P $planner"
    
done
