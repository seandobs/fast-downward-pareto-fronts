# pareto-front-planner
A modified version of the Fast Downward planner that includes Pareto Front heuristics for bounded-cost search.

This work was completed in service of a research project undertaken at the Australian National University 2016-2017. This project is not maintained, and was not written with the intention of being useful outside of the scope of that research.

Documentation:
* ICAPS Paper describing the theory behind this implementation: https://github.com/seandobs/pareto-front-planner/blob/master/docs/report.pdf

Usage:
* Compile: `./build.py`
* Preprocess PDDL to SAS:`./scripts/preprocess.sh <problemset file>`
* Run my experiments: `./scripts/run-experiment.sh`
* Postprocess planner outputs: `./scripts/postprocess.sh <output directory>`
* Tabulate postprocessed results: `./scripts/summarise.sh <output directory>`
* Plot results: `./scripts/compare-totals.R`

Licensing:
* Fast Downward and the supplementary scripts contained in this project are distributed under the GNU GPL V3: https://github.com/seandobs/pareto-front-planner/blob/master/LICENSE
