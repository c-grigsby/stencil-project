/*
 *   MyMPI.c -- A library of matrix/vector
 *   input/output/redistribution functions
 *
 *   Programmed by Michael J. Quinn
 *
 *   Last modification by the author: 4 September 2002
 * 
 *   Modified by c-grigsby: April 2023 
 *   INPUT FUNCTIONS: read_row_striped_matrix_stencil, exchange_row_striped_matix_stencil_data
 *   OUTPUT FUNCTIONS: write_row_striped_matrix_stencil, write_row_striped_matrix_stencil_noMeta
 *   Note: This library was not written well and is outdated, good luck
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "MyMPI.h"
// @global
int x = 0;

/***************** MISCELLANEOUS FUNCTIONS *****************/

/*
 *   Given MPI_Datatype 't', function 'get_size' returns the
 *   size of a single datum of that data type.
 */

int get_size(MPI_Datatype t)
{
   if (t == MPI_BYTE)
      return sizeof(char);
   if (t == MPI_DOUBLE)
      return sizeof(double);
   if (t == MPI_FLOAT)
      return sizeof(float);
   if (t == MPI_INT)
      return sizeof(int);
   printf("Error: Unrecognized argument to 'get_size'\n");
   fflush(stdout);
   MPI_Abort(MPI_COMM_WORLD, TYPE_ERROR);
   return 0;
}

/*
 *   Function 'my_malloc' is called when a process wants
 *   to allocate some space from the heap. If the memory
 *   allocation fails, the process prints an error message
 *   and then aborts execution of the program.
 */

void *my_malloc(
    int id,    /* IN - Process rank */
    int bytes) /* IN - Bytes to allocate */
{
   void *buffer;
   if ((buffer = malloc((size_t)bytes)) == NULL)
   {
      printf("Error: Malloc failed for process %d\n", id);
      fflush(stdout);
      MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
   }
   return buffer;
}

/*
 *   Function 'terminate' is called when the program should
 *   not continue execution, due to an error condition that
 *   all of the processes are aware of. Process 0 prints the
 *   error message passed as an argument to the function.
 *
 *   All processes must invoke this function together!
 */

void terminate(
    int id,              /* IN - Process rank */
    char *error_message) /* IN - Message to print */
{
   if (!id)
   {
      printf("Error: %s\n", error_message);
      fflush(stdout);
   }
   MPI_Finalize();
   exit(-1);
}

/************ DATA DISTRIBUTION FUNCTIONS ******************/

/*
 *   This function creates the count and displacement arrays
 *   needed by scatter and gather functions, when the number
 *   of elements send/received to/from other processes
 *   varies.
 */

void create_mixed_xfer_arrays(
    int id,      /* IN - Process rank */
    int p,       /* IN - Number of processes */
    int n,       /* IN - Total number of elements */
    int **count, /* OUT - Array of counts */
    int **disp)  /* OUT - Array of displacements */
{

   int i;

   *count = my_malloc(id, p * sizeof(int));
   *disp = my_malloc(id, p * sizeof(int));
   (*count)[0] = BLOCK_SIZE(0, p, n);
   (*disp)[0] = 0;
   for (i = 1; i < p; i++)
   {
      (*disp)[i] = (*disp)[i - 1] + (*count)[i - 1];
      (*count)[i] = BLOCK_SIZE(i, p, n);
   }
}

/*
 *   This function creates the count and displacement arrays
 *   needed in an all-to-all exchange, when a process gets
 *   the same number of elements from every other process.
 */

void create_uniform_xfer_arrays(
    int id,      /* IN - Process rank */
    int p,       /* IN - Number of processes */
    int n,       /* IN - Number of elements */
    int **count, /* OUT - Array of counts */
    int **disp)  /* OUT - Array of displacements */
{

   int i;

   *count = my_malloc(id, p * sizeof(int));
   *disp = my_malloc(id, p * sizeof(int));
   (*count)[0] = BLOCK_SIZE(id, p, n);
   (*disp)[0] = 0;
   for (i = 1; i < p; i++)
   {
      (*disp)[i] = (*disp)[i - 1] + (*count)[i - 1];
      (*count)[i] = BLOCK_SIZE(id, p, n);
   }
}

/*
 *   This function is used to transform a vector from a
 *   block distribution to a replicated distribution within a
 *   communicator.
 */

void replicate_block_vector(
    void *ablock,       /* IN - Block-distributed vector */
    int n,              /* IN - Elements in vector */
    void *arep,         /* OUT - Replicated vector */
    MPI_Datatype dtype, /* IN - Element type */
    MPI_Comm comm)      /* IN - Communicator */
{
   int *cnt;  /* Elements contributed by each process */
   int *disp; /* Displacement in concatenated array */
   int id;    /* Process id */
   int p;     /* Processes in communicator */

   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &id);
   create_mixed_xfer_arrays(id, p, n, &cnt, &disp);
   MPI_Allgatherv(ablock, cnt[id], dtype, arep, cnt,
                  disp, dtype, comm);
   free(cnt);
   free(disp);
}

/********************* INPUT FUNCTIONS *********************/

/*
 *   Function 'read_checkerboard_matrix' reads a matrix from
 *   a file. The first two elements of the file are integers
 *   whose values are the dimensions of the matrix ('m' rows
 *   and 'n' columns). What follows are 'm'*'n' values
 *   representing the matrix elements stored in row-major
 *   order.  This function allocates blocks of the matrix to
 *   the MPI processes.
 *
 *   The number of processes must be a square number.
 */

