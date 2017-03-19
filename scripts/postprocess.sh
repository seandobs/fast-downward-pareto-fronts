#!/bin/bash

# Postprocess the fast downward outputs into a table of results

outputs="$1"

competition=ipc6
parameters="./inputs/problems-$competition-seq-sat.txt"

resultsheader="hlb sol src best sbest newbest bound found oom oot cost length expanded evaluations searchtime totaltime runtime exitcode"

echo "heuristic domain problem $resultsheader" \
     > "./$outputs/results"

for heuristic in ./$outputs/*/;
do
    heuristic=$(basename $heuristic)

    echo "g u h" \
	 > "./$outputs/$heuristic/nodestats"
    
    for domain in ./$outputs/$heuristic/*/;
    do
	domain=$(basename $domain)

        domainparam=$(grep "$domain" "$parameters")
		    
	for problem in ./$outputs/$heuristic/$domain/*.fdlog;
	do
	    problem=$(basename "$problem" | cut -d "." -f1)
	    
	    runfile="./$outputs/$heuristic/$domain/$problem.run"
	    runtime=NONE
	    exitcode=NONE
	    if [ -e "$runfile" ]
	    then
		exitcode=$(tail -n +2 "$runfile" | cut -d " " -f 1)
		runtime=$(tail -n +2 "$runfile" | cut -d " " -f 2)
	    fi

	    fdlog="./$outputs/$heuristic/$domain/$problem.fdlog"
	    fdlogfiltered=$(grep -Eo 'bound=[0-9]*|Solution found.|Plan cost: [0-9]*|Plan length: [0-9]*|Expanded [0-9]*|Evaluations: [0-9]*|Search time: [0-9]*\.?[0-9]*|Total time: [0-9]*\.?[0-9]*' "$fdlog")
	    
	    bound=$(echo "$fdlogfiltered" | grep -Eo 'bound=[0-9]*' | head -n 1 | cut -d "=" -f 2)
	    found=0
	    oom=0
	    oot=0
	    cost=DNF
	    length=DNF
	    expanded=DNF
	    evaluations=DNF
	    searchtime=DNF
	    totaltime=DNF
	    
	    if (echo "$fdlogfiltered" | grep -Fxq "Solution found.")
	    then
		found=1
		cost=$(echo "$fdlogfiltered" | grep -Eo 'Plan cost: [0-9]*' | head -n 1 | cut -d " " -f 3)
		length=$(echo "$fdlogfiltered" | grep -Eo 'Plan length: [0-9]*' | head -n 1 | cut -d " " -f 3)
		expanded=$(echo "$fdlogfiltered" | grep -Eo 'Expanded [0-9]*' | head -n 1 | cut -d " " -f 2)
		evaluations=$(echo "$fdlogfiltered" | grep -Eo 'Evaluations: [0-9]*' | head -n 1 | cut -d " " -f 2)
		searchtime=$(echo "$fdlogfiltered" | grep -Eo 'Search time: [0-9]*\.?[0-9]*'| head -n 1 | cut -d " " -f 3)
		totaltime=$(echo "$fdlogfiltered" | grep -Eo 'Total time: [0-9]*\.?[0-9]*' | head -n 1 | cut -d " " -f 3)
	    fi

	    if grep -Fxq "Memory limit has been reached." $fdlog
	    then
		oom=1
	    fi

	    if grep -Fxq "caught signal 24 -- exiting" $fdlog
	    then
		oot=1
	    fi
	    
	    param=$(echo "$domainparam" | grep "$problem" | tr -s ' ' | cut -d " " -f 2-)

	    results="$param $bound $found $oom $oot $cost $length $expanded $evaluations $searchtime $totaltime $runtime $exitcode"
	    echo "$heuristic $domain $results"  >> "./$outputs/results"

	    if [ -e "./$outputs/$heuristic/$domain/$problem.stat" ]
	    then
		scale=$(echo "1 / $bound" | bc -l)
		
		cat "./$outputs/$heuristic/$domain/$problem.stat" |
		    awk -v SF="$scale" '{printf($1*SF" "$2" "$3*SF"\n")}' >> "./$outputs/$heuristic/nodestats"
	    fi
	done
    done
done

find "./$outputs" -name "results" -exec sh -c "column -t {} > {}.temp && mv {}.temp {}" \;

./scripts/summarise.sh "$outputs"
