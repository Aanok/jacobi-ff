#!/bin/bash

DATE="$(date +%F@%R)"
RUN="$(dirname $(readlink -f $0))"
RESULTS="$RUN/$DATE"
BIN="${RUN%%run}bin"

SIZE=10000
NITER=100
STEP=1

# read args
arch="$1"
case "$arch" in
  "mic")
    maxthreads=100
    binsuff="$arch"
    ;;
  "xeon")
    maxthreads=30
    binsuff="$arch"
    ;;
  "i5")
    maxthreads=10
    binsuff="xeon"
    ;;
  ?*)
    printf 'ERROR: Unknown architecture "%s"\n' "$arch" >&2
    exit 1
    ;;
esac

# create results folder
mkdir "$RESULTS"

# run tests
for test in "baseline" "blocks" "components"; do
  # generate data
  "$RUN/run.sh" "$BIN/${test}_${binsuff}" "$SIZE" "$NITER" "$maxthreads" "$STEP" "$RESULTS/${test}_${arch}.dat"
  # draw plot
  "$RUN/gnuplot.sh" "$RESULTS/${test}_${arch}.dat" "$RESULTS/${test}_${arch}.png"
done