void read_checkerboard_matrix(
    char *s,            /* IN - File name */
    void ***subs,       /* OUT - 2D array */
    void **storage,     /* OUT - Array elements */
    MPI_Datatype dtype, /* IN - Element type */
    int *m,             /* OUT - Array rows */
    int *n,             /* OUT - Array cols */
    MPI_Comm grid_comm) /* IN - Communicator */
{
   void *buffer;       /* File buffer */
   int coords[2];      /* Coords of proc receiving
                          next row of matrix */
   int datum_size;     /* Bytes per elements */
   int dest_id;        /* Rank of receiving proc */
   int grid_coord[2];  /* Process coords */
   int grid_id;        /* Process rank */
   int grid_period[2]; /* Wraparound */
   int grid_size[2];   /* Dimensions of grid */
   int i, j, k;
   FILE *infileptr;   /* Input file pointer */
   void *laddr;       /* Used when proc 0 gets row */
   int local_cols;    /* Matrix cols on this proc */
   int local_rows;    /* Matrix rows on this proc */
   void **lptr;       /* Pointer into 'subs' */
   int p;             /* Number of processes */
   void *raddr;       /* Address of first element
                         to send */
   void *rptr;        /* Pointer into 'storage' */
   MPI_Status status; /* Results of read */

   MPI_Comm_rank(grid_comm, &grid_id);
   MPI_Comm_size(grid_comm, &p);
   datum_size = get_size(dtype);

   /* Process 0 opens file, gets number of rows and
      number of cols, and broadcasts this information
      to the other processes. */

   if (grid_id == 0)
   {
      infileptr = fopen(s, "r");
      if (infileptr == NULL)
         *m = 0;
      else
      {
         fread(m, sizeof(int), 1, infileptr);
         fread(n, sizeof(int), 1, infileptr);
      }
   }
   MPI_Bcast(m, 1, MPI_INT, 0, grid_comm);

   if (!(*m))
      MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);

   MPI_Bcast(n, 1, MPI_INT, 0, grid_comm);

   /* Each process determines the size of the submatrix
      it is responsible for. */

   MPI_Cart_get(grid_comm, 2, grid_size, grid_period,
                grid_coord);
   local_rows = BLOCK_SIZE(grid_coord[0], grid_size[0], *m);
   local_cols = BLOCK_SIZE(grid_coord[1], grid_size[1], *n);

   /* Dynamically allocate two-dimensional matrix 'subs' */

   *storage = my_malloc(grid_id,
                        local_rows * local_cols * datum_size);
   *subs = (void **)my_malloc(grid_id, local_rows * PTR_SIZE);
   lptr = (void *)*subs;
   rptr = (void *)*storage;
   for (i = 0; i < local_rows; i++)
   {
      *(lptr++) = (void *)rptr;
      rptr += local_cols * datum_size;
   }

   /* Grid process 0 reads in the matrix one row at a time
      and distributes each row among the MPI processes. */

   if (grid_id == 0)
      buffer = my_malloc(grid_id, *n * datum_size);

   /* For each row of processes in the process grid... */
   for (i = 0; i < grid_size[0]; i++)
   {
      coords[0] = i;

      /* For each matrix row controlled by this proc row...*/
      for (j = 0; j < BLOCK_SIZE(i, grid_size[0], *m); j++)
      {

         /* Read in a row of the matrix */

         if (grid_id == 0)
         {
            fread(buffer, datum_size, *n, infileptr);
         }

         /* Distribute it among process in the grid row */

         for (k = 0; k < grid_size[1]; k++)
         {
            coords[1] = k;

            /* Find address of first element to send */
            raddr = buffer +
                    BLOCK_LOW(k, grid_size[1], *n) * datum_size;

            /* Determine the grid ID of the process getting
               the subrow */
            MPI_Cart_rank(grid_comm, coords, &dest_id);

            /* Process 0 is responsible for sending...*/
            if (grid_id == 0)
            {

               /* It is sending (copying) to itself */
               if (dest_id == 0)
               {
                  laddr = (*subs)[j];
                  memcpy(laddr, raddr,
                         local_cols * datum_size);

                  /* It is sending to another process */
               }
               else
               {
                  MPI_Send(raddr,
                           BLOCK_SIZE(k, grid_size[1], *n), dtype,
                           dest_id, 0, grid_comm);
               }

               /* Process 'dest_id' is responsible for
                  receiving... */
            }
            else if (grid_id == dest_id)
            {
               MPI_Recv((*subs)[j], local_cols, dtype, 0,
                        0, grid_comm, &status);
            }
         }
      }
   }
   if (grid_id == 0)
      free(buffer);
}

/*
 *   Function 'read_col_striped_matrix' reads a matrix from a
 *   file.  The first two elements of the file are integers
 *   whose values are the dimensions of the matrix ('m' rows
 *   and 'n' columns).  What follows are 'm'*'n' values
 *   representing the matrix elements stored in row-major
 *   order.  This function allocates blocks of columns of the
 *   matrix to the MPI processes.
 */

void read_col_striped_matrix(
    char *s,            /* IN - File name */
    void ***subs,       /* OUT - 2-D array */
    void **storage,     /* OUT - Array elements */
    MPI_Datatype dtype, /* IN - Element type */
    int *m,             /* OUT - Rows */
    int *n,             /* OUT - Cols */
    MPI_Comm comm)      /* IN - Communicator */
{
   void *buffer;   /* File buffer */
   int datum_size; /* Size of matrix element */
   int i;
   int id;          /* Process rank */
   FILE *infileptr; /* Input file ptr */
   int local_cols;  /* Cols on this process */
   void **lptr;     /* Pointer into 'subs' */
   void *rptr;      /* Pointer into 'storage' */
   int p;           /* Number of processes */
   int *send_count; /* Each proc's count */
   int *send_disp;  /* Each proc's displacement */

   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &id);
   datum_size = get_size(dtype);

   /* Process p-1 opens file, gets number of rows and
      cols, and broadcasts this info to other procs. */

   if (id == (p - 1))
   {
      infileptr = fopen(s, "r");
      if (infileptr == NULL)
         *m = 0;
      else
      {
         fread(m, sizeof(int), 1, infileptr);
         fread(n, sizeof(int), 1, infileptr);
      }
   }
   MPI_Bcast(m, 1, MPI_INT, p - 1, comm);

   if (!(*m))
      MPI_Abort(comm, OPEN_FILE_ERROR);

   MPI_Bcast(n, 1, MPI_INT, p - 1, comm);

   local_cols = BLOCK_SIZE(id, p, *n);

   /* Dynamically allocate two-dimensional matrix 'subs' */
   *storage = my_malloc(id, *m * local_cols * datum_size);
   *subs = (void **)my_malloc(id, *m * PTR_SIZE);
   lptr = (void *)*subs;
   rptr = (void *)*storage;
   for (i = 0; i < *m; i++)
   {
      *(lptr++) = (void *)rptr;
      rptr += local_cols * datum_size;
   }

   /* Process p-1 reads in the matrix one row at a time and
      distributes each row among the MPI processes. */

   if (id == (p - 1))
      buffer = my_malloc(id, *n * datum_size);
   create_mixed_xfer_arrays(id, p, *n, &send_count, &send_disp);
   for (i = 0; i < *m; i++)
   {
      if (id == (p - 1))
         fread(buffer, datum_size, *n, infileptr);
      MPI_Scatterv(buffer, send_count, send_disp, dtype,
                   (*storage) + i * local_cols * datum_size, local_cols,
                   dtype, p - 1, comm);
   }
   free(send_count);
   free(send_disp);
   if (id == (p - 1))
      free(buffer);
}

