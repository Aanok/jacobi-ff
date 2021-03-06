#!/bin/bash

# average each measurement over a sample size
SAMPLE=5

# function for average measure
function run_sample() { # ARGS: program nthread
  local time=0;
  local tmp=0;
  for ((S = 1; S <= $SAMPLE; S++)); do
    tmp=$("$1" "$size" "$niter" "$2")
    time=$(awk "BEGIN {printf \"%f\",${time}+${tmp}}")
  done
  awk "BEGIN {printf \"%f\",${time}/${SAMPLE}}"
}


# read args
program_seq="$1"
program_par="$2"
size="$3"
niter="$4"
maxthreads="$5"
step="$6"
outfile="$7"

# put header on output file
echo "#threads time speedup scalability efficiency" >> "$outfile"

# run sequentially and set aside
printf "$program_seq $size $niter... "
SEQTIME=$(run_sample "$program_seq" 1)
printf "done: %s\n" "$SEQTIME"
echo "# Sequential execution time: $SEQTIME" >> "$outfile"

# run in parallel
# program with 1 thread is always run
printf "$program_par $size $niter 1... "
PARONETIME=$(run_sample "$program_par" "1")
printf "done: %s\n" "$PARONETIME"
SPEEDUP=$(awk "BEGIN {printf \"%f\",${SEQTIME}/${PARONETIME}}")
echo 1 $PARONETIME $SPEEDUP 1 $SPEEDUP >> "$outfile"

# then if $step >1 we don't go to 1+$step but just $step
if [ "$step" == "1" ]; then start=2; else start="$step"; fi

for ((NTHREAD=start; NTHREAD <= maxthreads; NTHREAD+=step)); do
  printf "$program_par $size $niter $NTHREAD... "
  PARTIME=$(run_sample "$program_par" $NTHREAD)
  printf "done: %s\n" "$PARTIME"
  SPEEDUP=$(awk "BEGIN {printf \"%f\",${SEQTIME}/${PARTIME}}")
  SCALABILITY=$(awk "BEGIN {printf\"%f\",${PARONETIME}/${PARTIME}}")
  EFFICIENCY=$(awk "BEGIN {printf\"%f\",${SPEEDUP}/${NTHREAD}}")
  echo $NTHREAD $PARTIME $SPEEDUP $SCALABILITY $EFFICIENCY >> "$outfile"
done
