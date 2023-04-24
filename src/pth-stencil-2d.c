/* 
team: Christopher Grigsby & Jordan Drakos
author: c-grigsby
pth-stencil-2d.c: utilizes pthreads for stencil computations on a 2D matrix from a file
notes: #define info => id = thread rank from 0 -> p-1, p = total # of threads, n = # of elements to divide
*/

// @packages
#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
// @scripts
#include "utilities.h"
#include "timer.h"
// @define
#define MIN(a,b)           ((a)<(b)?(a):(b))
#define BLOCK_LOW(id,p,n)  ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)
#define BLOCK_OWNER(j,p,n) (((p)*((j)+1)-1)/(n))
#define PTR_SIZE           (sizeof(void*))
#define CEILING(i,j)       (((i)+(j)-1)/(j)) 
// @global
char *raw_file_name;
double **arr2D, **new_arr2D, elapsed_io_time, io_time_start, io_time_end;
int num_iterations, rows, columns, debug_level, num_threads;
pthread_barrier_t barrier;
// @parallel functions 
void *Pth_stencil(void* rank);
// @serial functions 
void Usage(char *prog_name);


int main(int argc, char *argv[]) {
  char *input_file_name, *output_file_name, *summary_file_name;
  raw_file_name = NULL;
  summary_file_name = NULL;
  elapsed_io_time = 0.0;
  double total_time_start, total_time_end, compute_time_start, compute_time_end;
  double elapsed_total_time, elapsed_compute_time, elapsed_other_time;
  long thread;
  pthread_t* thread_handles;
  
  GET_TIME(total_time_start);

  if (argc < 6) { Usage(argv[0]); }
  if (argc == 7) { summary_file_name = argv[6]; }
  if (argc == 8) { raw_file_name = argv[7]; }

  // Store usage vars
  num_iterations = atoi(argv[1]);
  input_file_name = argv[2];
  output_file_name = argv[3];
  debug_level = atoi(argv[4]);
  num_threads = atoi(argv[5]);

  if (debug_level == 1 || debug_level == 2) {
    printf("iterations: %d\ninput_file: %s\noutput_file: %s\ndebug_level: %d\nnum_threads: %d\nsummary_file_name: %s\nraw_file_name: %s\n\n", num_iterations, input_file_name, output_file_name, debug_level, num_threads, summary_file_name, raw_file_name);
  }

  // Get rows & columns from matrix file meta data
  int *arr = readRowsAndColumns(input_file_name);
  rows = arr[0];
  columns = arr[1];
  if (debug_level == 1 || debug_level == 2) { 
    printf("matrix rows: %d\nmatrix columns: %d\n\n", rows, columns); 
  }

  // Get initial matrix data 
  arr2D = read2DArray(input_file_name);

  // Create 2nd matrix for computations
  malloc2D(&new_arr2D, rows, columns);
  int i, j;
  for (i = 0; i < rows; i++) {
    for (j = 0; j < columns; j++) {
      new_arr2D[i][j] = arr2D[i][j];
    }
  }

  // Persist matrix data to raw file (if opted)
  if (raw_file_name != NULL) {
    write2DArray_NoMeta(raw_file_name, rows, columns, (double **)arr2D);
  }

  // @init pthread data
  pthread_barrier_init(&barrier, NULL, num_threads);
  thread_handles = malloc(num_threads*sizeof(pthread_t));

  GET_TIME(compute_time_start); 
  // @init pthread work
  for (thread = 0; thread < num_threads; thread++) {
    pthread_create(&thread_handles[thread], NULL, Pth_stencil, (void *)thread);
  }

  // @end pthreads
  for (thread = 0; thread < num_threads; thread++) {
    pthread_join(thread_handles[thread], NULL);
  }
  pthread_barrier_destroy(&barrier);
  GET_TIME(compute_time_end); 

  // Write final 2DArray data
  write2DArray(output_file_name, rows, columns, (double **)arr2D);

  // Time Calculations
  GET_TIME(total_time_end);
  elapsed_total_time = total_time_end - total_time_start;
  elapsed_compute_time = (compute_time_end - compute_time_start) - elapsed_io_time;
  elapsed_other_time = elapsed_total_time - elapsed_compute_time;

  if (debug_level == 1 || debug_level == 2) {
    getFileSize(input_file_name);
    getFileSize(output_file_name);
    if (raw_file_name != NULL) { getFileSize(raw_file_name); }
  }

  // Display
  printf("\nTotal Elapsed Time: %f seconds\n", elapsed_total_time);
  printf("Computation Time:   %f seconds\n", elapsed_compute_time);
  printf("Other Time:         %f seconds\n\n", elapsed_other_time);

  if (summary_file_name != NULL) {
    write_stencil_summary_data(summary_file_name, elapsed_total_time, elapsed_compute_time, num_threads, rows, columns, num_iterations);
  }

  free(arr2D);
  free(new_arr2D);
  exit(EXIT_SUCCESS);
}


// @helper *Pth_stencil: performs a 9-point stencil calculation on a 2D matrix w/ pthreads
void *Pth_stencil(void *rank) {
  long my_rank = (long) rank;
  long my_first_row, my_last_row;
  int loop_count = 0, i, j;
  double my_start_time, my_end_time, io_time_start, io_time_end;

  my_first_row = BLOCK_LOW(my_rank, num_threads, rows);
  my_last_row = BLOCK_HIGH(my_rank, num_threads, rows);

  if (debug_level == 2) { 
    printf("my_rank: %ld my_first_row: %ld my_last_row: %ld\n\n", my_rank, my_first_row, my_last_row);
    GET_TIME(my_start_time);
  }

  while (loop_count < num_iterations) {
    // Traverse matrix
    for (i = my_first_row; i <= my_last_row; i++) {
      for (j = 1; j < columns -1; j++) {
        if(i != 0 && i != rows -1) {
          // Stencil calculations
          new_arr2D[i][j] = (arr2D[i - 1][j - 1] + arr2D[i - 1][j] + arr2D[i - 1][j + 1] +
                           arr2D[i][j + 1] + arr2D[i + 1][j + 1] + arr2D[i + 1][j] +
                           arr2D[i + 1][j - 1] + arr2D[i][j - 1] + arr2D[i][j]) / 9.0;
        }
      }
    }
    // Wait for all thread calculations
    pthread_barrier_wait(&barrier);
    // Swap matrix pointers
    if (my_rank == 0) {
      double **temp = new_arr2D;
      new_arr2D = arr2D;
      arr2D = temp;
      if (debug_level == 2) { print2DArray(rows, columns, (double **)arr2D); }
      // Persist matrix data to raw file (if opted)
      if (raw_file_name != NULL) {
        GET_TIME(io_time_start); 
        appendWrite2DArray(raw_file_name, rows, columns, (double **)arr2D);
        GET_TIME(io_time_end);
        elapsed_io_time += io_time_end - io_time_start;
      }
    }
    // Wait for thread 0 to swap pointers
    pthread_barrier_wait(&barrier);
    loop_count++;
	}

  if (debug_level == 2) {
    GET_TIME(my_end_time);
    printf("thread_number: %ld thread_calculation_time: %f seconds\n",my_rank, my_end_time - my_start_time);
  }
  return NULL; 
} 


// @helper Usage: displays usage data to the console
void Usage(char *prog_name) {
   printf("usage: %s <num iterations> <input file> <output file> <debug level> <num threads> <summary-file-name (optional)> <all-stacked-file-name.raw (optional)>\n", prog_name);
   exit(EXIT_FAILURE);
} 
