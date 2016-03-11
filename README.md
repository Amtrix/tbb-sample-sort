# tbb-sample-sort
Implementation of sample sort, using the tbb threading library.

Build via cmake:
  * mkdir build; cd build; cmake ../src; make

Execute ./run-benchmark.sh:
 * executes the benchmark for some experiments
 * writes output to run.log

Execute ./process-results.sh:
 * parses the benchmark results
 * inserts aggregated data into ./latex/doc.tex

Requirements:
 * sqlplot-tools (see: https://github.com/bingmann/sqlplot-tools)