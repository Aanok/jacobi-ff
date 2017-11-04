#!/bin/bash
gnuplot << EOF

reset
set term svg enhanced size 1200,600 font "arial,20"
set output "$2"
unset label
set key top right
set xlabel "Workers"
set ylabel "Efficiency (Speedup/Workers)"
plot "${1}_1000.dat" u 1:5 title '1000' with linespoints,\
     "${1}_5000.dat" u 1:5 title '5000' with linespoints,\
     "${1}_10000.dat" u 1:5 title '10000' with linespoints
EOF