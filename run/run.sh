#!/bin/bash

# read args
program=$1
size=$2
niter=$3
maxthreads=$4
step=$5
outfile=$6

# average each measurement over a sample size
SAMPLE=3

# function for average measure
function run_sample() { # ARG: nthread
  local time=0;
  local tmp=0;
  for ((S = 1; S <= $SAMPLE; S++)); do
    tmp=$("$program" "$size" "$niter" "$1")
    time=$(awk "BEGIN {printf \"%f\",${time}+${tmp}}")
  done
  awk "BEGIN {printf \"%f\",${time}/${SAMPLE}}"
}


# put header on output file
echo "#threads performance time" >> "$outfile"

# run sequentially and set aside
printf "$program $size $niter 1... "
SEQTIME=$(run_sample 1)
printf "done: %s\n" "$SEQTIME"
echo 1 1 $SEQTIME >> "$outfile"

for ((NTHREAD=step; NTHREAD <= maxthreads; NTHREAD+=step)); do
  printf "$program $size $niter $NTHREAD... "
  PARTIME=$(run_sample $NTHREAD)
  printf "done: %s\n" "$PARTIME"
  PERF=$(awk "BEGIN {printf \"%f\",${SEQTIME}/${PARTIME}}")
  echo $NTHREAD $PERF $PARTIME >> "$outfile"
done

