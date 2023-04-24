#!/bin/bash
#SBATCH --job-name="pth-stencil-2d"
#SBATCH --output="pth-stencil-console-output.%j.%N.out"
#SBATCH --partition=shared
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64GB
#SBATCH --account=ccu107
#SBATCH --export=ALL
#SBATCH -t 00:15:00        

# note about time above, don't use more than 5 minutes for a run
# set problem size and/or iterations such that on 1 thread, takes < 5 minutes
# **Here we perform 25 executions & create the matrices, thus longer time**
# think about the size of memory, add some padding and set 

# --cpus-per-task=, should be set to the number of threads you may want

# DO NOT USE THE COMPUTE PARTITION
module purge

# set whatever modules you need 
module load cpu
module load slurm
module load gcc

#Run the job, pth-stencil-2d usage: ./pth-stencil-2d <num iterations> <input file> <output file> <debug level> <num threads> <summary-file-name.raw (optional)> <all-stacked-file-name.raw (optional)>
NUM_ITERATIONS=100                  # number of iterations 
INITIAL_FILE_NAME=init.dat          # input data file name
FINAL_FILE_NAME=final.dat           # final state file name
STACKED_FILE_NAME=all.raw           # stacked matrix timestamps  (*unused here)
SUMMARY_FILE_NAME=summary-pth.txt   # summary data file for py-charts

PATH=/scratch/$USER/job_$SLURM_JOB_ID

# lscpu > lscpu.%j.%N.out
echo 'Hello, about to run the application for the pthread version'
echo ''
echo "Making input matrices..."
# Loop iterates over a range of values to create the matrix sizes
for (( i=1000; i<=16000; i=i*2 ))
do
    ./make-2d $i $i $i-$INITIAL_FILE_NAME
    echo "NxN matrix of size: $i created"
done
echo ""
echo "Running Pthread Stencil Calculations ..."
for (( i=1000; i<=16000; i=i*2 ))
do
    # Second loop iterates over a range of values for j
    for j in 1 2 4 8 16
    do
        echo "==== EXECUTING: ./pth-stencil-2d $NUM_ITERATIONS $i-$INITIAL_FILE_NAME $FINAL_FILE_NAME 1 $j $SUMMARY_FILE_NAME ===="
        echo ""

        ./pth-stencil-2d $NUM_ITERATIONS $i-$INITIAL_FILE_NAME $FINAL_FILE_NAME 1 $j $SUMMARY_FILE_NAME
    done
done

echo "==== All EXECUTIONS COMPLETE ===="

# copy data files out of scratch space to current working directory
/usr/bin/cp $PATH/$INITIAL_FILE_NAME ./
/usr/bin/cp $PATH/$FINAL_FILE_NAME ./
/usr/bin/cp $PATH/$SUMMARY_FILE_NAME ./
#/usr/bin/cp $PATH/$ALL ./         (*unused here)

# how to tell your usage
# make sure this module is loaded: module load sdsc
# then run this command:
# expanse-client project ccu107 -p | grep $USER