/*
 *   Process p-1 opens a file and inputs a two-dimensional
 *   matrix, reading and distributing blocks of rows to the
 *   other processes.
 */

int read_row_striped_matrix(
    char *s,            /* IN - File name */
    void ***subs,       /* OUT - 2D submatrix indices */
    void **storage,     /* OUT - Submatrix stored here */
    MPI_Datatype dtype, /* IN - Matrix element type */
    int *m,             /* OUT - Matrix rows */
    int *n,             /* OUT - Matrix cols */
    MPI_Comm comm)      /* IN - Communicator */
{
   int datum_size; /* Size of matrix element */
   int i;
   int id;            /* Process rank */
   FILE *infileptr;   /* Input file pointer */
   int local_rows;    /* Rows on this proc */
   void **lptr;       /* Pointer into 'subs' */
   int p;             /* Number of processes */
   void *rptr;        /* Pointer into 'storage' */
   MPI_Status status; /* Result of receive */
   int x;             /* Result of read */

   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &id);
   datum_size = get_size(dtype);

   /* Process p-1 opens file, gets number of rows and
      cols, and broadcasts this info to other procs. */
   if (id == (p - 1))
   {
      infileptr = fopen(s, "r");
      if (infileptr == NULL)
         *m = 0;
      else
      {
         fread(m, sizeof(int), 1, infileptr);
         fread(n, sizeof(int), 1, infileptr);
      }
   }
   MPI_Bcast(m, 1, MPI_INT, p - 1, comm);

   if (!(*m))
   {
      MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);
   }

   MPI_Bcast(n, 1, MPI_INT, p - 1, comm);

   local_rows = BLOCK_SIZE(id, p, *m);

   /* Dynamically allocate matrix. Allow double subscripting
      through 'a'. */

   *storage = (void *)my_malloc(id,
                                local_rows * *n * datum_size);

   *subs = (void **)my_malloc(id, local_rows * PTR_SIZE);

   lptr = (void *)&(*subs[0]);
   rptr = (void *)*storage;
   for (i = 0; i < local_rows; i++)
   {
      *(lptr++) = (void *)rptr;
      rptr += *n * datum_size;
   }

   /* Process p-1 reads blocks of rows from file and
      sends each block to the correct destination process.
      The last block it keeps. */

   if (id == (p - 1))
   {
      for (i = 0; i < p - 1; i++)
      {
         x = fread(*storage, datum_size,
                   BLOCK_SIZE(i, p, *m) * *n, infileptr);
         MPI_Ssend(*storage, BLOCK_SIZE(i, p, *m) * *n, dtype,
                   i, DATA_MSG, comm);
      }
      x = fread(*storage, datum_size, local_rows * *n,
                infileptr);
      fclose(infileptr);
      return x;
   }
   else
      MPI_Recv(*storage, local_rows * *n, dtype, p - 1,
               DATA_MSG, comm, &status);

   return 0;
}

/*
 *   Process p-1 opens a file and inputs a two-dimensional
 *   matrix, reading and distributing blocks of rows to the
 *   other processes taking into consideration the additional 
 *   rows required for a 9 point stencil.
 */
