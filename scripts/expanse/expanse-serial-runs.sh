#!/bin/bash
#SBATCH --job-name="serial-stencil-2d-runs"
#SBATCH --output="serial-stencil-console-output.%j.%N.out"
#SBATCH --partition=shared
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=64GB
#SBATCH --account=ccu107
#SBATCH --export=ALL
#SBATCH -t 00:08:00        

# note about time above, don't use more than 5 minutes for a run
# **Here we perform 5 different executions & creating the matrices, thus more time**
# set problem size and/or iterations such that on 1 thread, takes < 5 minutes
# think about the size of memory, add some padding and set 
# --cpus-per-task=, should be set to the number of threads you may want

# DO NOT USE THE COMPUTE PARTITION
module purge 

# set whatever modules you need 
module load cpu
module load slurm
module load gcc

#Run the job, stencil-2d usage: ./stencil-2d <num iterations> <input file> <summary_file_name (optional)> <all-stacked-file-name.raw (optional)>
    
NUM_ITERATIONS=100                     # number of iterations 
INITIAL_FILE_NAME=init.dat             # input data file name
FINAL_FILE_NAME=final.dat              # final state file name
STACKED_FILE_NAME=all.raw              # stacked matrix timestmaps (*unused here)
SUMMARY_FILE_NAME=summary-serial.txt   # summary data file for py-charts

PATH=/scratch/$USER/job_$SLURM_JOB_ID

# lscpu > lscpu.%j.%N.out
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

# copy data files out of scratch space to current working directory
/usr/bin/cp $PATH/$INITIAL_FILE_NAME ./
/usr/bin/cp $PATH/$FINAL_FILE_NAME ./
/usr/bin/cp $PATH/$SUMMARY_FILE_NAME ./
#/usr/bin/cp $PATH/$ALL ./ (*unused here)

# how to tell your usage
# make sure this module is loaded: module load sdsc
# then run this command:
# expanse-client project ccu107 -p | grep $USER