#!/bin/bash

DATE="$(date +%F@%R)"
RUN="$(dirname $(readlink -f $0))"
RESULTS="$RUN/../tests/$DATE"
BIN="${RUN%%run}bin"

MAXITER=50

# read args
arch="$1"
case "$arch" in
  "mic")
    maxthreads=100
    step=10
    binsuff="$arch"
    # prep work: copy data to mic
    scp -r "$BIN" "mic1:"
    scp -r "$RUN" "mic1:"
    # prep work: setup results dir
    ssh mic1 "mkdir -p tests/$DATE"
    ;;
  "xeon")
    maxthreads=20
    step=1
    binsuff="$arch"
    ;;
  "i5")
    maxthreads=10
    step=1
    binsuff="xeon"
    ;;
  ?*)
    printf '$0: Unknown architecture "%s"\n' "$arch" >&2
    exit 1
    ;;
esac

# create results folder
mkdir -p "$RESULTS"

# run tests
for size in "1000" "5000" "10000"; do
  for test in "baseline" "components"; do
    # generate data
    if [ "$arch" == "mic" ]; then
      # run test on mic, then copy back result
      ssh mic1 "run/run.sh" "bin/${test}_${binsuff}" "$size" "$MAXITER" "$maxthreads" "$step" "tests/$DATE/${test}_${arch}_${size}.dat"
      scp "mic1:tests/$DATE/${test}_${arch}_${size}.dat" "$RESULTS/${test}_${arch}_${size}.dat"
    else
      # run test locally
      "$RUN/run.sh" "$BIN/${test}_${binsuff}" "$size" "$MAXITER" "$maxthreads" "$step" "$RESULTS/${test}_${arch}_${size}.dat"
    fi
    # draw plot
    "$RUN/gnuplot.sh" "$RESULTS/${test}_${arch}" "$RESULTS/${test}_${arch}.svg"
  done
done

# if running on mic, clean up
if [ "$arch" == "mic" ]; then
  ssh mic1 rm -rf "$RUN" "$BIN" "tests/$DATE"
fi

touch "$RUN/../DONE"
