# MPI Stencil

This directory is the source code for an experiment in parallel computing utilizing pthreads, MPI, and serial techniques to execute a 9-point stencil calculation upon a two dimensional array under a variety of matrix sizes and calculation iterations

## Project Details

- Developed in C

## Getting Started

- To compile

```
$ make
```

- To execute MPI

```
$ mpirun -np <num_cores> ./mpi-stencil-2d <in_file> <out_file> <debug level> <summary-file-name (optional)> <all-stacked-file-name.raw (optional)>
```

- To execute with MPI oversubscibe

```
$ mpirun -np <num_cores> --oversubscribe ./mpi-stencil-2d <num_iterations> <in_file> <out_file> <debug level> <summary-file-name (optional)> <all-stacked-file-name.raw (optional)>
```

- To load MPI on expanse

```
$ module load cpu/0.15.4 gcc/9.2.0 openmpi/4.1.1
```

- To release permissions for a bash script

```
$ chmod +x <your_script_file.sh>
```
