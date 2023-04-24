CC=gcc
MCC=mpicc
OMP=-fopenmp
CFLAGS=-g -Wall -Wstrict-prototypes -std=gnu99 -pthread
LFLAGS=-lm -pthread
PROGS=print-2d make-2d pth-stencil-2d stencil-2d mpi-stencil-2d
all:	$(PROGS)
print-2d: utilities.o print-2d.o
		$(CC) $(LFLAGS) -o print-2d utilities.o print-2d.o
make-2d: utilities.o make-2d.o
		$(CC) $(LFLAGS) -o make-2d utilities.o make-2d.o
pth-stencil-2d: utilities.o pth-stencil-2d.o 
		$(CC) $(LFLAGS) -o pth-stencil-2d utilities.o pth-stencil-2d.o
stencil-2d: utilities.o stencil-2d.o
		$(CC) $(LFLAGS) -o stencil-2d utilities.o stencil-2d.o
mpi-stencil-2d: utilities.o mpi-stencil-2d.o MyMPI.o
		$(MCC) $(OMP) $(LFLAGS) -o mpi-stencil-2d utilities.o mpi-stencil-2d.o MyMPI.o
utilities.o: utilities.c utilities.h
		$(MCC) $(CFLAGS) -c utilities.c
MyMPI.o: MyMPI.c MyMPI.h
		$(MCC) $(CFLAGS) -c MyMPI.c
print-2d.o: print-2d.c utilities.h
		$(CC) $(CFLAGS) -c print-2d.c
make-2d.o: make-2d.c utilities.h
		$(CC) $(CFLAGS) -c make-2d.c
pth-stencil-2d.o: pth-stencil-2d.c utilities.h timer.h
		$(CC) $(CFLAGS) -c pth-stencil-2d.c
stencil-2d.o: stencil-2d.c utilities.h
		$(CC) $(CFLAGS) -c stencil-2d.c
mpi-stencil-2d.o: mpi-stencil-2d.c utilities.h timer.h MyMPI.h
		$(MCC) $(CFLAGS) -c mpi-stencil-2d.c
clean: 
	rm -f *.o core* $(PROGS)