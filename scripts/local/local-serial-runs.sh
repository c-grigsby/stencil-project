#!/bin/bash

NUM_ITERATIONS=100                     # number of iterations 
INITIAL_FILE_NAME=init.dat             # input data file name
FINAL_FILE_NAME=final.dat              # final state file name
STACKED_FILE_NAME=all.raw              # stacked matrix timestmaps (unused here)
SUMMARY_FILE_NAME=summary-serial.txt   # summary data file for py-charts

PATH=$PATH:$(pwd)

echo 'Hello, about to run the application for the serial version'
echo ''
echo "Making input matrices..."
# Loop iterates over a range of values to create the matrix sizes
for (( i=1000; i<=16000; i=i*2 ))
do
    ./make-2d $i $i $i-$INITIAL_FILE_NAME
    echo "NxN matrix of size: $i created"
done
echo ""
echo "Running Serial Stencil Calculations ..."
for (( i=1000; i<=16000; i=i*2 ))
do
    echo "==== ./stencil-2d $NUM_ITERATIONS $i-$INITIAL_FILE_NAME $FINAL_FILE_NAME $SUMMARY_FILE_NAME ===="
    echo ""
    ./stencil-2d $NUM_ITERATIONS $i-$INITIAL_FILE_NAME $FINAL_FILE_NAME $SUMMARY_FILE_NAME
done

echo "==== All EXECUTIONS COMPLETE ===="

