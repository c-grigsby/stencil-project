# MPI Stencil

Team Members: Christopher Grigsby & Jordan Drakos

## Getting Started with C

- To compile

```
$ make
```

- To execute MPI

```
$ mpirun -np <num_cores> ./mpi-floyds <in_file> <out_file> <debug level> <summary-file-name (optional)> <all-stacked-file-name.raw (optional)>
```

- To execute with MPI oversubscibe

```
$ mpirun -np <num_cores> --oversubscribe ./mpi-stencil-2d <num_iterations> <in_file> <out_file> <debug level> <summary-file-name (optional)> <all-stacked-file-name.raw (optional)>
```

- To execute on MPI on expanse

```
$ module load cpu/0.15.4 gcc/9.2.0 openmpi/4.1.1
```

- To release permissions for a bash script

```
$ chmod +x <your_script_file.sh>
```
