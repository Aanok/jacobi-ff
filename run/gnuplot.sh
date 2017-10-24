#!/bin/bash
gnuplot << EOF

reset
set term pngcairo dashed size 1200,600
set output "$2"
stats "$1" u 1:2 nooutput
unset label
unset key
set yrange [0:STATS_max_y]
set arrow from STATS_pos_max_y,graph(0,0) to STATS_pos_max_y,graph(1,1) lc rgb "red" dt '-' nohead
set label 1 sprintf("%d", STATS_pos_max_y) at STATS_pos_max_y,-1 center
set xlabel "Threads"
set ylabel "Performance (T_{par}/T_{seq})"
plot x, "$1" u 1:2 with linespoints

EOF