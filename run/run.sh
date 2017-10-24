#!/bin/bash

# read args
program=$1
size=$2
niter=$3
maxthreads=$4
step=$5
outfile=$6

# put header on output file
echo "#threads performance time" >> "$outfile"

# run sequentially and set aside
printf "$program $size $niter 1... "
SEQTIME=$("$program" "$size" "$niter" 1)
printf "done.\n"
echo 1 1 $SEQTIME >> "$outfile"

for ((NTHREAD = 2; NTHREAD <= maxthreads; NTHREAD+=step)); do
  printf "$program $size $niter $NTHREAD... "
  PARTIME=$($program $size $niter $NTHREAD)
  printf "done.\n"
  PERF=$(awk "BEGIN {printf \"%f\",${SEQTIME}/${PARTIME}}")
  echo $NTHREAD $PERF $PARTIME >> "$outfile"
done

touch DONE