void read_row_striped_matrix_stencil(
    char *s,            /* IN - File name */
    void ***subs,       /* OUT - 2D submatrix indices */
    void **storage,     /* OUT - Submatrix stored here */
    MPI_Datatype dtype, /* IN - Matrix element type */
    int *m,             /* OUT - Matrix rows */
    int *n,             /* OUT - Matrix cols */
    MPI_Comm comm)      /* IN - Communicator */
{
   int datum_size;      /* Size of matrix element */
   int i;               /* loop variable */
   int id;              /* Process rank */
   FILE *infileptr;     /* Input file pointer */
   int local_rows;      /* Rows on this proc */
   void **lptr;         /* Pointer into 'subs' */
   int p;               /* Number of processes */
   void *rptr;          /* Pointer into 'storage' */
   MPI_Status status;   /* Result of receive */

   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &id);
   datum_size = get_size(dtype);

   // Process p-1 opens file, reads size of matrix, 
   // and broadcasts matrix dimensions to other processes
   if (id == (p - 1))
   {
      infileptr = fopen(s, "r");
      if (infileptr == NULL)
         *m = 0;
      else
      {
         fread(m, sizeof(int), 1, infileptr);
         fread(n, sizeof(int), 1, infileptr);
      }
   }

   MPI_Bcast(m, 1, MPI_INT, p - 1, comm);

   if (!(*m)) { MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR); }

   MPI_Bcast(n, 1, MPI_INT, p - 1, comm);

   local_rows = BLOCK_SIZE(id, p, (*m));

   // Additional number of rows needed for the stencil calculations
   int additional_halo_rows = 0;
   if (id == 0 || id == p - 1) {
      additional_halo_rows = 1;
   }
   else {
      additional_halo_rows = 2;
   }

   // Dynamically allocate matrix
   *storage = (void *)my_malloc(id, (local_rows + additional_halo_rows) * *n * datum_size);
   *subs = (void **)my_malloc(id, (local_rows + additional_halo_rows) * PTR_SIZE);

   lptr = (void *)&(*subs[0]);
   rptr = (void *)*storage;
   for (i = 0; i < (local_rows + additional_halo_rows); i++)
   {
      *(lptr++) = (void *)rptr;
      rptr += *n * datum_size;
   }

   // Process p-1 reads blocks of rows from file 
   // and sends each block to the correct destination process.
   // The last block it keeps.
   for (int i = 0; i < local_rows + additional_halo_rows; i++)
   {
      for (int j = 0; j < *n; j++)
      {
         ((double **)(*subs))[i][j] = -1.000;
      }
   }

   if (id == (p - 1))
   {
      for (i = 0; i < p - 1; i++)
      {
         // Read data from the file 
         x = fread(*storage, datum_size, (BLOCK_SIZE(i, p, *m)) * *n, infileptr);
         // Send to the i process
         MPI_Send(*storage, BLOCK_SIZE(i, p, *m) * *n, dtype, i, DATA_MSG, comm);
      }
      // Last process stores the another row for the stencil halo
      int i = 0;
      for (int j = 0; j < *n; j++) {
        ((double **)(*subs))[i][j] = -1.000;
      }
      x = fread(((double **)(*subs))[1], datum_size, local_rows * *n, infileptr);
      fclose(infileptr);
   }
   else
   {
      if (id == 0)
      {
         // Process 0 stores the first row and the stencil halo after it
         MPI_Recv(*storage, local_rows * *n, dtype, p - 1, DATA_MSG, comm, &status);
      }
      else
      {
         // Processes > 0 & < p-1 store data in the 2nd row, between rows for halo stencil
         MPI_Recv(((double **)(*subs))[1], local_rows * *n, dtype, p - 1, DATA_MSG, comm, &status);
      }
   }
}


/*
 *   Exchanges matrix data between processes for a 9-point stencil calculation
 */
void exchange_row_striped_matix_stencil_data(
    void **a,           /* IN - 2D array */
    MPI_Datatype dtype, /* IN - Matrix element type */
    int m,              /* IN - Matrix rows */
    int n,              /* IN - Matrix cols */
    MPI_Comm comm)      /* IN - Communicator */

{
   int id;              /* Process rank */
   int p;               /* Number of processes */
   int next;            /* Process id + 1 */
   int prev;            /* Process id - 1 */
   int local_rows;      /* rows local to the process */

   // Gets the comm and rank and assigns them to variable
   MPI_Comm_rank(MPI_COMM_WORLD, &id);
   MPI_Comm_size(MPI_COMM_WORLD, &p);
   local_rows = BLOCK_SIZE(id, p, (m));

   // Add additional rows for the stencil
   if (id == 0 || id == p - 1) { 
      local_rows += 1; 
   } 
   else {
      local_rows += 2;
   }

   // Determine stencil neighbors
   prev = id - 1;
   next = id + 1;
   if (id == 0) { prev = p - 1; }
   if (id == p - 1) { next = 0; }

   // Execute non-blocking sends & receives for stencil neighbors
   if (id == 0)
   {
      MPI_Request requests[2]; 
      // Send the row the next process will need
      MPI_Isend(a[local_rows - 2], n, dtype, next, 1, MPI_COMM_WORLD, &requests[1]);
      // Receive the stencil halo
      MPI_Irecv(a[local_rows - 1], n, dtype, next, 1, MPI_COMM_WORLD, &requests[0]);
      
   }
   else if (id > 0 && id < p - 1)
   {
      MPI_Request requests[4]; 
      // Receive the upper stencil halo
      MPI_Irecv(a[0], n, dtype, prev, 1, MPI_COMM_WORLD, &requests[0]);
      // Receive the lower stencil halo
      MPI_Irecv(a[local_rows - 1], n, dtype, next, 1, MPI_COMM_WORLD, &requests[1]);

      // Send first local row to the previous process
      MPI_Isend(a[1], n, dtype, prev, 1, MPI_COMM_WORLD, &requests[2]);
      // Send last local row the next process
      MPI_Isend(a[local_rows - 2], n, dtype, next, 1, MPI_COMM_WORLD, &requests[3]);
      MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);
   }
   else
   {
      MPI_Request requests[2]; 
      // Last process receives its upper halo for stencil
      MPI_Irecv(a[0], n, dtype, prev, 1, MPI_COMM_WORLD, &requests[0]);
      // Send first local row
      MPI_Isend(a[1], n, dtype, prev, 1, MPI_COMM_WORLD, &requests[1]);
      MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
   }
}

/*
 *   Open a file containing a vector, read its contents,
 *   and distributed the elements by block among the
 *   processes in a communicator.
 */

