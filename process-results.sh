#!/bin/bash
g++ -std=c++14 ./main.cpp -O2 -fopenmp -D_GLIBCXX_PARALLEL -Wall -DNDEBUG
minelsize=1
maxelsize=33554432
iterationcount=50
threadcount=4
#./a.out $minelsize $maxelsize $iterationcount $threadcount > run.log
cd latex
rm doc.pdf
sp-process doc.tex; pdflatex doc.tex
evince doc.pdf
