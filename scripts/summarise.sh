#!/bin/bash

# Finds the total the number of problems that were solved / ran out of memory / ran out of time

outputs="$1"

competition=ipc6
parameters="./inputs/problems-$competition-seq-sat.txt"

echo "domain\heuristic" > "./$outputs/solved"
echo "domain\heuristic" > "./$outputs/oom"
echo "domain\heuristic" > "./$outputs/oot"
echo "domain\heuristic" > "./$outputs/smt"

for heuristic in ./$outputs/*/;
do
    heuristic=$(basename $heuristic)

    solvedline=$(cat "./$outputs/solved")
    oomline=$(cat "./$outputs/oom")
    ootline=$(cat "./$outputs/oot")
    smtline=$(cat "./$outputs/smt")
    
    echo "$solvedline &$heuristic" > "./$outputs/solved"
    echo "$oomline &$heuristic" > "./$outputs/oom"
    echo "$ootline &$heuristic" > "./$outputs/oot"
    echo "$smtline &$heuristic" > "./$outputs/smt"
done

for domain in $(basename -a $(ls -d ./$outputs/*/*/) | perl -ne 'print unless $seen{$_}++');
do
    domain=$(basename $domain)
    solvedline="$domain"
    oomline="$domain"
    ootline="$domain"
    smtline="$domain"

    domainresults=$(grep "$domain " "$outputs/results" | tr -s " ")
    
    for heuristic in ./$outputs/*/;
    do
	heuristic=$(basename $heuristic)

	heuristicresults=$(echo "$domainresults" | grep "$heuristic ")
	
	totalsolved=0
	totaloom=0
	totaloot=0
	
	if [[ ! -z "$heuristicresults" ]]
	then
	    totalsolved=$(echo "$heuristicresults" |  cut -d " " -f11 |
				 paste -sd+ - |
				     bc)
	    totaloom=$(echo "$heuristicresults" |  cut -d " " -f12 |
				 paste -sd+ - |
				     bc)
	    totaloot=$(echo "$heuristicresults" |  cut -d " " -f13 |
				 paste -sd+ - |
				     bc)
	fi
        
	solvedline="$solvedline &$totalsolved"
	oomline="$oomline &$totaloom"
	ootline="$ootline &$totaloot"
	smtline="$smtline &$totalsolved/$totaloom/$totaloot"
    done

    echo "$solvedline" >> "./$outputs/solved"
    echo "$oomline" >> "./$outputs/oom"
    echo "$ootline" >> "./$outputs/oot"
    echo "$smtline" >> "./$outputs/smt"
done

solvedline="total"
oomline="total"
ootline="total"
smtline="total"

i=2
for heuristic in ./$outputs/*/;
do
    smtcol=$(tail -n +2 "./$outputs/smt" |  cut -d " " -f$i | tr -d "&")
    totalsolved=$(echo "$smtcol" |  cut -d "/" -f1 |
			 paste -sd+ - |
			 bc)
    totaloom=$(echo "$smtcol" |  cut -d "/" -f2 |
			 paste -sd+ - |
			 bc)
    totaloot=$(echo "$smtcol" |  cut -d "/" -f3 |
			 paste -sd+ - |
			 bc)

    solvedline="$solvedline &$totalsolved"
    oomline="$oomline &$totaloom"
    ootline="$ootline &$totaloot"
    smtline="$smtline &$totalsolved/$totaloom/$totaloot"
    
    i=$(($i+1))
done

echo "$solvedline" >> "./$outputs/solved"
echo "$oomline" >> "./$outputs/oom"
echo "$ootline" >> "./$outputs/oot"
echo "$smtline" >> "./$outputs/smt"

column -t "./$outputs/solved" |
    sed 's/&/& /g' |
    sed 's/\// \/ /g' > "./$outputs/solved.temp" &&
    mv "./$outputs/solved.temp" "./$outputs/solved"
column -t "./$outputs/oom" |
    sed 's/&/& /g' |
    sed 's/\// \/ /g' > "./$outputs/oom.temp" &&
    mv "./$outputs/oom.temp" "./$outputs/oom"
column -t "./$outputs/oot" |
    sed 's/&/& /g' |
    sed 's/\// \/ /g' > "./$outputs/oot.temp" &&
    mv "./$outputs/oot.temp" "./$outputs/oot"
column -t "./$outputs/smt" |
    sed 's/&/& /g' |
    sed '1 s/&/    &/g' |
    sed 's/\// \/ /g'  > "./$outputs/smt.temp" &&
    mv "./$outputs/smt.temp" "./$outputs/smt"