int read_block_vector(
    char *s,            /* IN - File name */
    void **v,           /* OUT - Subvector */
    MPI_Datatype dtype, /* IN - Element type */
    int *n,             /* OUT - Vector length */
    MPI_Comm comm)      /* IN - Communicator */
{
   int datum_size; /* Bytes per element */
   int i;
   FILE *infileptr;   /* Input file pointer */
   int local_els;     /* Elements on this proc */
   MPI_Status status; /* Result of receive */
   int id;            /* Process rank */
   int p;             /* Number of processes */
   int x;             /* Result of read */

   datum_size = get_size(dtype);
   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &id);

   /* Process p-1 opens file, determines number of vector
      elements, and broadcasts this value to the other
      processes. */

   if (id == (p - 1))
   {
      infileptr = fopen(s, "r");
      if (infileptr == NULL)
         *n = 0;
      else
         fread(n, sizeof(int), 1, infileptr);
   }
   MPI_Bcast(n, 1, MPI_INT, p - 1, comm);
   if (!*n)
   {
      if (!id)
      {
         printf("Input file '%s' cannot be opened\n", s);
         fflush(stdout);
      }
   }

   /* Block mapping of vector elements to processes */

   local_els = BLOCK_SIZE(id, p, *n);

   /* Dynamically allocate vector. */

   *v = my_malloc(id, local_els * datum_size);
   if (id == (p - 1))
   {
      for (i = 0; i < p - 1; i++)
      {
         x = fread(*v, datum_size, BLOCK_SIZE(i, p, *n),
                   infileptr);
         MPI_Send(*v, BLOCK_SIZE(i, p, *n), dtype, i, DATA_MSG,
                  comm);
      }
      x = fread(*v, datum_size, BLOCK_SIZE(id, p, *n),
                infileptr);
      fclose(infileptr);
      return x;
   }
   else
   {
      MPI_Recv(*v, BLOCK_SIZE(id, p, *n), dtype, p - 1, DATA_MSG,
               comm, &status);
   }
   return 0;
}

/*   Open a file containing a vector, read its contents,
     and replicate the vector among all processes in a
     communicator. */

void read_replicated_vector(
    char *s,            /* IN - File name */
    void **v,           /* OUT - Vector */
    MPI_Datatype dtype, /* IN - Vector type */
    int *n,             /* OUT - Vector length */
    MPI_Comm comm)      /* IN - Communicator */
{
   int datum_size;  /* Bytes per vector element */
   int id;          /* Process rank */
   FILE *infileptr; /* Input file pointer */
   int p;           /* Number of processes */

   MPI_Comm_rank(comm, &id);
   MPI_Comm_size(comm, &p);
   datum_size = get_size(dtype);
   if (id == (p - 1))
   {
      infileptr = fopen(s, "r");
      if (infileptr == NULL)
         *n = 0;
      else
         fread(n, sizeof(int), 1, infileptr);
   }
   MPI_Bcast(n, 1, MPI_INT, p - 1, MPI_COMM_WORLD);
   if (!*n)
      terminate(id, "Cannot open vector file");

   *v = my_malloc(id, *n * datum_size);

   if (id == (p - 1))
   {
      fread(*v, datum_size, *n, infileptr);
      fclose(infileptr);
   }
   MPI_Bcast(*v, *n, dtype, p - 1, MPI_COMM_WORLD);
}

/******************** OUTPUT FUNCTIONS ********************/

/*
 *   Print elements of a doubly-subscripted array.
 */

void print_submatrix(
    void **a,           /* OUT - Doubly-subscripted array */
    MPI_Datatype dtype, /* OUT - Type of array elements */
    int rows,           /* OUT - Matrix rows */
    int cols)           /* OUT - Matrix cols */
{
   int i, j;

   for (i = 0; i < rows; i++)
   {
      for (j = 0; j < cols; j++)
      {
         if (dtype == MPI_DOUBLE)
            printf("%6.3f ", ((double **)a)[i][j]);
         else
         {
            if (dtype == MPI_FLOAT)
               printf("%6.3f ", ((float **)a)[i][j]);
            else if (dtype == MPI_INT)
               printf("%6d ", ((int **)a)[i][j]);
         }
      }
      putchar('\n');
   }
}

/*
 *   Writes elements of a doubly-subscripted array to a file
 */
void write_submatrix(
    char *file_name,
    void **a,           /* OUT - Doubly-subscripted array */
    MPI_Datatype dtype, /* OUT - Type of array elements */
    int rows,           /* OUT - Matrix rows */
    int cols)           /* OUT - Matrix cols */
{
   int i, j;

   FILE *fp;
   fp = fopen(file_name, "ab");
   if (fp == NULL)
   {
      printf("EXIT_FAILURE: Error opening file\n");
      exit(EXIT_FAILURE);
   }

   for (i = 0; i < rows; i++)
   {
      for (j = 0; j < cols; j++)
      {
         if (dtype == MPI_DOUBLE)
         {
            // Dereference double-pointed array
            double value = ((double **)a)[i][j];
            fwrite(&value, sizeof(double), 1, fp);
         }
         else
         {
            if (dtype == MPI_FLOAT)
            {
               // Dereference double-pointed array
               float value = ((float **)a)[i][j];
               fwrite(&value, sizeof(float), 1, fp);
            }
            else if (dtype == MPI_INT)
            {
               // Dereference double-pointed array
               int value = ((int **)a)[i][j];
               fwrite(&value, sizeof(int), 1, fp);
            }
         }
      }
      // putchar('\n');
   }
   fclose(fp);
}

/*
 *   Print elements of a singly-subscripted array.
 */

void print_subvector(
    void *a,            /* IN - Array pointer */
    MPI_Datatype dtype, /* IN - Array type */
    int n)              /* IN - Array size */
{
   int i;

   for (i = 0; i < n; i++)
   {
      if (dtype == MPI_DOUBLE)
         printf("%6.3f ", ((double *)a)[i]);
      else
      {
         if (dtype == MPI_FLOAT)
            printf("%6.3f ", ((float *)a)[i]);
         else if (dtype == MPI_INT)
            printf("%6d ", ((int *)a)[i]);
      }
   }
}

/*
 *   Print a matrix distributed checkerboard fashion among
 *   the processes in a communicator.
 */

