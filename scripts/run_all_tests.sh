#!/bin/bash

THREADS=4

while getopts "t:" opt; do
  case $opt in
    t) THREADS=$OPTARG ;;
    *) echo "Usage: $0 [-t num_threads]"; exit 1 ;;
  esac
  shift $((OPTIND -1))
done

mkdir -p build/build_docker && cd build/build_docker
cmake ../cmake
make -j
cd ../../

# remove all previous outputs/diffs
rm -rf output/task1/
rm -rf output/diffs/

echo "Creating diffs..."
mkdir -p output && mkdir -p output/diffs
echo "------------------Testing all cases------------------"
TESTS="data/task1/*.json"; for i in $TESTS; do build/build_docker/bin/task1 -t "$THREADS" "$i" || true; echo "Ran task ${i}"; done
OUTPUTS="output/task1/*"; for i in $OUTPUTS; do compare -compose src "$i" "output/ref/${i##*/}" "output/diffs/${i##*/}" || true; echo "Output diffed ${i}"; done
exit 0