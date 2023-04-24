#!/bin/bash

# The following is a script to generate 3 summary-{version}.txt files
# From 3 versions of the application (serial, pthread, mpi)
# Each file representing a collective performance evaluation from the set of arguments

NUM_ITERATIONS=100                            # number of iterations 
INITIAL_FILE_NAME=init.dat                    # input data file name
FINAL_FILE_NAME=final.dat                     # final state file name (re-used for all)
STACKED_FILE_NAME=all.raw                     # stacked matrix timestamps (unused)
MPI_SUMMARY_FILE_NAME=summary-mpi.txt         # summary data file for py-charts
PTH_SUMMARY_FILE_NAME=summary-pth.txt         # summary data file for py-charts
SERIAL_SUMMARY_FILE_NAME=summary-serial.txt   # summary data file for py-charts

PATH=$PATH:$(pwd)

echo "Hello, making input matrices..."
echo ""
# Loop iterates over a range of values to create the matrix sizes
for (( i=1000; i<=16000; i=i*2 ))
do
    ./make-2d $i $i $i-$INITIAL_FILE_NAME
    echo "NxN matrix of size: $i created"
done

echo ""
echo "=== About to run the application for the serial version ==="
echo ""
echo "Running Serial Stencil Calculations..."
for (( i=1000; i<=16000; i=i*2 ))
do
    echo "==== ./stencil-2d $NUM_ITERATIONS $i-$INITIAL_FILE_NAME $FINAL_FILE_NAME $SERIAL_SUMMARY_FILE_NAME ===="
    echo ""

    ./stencil-2d $NUM_ITERATIONS $i-$INITIAL_FILE_NAME $FINAL_FILE_NAME $SERIAL_SUMMARY_FILE_NAME
done

echo ""
echo "=== About to run the application for the pthread version ==="
echo ""
echo "Running Pthread Stencil Calculations..."
for (( i=1000; i<=16000; i=i*2 ))
do
    # Second loop iterates over a range of values for j
    for j in 1 2 4 8 16
    do
        echo "==== EXECUTING: ./pth-stencil-2d $NUM_ITERATIONS $i-$INITIAL_FILE_NAME $FINAL_FILE_NAME 1 $j $PTH_SUMMARY_FILE_NAME ===="
        echo ""

        ./pth-stencil-2d $NUM_ITERATIONS $i-$INITIAL_FILE_NAME $FINAL_FILE_NAME 1 $j $PTH_SUMMARY_FILE_NAME
    done
done

echo "=== About to run the application for the MPI version ==="
echo ""
echo "Running MPI Stencil Calculations..."
for (( i=1000; i<=16000; i=i*2 ))
do
    # Second loop iterates over a range of values for j
    for j in 1 2 4 8 16
    do
        echo "==== EXECUTING: mpirun -np $j --oversubscribe ./mpi-stencil-2d $NUM_ITERATIONS $i-$INITIAL_FILE_NAME $FINAL_FILE_NAME 1 $MPI_SUMMARY_FILE_NAME ===="
        echo ""

        mpirun -np $j --oversubscribe ./mpi-stencil-2d $NUM_ITERATIONS $i-$INITIAL_FILE_NAME $FINAL_FILE_NAME 1 $MPI_SUMMARY_FILE_NAME
    done
done

echo "=== ALL EXECUTIONS COMPLETE ==="

