#!/bin/bash

# Preprocess the problemset .pddl files into .sas files

# Competition: -c (competition name)
problemset="./inputs/ipc6.pset"

# Planner: -P (planner location)
planner="./fast-downward.py"

# Parse command line args
while [[ $# -gt 1 ]]
do
    key="$1"

    case $key in
	-p|--problemset)
	    problemset="$2"
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

cat "$problemset" | tail -n +2 | while read -r line
do
    line=$(echo $line | tr -s ' ')
    competition=$(echo $line | cut -d " " -f 1)
    domain=$(echo $line | cut -d " " -f 2)
    problem=$(echo $line | cut -d " " -f 3)
    
    pfile="./inputs/problems/$competition/seq-sat/$domain/$problem.pddl"
    dfile="./inputs/problems/$competition/seq-sat/$domain/${problem}-domain.pddl"
    ofile="./inputs/problems/$competition/seq-sat/$domain/$problem.out"
    
    if [ ! -e "$dfile" ]
    then
	dfile="./inputs/problems/$competition/seq-sat/$domain/domain.pddl"
    fi
    
    if [ ! -e "$pfile" ]
    then
	echo ERROR: Problem file missing: $pfile
    else
        $planner --translate --preprocess "$dfile" "$pfile"
	mv "./output" "$ofile"
    fi
done