void print_checkerboard_matrix(
    void **a,           /* IN -2D matrix */
    MPI_Datatype dtype, /* IN -Matrix element type */
    int m,              /* IN -Matrix rows */
    int n,              /* IN -Matrix columns */
    MPI_Comm grid_comm) /* IN - Communicator */
{
   void *buffer;       /* Room to hold 1 matrix row */
   int coords[2];      /* Grid coords of process
                          sending elements */
   int datum_size;     /* Bytes per matrix element */
   int els;            /* Elements received */
   int grid_coords[2]; /* Coords of this process */
   int grid_id;        /* Process rank in grid */
   int grid_period[2]; /* Wraparound */
   int grid_size[2];   /* Dims of process grid */
   int i, j, k;
   void *laddr;       /* Where to put subrow */
   int local_cols;    /* Matrix cols on this proc */
   int p;             /* Number of processes */
   int src;           /* ID of proc with subrow */
   MPI_Status status; /* Result of receive */

   MPI_Comm_rank(grid_comm, &grid_id);
   MPI_Comm_size(grid_comm, &p);
   datum_size = get_size(dtype);

   MPI_Cart_get(grid_comm, 2, grid_size, grid_period,
                grid_coords);
   local_cols = BLOCK_SIZE(grid_coords[1], grid_size[1], n);

   if (!grid_id)
      buffer = my_malloc(grid_id, n * datum_size);

   /* For each row of the process grid */
   for (i = 0; i < grid_size[0]; i++)
   {
      coords[0] = i;

      /* For each matrix row controlled by the process row */
      for (j = 0; j < BLOCK_SIZE(i, grid_size[0], m); j++)
      {

         /* Collect the matrix row on grid process 0 and
            print it */
         if (!grid_id)
         {
            for (k = 0; k < grid_size[1]; k++)
            {
               coords[1] = k;
               MPI_Cart_rank(grid_comm, coords, &src);
               els = BLOCK_SIZE(k, grid_size[1], n);
               laddr = buffer +
                       BLOCK_LOW(k, grid_size[1], n) * datum_size;
               if (src == 0)
               {
                  memcpy(laddr, a[j], els * datum_size);
               }
               else
               {
                  MPI_Recv(laddr, els, dtype, src, 0,
                           grid_comm, &status);
               }
            }
            print_subvector(buffer, dtype, n);
            putchar('\n');
         }
         else if (grid_coords[0] == i)
         {
            MPI_Send(a[j], local_cols, dtype, 0, 0,
                     grid_comm);
         }
      }
   }
   if (!grid_id)
   {
      free(buffer);
      putchar('\n');
   }
}

/*
 *   Print a matrix that has a columnwise-block-striped data
 *   decomposition among the elements of a communicator.
 */

void print_col_striped_matrix(
    void **a,           /* IN - 2D array */
    MPI_Datatype dtype, /* IN - Type of matrix elements */
    int m,              /* IN - Matrix rows */
    int n,              /* IN - Matrix cols */
    MPI_Comm comm)      /* IN - Communicator */
{
   // MPI_Status status;     /* Result of receive */
   int datum_size; /* Bytes per matrix element */
   void *buffer;   /* Enough room to hold 1 row */
   int i;
   int id;         /* Process rank */
   int p;          /* Number of processes */
   int *rec_count; /* Elements received per proc */
   int *rec_disp;  /* Offset of each proc's block */

   MPI_Comm_rank(comm, &id);
   MPI_Comm_size(comm, &p);
   datum_size = get_size(dtype);
   create_mixed_xfer_arrays(id, p, n, &rec_count, &rec_disp);

   if (!id)
      buffer = my_malloc(id, n * datum_size);

   for (i = 0; i < m; i++)
   {
      MPI_Gatherv(a[i], BLOCK_SIZE(id, p, n), dtype, buffer,
                  rec_count, rec_disp, dtype, 0, MPI_COMM_WORLD);
      if (!id)
      {
         print_subvector(buffer, dtype, n);
         putchar('\n');
      }
   }
   free(rec_count);
   free(rec_disp);
   if (!id)
   {
      free(buffer);
      putchar('\n');
   }
}

/*
 *   Print a matrix that is distributed in row-striped
 *   fashion among the processes in a communicator.
 */

void print_row_striped_matrix(
    void **a,           /* IN - 2D array */
    MPI_Datatype dtype, /* IN - Matrix element type */
    int m,              /* IN - Matrix rows */
    int n,              /* IN - Matrix cols */
    MPI_Comm comm)      /* IN - Communicator */
{
   MPI_Status status; /* Result of receive */
   void *bstorage;    /* Elements received from
                         another process */
   void **b;          /* 2D array indexing into
                         'bstorage' */
   int datum_size;    /* Bytes per element */
   int i;
   int id;             /* Process rank */
   int local_rows;     /* This proc's rows */
   int max_block_size; /* Most matrix rows held by
                          any process */
   int prompt;         /* Dummy variable */
   int p;              /* Number of processes */

   MPI_Comm_rank(comm, &id);
   MPI_Comm_size(comm, &p);
   local_rows = BLOCK_SIZE(id, p, m);
   if (!id)
   {
      print_submatrix(a, dtype, local_rows, n);
      if (p > 1)
      {

         datum_size = get_size(dtype);

         max_block_size = BLOCK_SIZE(p - 1, p, m);

         // data payload malloc
         bstorage = my_malloc(id,
                              max_block_size * n * datum_size);

         // data pointers to the payload malloc
         b = (void **)my_malloc(id,
                                max_block_size * PTR_SIZE);

         b[0] = bstorage;

         for (i = 1; i < max_block_size; i++)
         {
            b[i] = b[i - 1] + n * datum_size;
         }

         for (i = 1; i < p; i++)
         {
            MPI_Send(&prompt, 1, MPI_INT, i, PROMPT_MSG,
                     MPI_COMM_WORLD);
            MPI_Recv(bstorage, BLOCK_SIZE(i, p, m) * n, dtype,
                     i, RESPONSE_MSG, MPI_COMM_WORLD, &status);
            print_submatrix(b, dtype, BLOCK_SIZE(i, p, m), n);
         }
         free(b);
         free(bstorage);
      }
      putchar('\n');
   }
   else
   {
      MPI_Recv(&prompt, 1, MPI_INT, 0, PROMPT_MSG,
               MPI_COMM_WORLD, &status);
      MPI_Send(*a, local_rows * n, dtype, 0, RESPONSE_MSG,
               MPI_COMM_WORLD);
   }
}

/*
 *   Writes a matrix that is distributed in row-striped
 *   fashion among the processes in a communicator.
 */
