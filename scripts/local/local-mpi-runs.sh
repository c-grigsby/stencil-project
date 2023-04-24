#!/bin/bash

NUM_ITERATIONS=100                  # number of iterations 
INITIAL_FILE_NAME=init.dat          # input data file name
FINAL_FILE_NAME=final.dat           # final state file name
STACKED_FILE_NAME=all.raw           # stacked matrix timestamps
SUMMARY_FILE_NAME=summary-mpi.txt   # summary data file for py-charts

PATH=$PATH:$(pwd)

echo 'Hello, about to run the application for the MPI version'
echo ''
echo "Making input matrices..."
# Loop iterates over a range of values to create the matrix sizes
for (( i=1000; i<=16000; i=i*2 ))
do
    ./make-2d $i $i $i-$INITIAL_FILE_NAME
    echo "NxN matrix of size: $i created"
done
echo ""
echo "Running MPI Stencil Calculations ..."
for (( i=1000; i<=16000; i=i*2 ))
do
    # Second loop iterates over a range of values for j
    for j in 1 2 4 8 16
    do
        echo "==== EXECUTING: mpirun -np $j --oversubscribe ./mpi-stencil-2d $NUM_ITERATIONS $i-$INITIAL_FILE_NAME $FINAL_FILE_NAME 1 $SUMMARY_FILE_NAME ===="
        echo ""

        mpirun -np $j --oversubscribe ./mpi-stencil-2d $NUM_ITERATIONS $i-$INITIAL_FILE_NAME $FINAL_FILE_NAME 1 $SUMMARY_FILE_NAME
    done
done

echo "==== All EXECUTIONS COMPLETE ===="

