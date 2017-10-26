#!/bin/bash
gnuplot << EOF

reset
set term svg enhanced size 1200,600 font "arial,20"
set output "$2"
unset label
unset key
set xlabel "Workers"
set ylabel "Scalability (T_{n}/T_{1})"
plot x, "$1" u 1:2 with linespoints

EOF