void write_row_striped_matrix(
    char *file_name,
    void **a,           /* IN - 2D array */
    MPI_Datatype dtype, /* IN - Matrix element type */
    int m,              /* IN - Matrix rows */
    int n,              /* IN - Matrix cols */
    MPI_Comm comm)      /* IN - Communicator */
{
   MPI_Status status; /* Result of receive */
   void *bstorage;    /* Elements received from
                         another process */
   void **b;          /* 2D array indexing into
                         'bstorage' */
   int datum_size;    /* Bytes per element */
   int i;
   int id;             /* Process rank */
   int local_rows;     /* This proc's rows */
   int max_block_size; /* Most matrix rows held by
                          any process */
   int prompt;         /* Dummy variable */
   int p;              /* Number of processes */

   MPI_Comm_rank(comm, &id);
   MPI_Comm_size(comm, &p);
   local_rows = BLOCK_SIZE(id, p, m);

   // Write meta data
   if (id == 0)
   {
      FILE *fp;
      fp = fopen(file_name, "wb");
      if (fp == NULL)
      {
         printf("EXIT_FAILURE: Error opening file\n");
         exit(EXIT_FAILURE);
      }
      // for rows & columns
      fwrite(&m, sizeof(int), 1, fp);
      fwrite(&n, sizeof(int), 1, fp);
      fclose(fp);
   }

   if (!id)
   {
      write_submatrix(file_name, a, dtype, local_rows, n);
      if (p > 1)
      {

         datum_size = get_size(dtype);

         max_block_size = BLOCK_SIZE(p - 1, p, m);

         // data payload malloc
         bstorage = my_malloc(id,
                              max_block_size * n * datum_size);

         // data pointers to the payload malloc
         b = (void **)my_malloc(id,
                                max_block_size * PTR_SIZE);

         b[0] = bstorage;

         for (i = 1; i < max_block_size; i++)
         {
            b[i] = b[i - 1] + n * datum_size;
         }

         for (i = 1; i < p; i++)
         {
            MPI_Send(&prompt, 1, MPI_INT, i, PROMPT_MSG,
                     MPI_COMM_WORLD);
            MPI_Recv(bstorage, BLOCK_SIZE(i, p, m) * n, dtype,
                     i, RESPONSE_MSG, MPI_COMM_WORLD, &status);
            write_submatrix(file_name, b, dtype, BLOCK_SIZE(i, p, m), n);
         }
         free(b);
         free(bstorage);
      }
   }
   else
   {
      MPI_Recv(&prompt, 1, MPI_INT, 0, PROMPT_MSG,
               MPI_COMM_WORLD, &status);
      MPI_Send(*a, local_rows * n, dtype, 0, RESPONSE_MSG,
               MPI_COMM_WORLD);
   }
}

/*
 *   Writes a matrix that is distributed in row-striped
 *   fashion among the processes in a communicator, taking
 *   into account stencil data.
 */
void write_row_striped_matrix_stencil(
    char *out_file,      /* IN - output file name */
    void **a,            /* IN - 2D array */
    MPI_Datatype dtype,  /* IN - Matrix element type */
    int m,               /* IN - Matrix rows */
    int n,               /* IN - Matrix cols */
    MPI_Comm comm)       /* IN - Communicator */
{
   MPI_Status status;    /* Result of receive */
   void *bstorage;       /* Elements received from another process */
   void **b;             /* 2D array indexing into 'bstorage' */
   int datum_size;       /* Bytes per element */
   int i;                /* Loop variable */
   int id;               /* Process rank */
   int local_rows;       /* This proc's rows */
   int max_block_size;   /* Most matrix rows held by any process */
   int prompt;           /* Dummy variable */
   int p;                /* Number of processes */
   int total_halo_rows = 0;
   int additional_stencil_rows = 0;
   MPI_Comm_rank(comm, &id);
   MPI_Comm_size(comm, &p);
   local_rows = BLOCK_SIZE(id, p, m);

   // Write rows & columns
   FILE *fp;
   if (id == 0)
   {
      fp = fopen(out_file, "wb");
      fwrite(&m, sizeof(int), 1, fp);
      fwrite(&n, sizeof(int), 1, fp);
   }

   // Determine # of additional rows for stencil
   if (id == 0 || id == p - 1) {
      additional_stencil_rows = 1;
   }
   else {
      additional_stencil_rows = 2;
   }

   if (!id)
   {
      fwrite(a[0], (local_rows) * n, sizeof(dtype), fp);
      // If # of processes is greater than zero
      if (p > 1)
      {
         for (int i = 0; i < p; i++)
         {
            if (i == 0 || i == p - 1)
            {
               total_halo_rows += 1;
            }
            else
            {
               total_halo_rows += 2;
            }
         }
         datum_size = get_size(dtype);
         max_block_size = BLOCK_SIZE(id, p, m) + total_halo_rows;
         // Data payload malloc
         bstorage = my_malloc(id, max_block_size * n * datum_size);
         // Data pointers to the payload malloc
         b = (void **)my_malloc(id, max_block_size * PTR_SIZE);
         b[0] = bstorage;
         for (i = 1; i < max_block_size; i++)
         {
            b[i] = b[i - 1] + n * datum_size;
         }
         for (i = 1; i < p; i++)
         {
            if (i == p - 1 || i == 0) {
               additional_stencil_rows = 1;
            } else {
               additional_stencil_rows = 2;
            }
            MPI_Send(&prompt, 1, MPI_INT, i, PROMPT_MSG, MPI_COMM_WORLD);
            MPI_Recv(bstorage, (BLOCK_SIZE(i, p, m) + additional_stencil_rows) * n, dtype, i, RESPONSE_MSG, MPI_COMM_WORLD, &status);
            fwrite(b[1], (BLOCK_SIZE(i, p, m)) * n, sizeof(dtype), fp);
         }
         free(b);
         free(bstorage);
         fclose(fp);
      }
   }
   else
   {
      MPI_Recv(&prompt, 1, MPI_INT, 0, PROMPT_MSG, MPI_COMM_WORLD, &status);
      MPI_Send(*a, (local_rows + additional_stencil_rows) * n, dtype, 0, RESPONSE_MSG, MPI_COMM_WORLD);
   }
}

