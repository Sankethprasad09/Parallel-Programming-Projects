#!/bin/bash

# Specify the path to the GCC compiler installed via Homebrew if not set in the PATH
# Example: export PATH="/usr/local/bin:$PATH" or set it explicitly
# export CC=gcc-11  # Adjust the version number based on your installation

for t in 1 2 4 8 12 16 20 24 32
do
  for n in 1 10 100 1000 10000 100000 500000 1000000
  do
    # Use GCC with OpenMP support, specifying the number of threads and trials as macros
    g++ -Xpreprocessor -fopenmp -I/usr/local/include -L/usr/local/lib -lomp proj01.cpp -DNUMT=$t -DNUMTRIALS=$n -o proj01
    ./proj01
  done
done
