#!/bin/bash

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT


todo_array=()

stats="false"
aggregate="max"

tightness="loose"
outputs="outputs/$tightness"
params="-t $tightness -o $outputs -s $stats -p inputs/ipc6.pset"

heuristic="ff-d"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

heuristic="ipdb-astar"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh -h "$heuristic" $params)

heuristic="ipdb-d-sum"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

heuristic="ipdb-ework-sum"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

heuristic="ipdb-pts-max"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh -h "$heuristic" $params)

heuristic="ipdb-pts-sum"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

heuristic="ipfpdb-d-sum"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

heuristic="ipfpdb-ework-sum"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

tightness="tight"
outputs="outputs/$tightness"
params="-t $tightness -o $outputs -s $stats -p inputs/ipc6.pset"

heuristic="ff-d"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

heuristic="ipdb-astar"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh -h "$heuristic" $params)

heuristic="ipdb-d-sum"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

heuristic="ipdb-ework-sum"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

heuristic="ipdb-pts-max"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh -h "$heuristic" $params)

heuristic="ipdb-pts-sum"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

heuristic="ipfpdb-d-sum"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

heuristic="ipfpdb-ework-sum"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)


tightness="loose"
outputs="outputs/$tightness"
params="-t $tightness -o $outputs -s $stats -p inputs/ipc6.pset"

heuristic="ipdb-h-sum"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

tightness="tight"
outputs="outputs/$tightness"
params="-t $tightness -o $outputs -s $stats -p inputs/ipc6.pset"

heuristic="ipdb-h-sum"
mapfile -t -O ${#todo_array[@]} todo_array < <(./scripts/build-todo.sh  -h "$heuristic" $params)

./scripts/batch -t $1 -j $2 "${todo_array[@]}"