/*
 *   Writes a matrix that is distributed in row-striped
 *   fashion among the processes in a communicator, taking
 *   into account stencil data with no meta data
 */
void write_row_striped_matrix_stencil_noMeta(
    char *out_file,      /* IN - output file name */
    void **a,            /* IN - 2D array */
    MPI_Datatype dtype,  /* IN - Matrix element type */
    int m,               /* IN - Matrix rows */
    int n,               /* IN - Matrix cols */
    MPI_Comm comm,       /* IN - Communicator */
    int type_of_write    /* IN - 1 = 'wb', 2 = 'ab'*/ 
    )  
{
   MPI_Status status;    /* Result of receive */
   void *bstorage;       /* Elements received from another process */
   void **b;             /* 2D array indexing into 'bstorage' */
   int datum_size;       /* Bytes per element */
   int i;                /* Loop variable */
   int id;               /* Process rank */
   int local_rows;       /* This proc's rows */
   int max_block_size;   /* Most matrix rows held by any process */
   int prompt;           /* Dummy variable */
   int p;                /* Number of processes */
   int total_halo_rows = 0;
   int additional_stencil_rows = 0;
   MPI_Comm_rank(comm, &id);
   MPI_Comm_size(comm, &p);
   local_rows = BLOCK_SIZE(id, p, m);

   FILE *fp;
   if (id == 0 && type_of_write == 1) { fp = fopen(out_file, "wb"); }
   else if (id == 0 && type_of_write == 2) { fp = fopen(out_file, "ab"); }

   // Determine # of additional rows for stencil
   if (id == 0 || id == p - 1) {
      additional_stencil_rows = 1;
   }
   else {
      additional_stencil_rows = 2;
   }

   if (!id)
   {
      fwrite(a[0], (local_rows) * n, sizeof(dtype), fp);
      // If # of processes is greater than zero
      if (p > 1)
      {
         for (int i = 0; i < p; i++)
         {
            if (i == 0 || i == p - 1)
            {
               total_halo_rows += 1;
            }
            else
            {
               total_halo_rows += 2;
            }
         }
         datum_size = get_size(dtype);
         max_block_size = BLOCK_SIZE(id, p, m) + total_halo_rows;
         // Data payload malloc
         bstorage = my_malloc(id, max_block_size * n * datum_size);
         // Data pointers to the payload malloc
         b = (void **)my_malloc(id, max_block_size * PTR_SIZE);
         b[0] = bstorage;
         for (i = 1; i < max_block_size; i++)
         {
            b[i] = b[i - 1] + n * datum_size;
         }
         for (i = 1; i < p; i++)
         {
            if (i == p - 1 || i == 0) {
               additional_stencil_rows = 1;
            } else {
               additional_stencil_rows = 2;
            }
            MPI_Send(&prompt, 1, MPI_INT, i, PROMPT_MSG, MPI_COMM_WORLD);
            MPI_Recv(bstorage, (BLOCK_SIZE(i, p, m) + additional_stencil_rows) * n, dtype, i, RESPONSE_MSG, MPI_COMM_WORLD, &status);
            fwrite(b[1], (BLOCK_SIZE(i, p, m)) * n, sizeof(dtype), fp);
         }
         free(b);
         free(bstorage);
         fclose(fp);
      }
   }
   else
   {
      MPI_Recv(&prompt, 1, MPI_INT, 0, PROMPT_MSG, MPI_COMM_WORLD, &status);
      MPI_Send(*a, (local_rows + additional_stencil_rows) * n, dtype, 0, RESPONSE_MSG, MPI_COMM_WORLD);
   }
}


/*
 *   Print a vector that is block distributed among the
 *   processes in a communicator.
 */

void print_block_vector(
    void *v,            /* IN - Address of vector */
    MPI_Datatype dtype, /* IN - Vector element type */
    int n,              /* IN - Elements in vector */
    MPI_Comm comm)      /* IN - Communicator */
{
   int datum_size; /* Bytes per vector element */
   int i;
   int prompt;        /* Dummy variable */
   MPI_Status status; /* Result of receive */
   void *tmp;         /* Other process's subvector */
   int id;            /* Process rank */
   int p;             /* Number of processes */

   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &id);
   datum_size = get_size(dtype);

   if (!id)
   {
      print_subvector(v, dtype, BLOCK_SIZE(id, p, n));
      if (p > 1)
      {
         tmp = my_malloc(id, BLOCK_SIZE(p - 1, p, n) * datum_size);
         for (i = 1; i < p; i++)
         {
            MPI_Send(&prompt, 1, MPI_INT, i, PROMPT_MSG,
                     comm);
            MPI_Recv(tmp, BLOCK_SIZE(i, p, n), dtype, i,
                     RESPONSE_MSG, comm, &status);
            print_subvector(tmp, dtype, BLOCK_SIZE(i, p, n));
         }
         free(tmp);
      }
      printf("\n\n");
   }
   else
   {
      MPI_Recv(&prompt, 1, MPI_INT, 0, PROMPT_MSG, comm,
               &status);
      MPI_Send(v, BLOCK_SIZE(id, p, n), dtype, 0,
               RESPONSE_MSG, comm);
   }
}

/*
 *   Print a vector that is replicated among the processes
 *   in a communicator.
 */

void print_replicated_vector(
    void *v,            /* IN - Address of vector */
    MPI_Datatype dtype, /* IN - Vector element type */
    int n,              /* IN - Elements in vector */
    MPI_Comm comm)      /* IN - Communicator */
{
   int id; /* Process rank */

   MPI_Comm_rank(comm, &id);

   if (!id)
   {
      print_subvector(v, dtype, n);
      printf("\n\n");
   }
}