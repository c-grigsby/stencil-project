/*
team: Christopher Grigsby & Jordan Drakos 
author: c-grigsby
mpi-stencil-2d.c: performs a stencil calculation on a matrix using MPI for parallelization
*/

// @packages
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
// @scripts
#include "MyMPI.h"
#include "timer.h"
#include "utilities.h"
// @define
#define dtype double
#define SWAP_PTR(xnew,xold,xtmp) (xtmp=xnew, xnew=xold, xold=xtmp)
// @helpers
double **stencil_2D_MPI(double **arr2D, double **new_arr2D, int row, int column);
void Usage(char *prog_name);

// main program
int main(int argc, char *argv[]) {
  char *input_file_name, *output_file_name, *raw_file_name, *summary_file_name;
  double elapsed_total_time, elapsed_compute_time, elapsed_other_time;
  double total_time_start, total_time_end, compute_time_start, compute_time_end;
  int num_iterations, rows, columns, debug_level;
  int process_id, num_processes, type_of_write;
  summary_file_name = NULL;
  raw_file_name = NULL;
  dtype **arr2D, *arr2D_data;
  dtype **new_arr2D, *new_arr2D_data;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
  MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

  if (process_id == 0) { GET_TIME(total_time_start); }

  // Handle usage data
  if (argc < 5) { Usage(argv[0]); }
  if (argc == 6) { summary_file_name = argv[5]; }
  if (argc == 7) {
    summary_file_name = argv[5];
    raw_file_name = argv[6];
  }

  num_iterations = atoi(argv[1]);
  input_file_name = argv[2];
  output_file_name = argv[3];
  debug_level = atoi(argv[4]);

  if (debug_level == 1 && process_id == 0) {
    printf("input_file: %s, output_file: %s, debug_level: %d\n\n", input_file_name, output_file_name, debug_level);
  }
    
  // Get 2D matrix data from file for the matrices
  read_row_striped_matrix_stencil(input_file_name, (void ***)&arr2D, (void **)&arr2D_data, MPI_DOUBLE, &rows, &columns, MPI_COMM_WORLD);
  read_row_striped_matrix_stencil(input_file_name, (void ***)&new_arr2D, (void **)&new_arr2D_data, MPI_DOUBLE, &rows, &columns, MPI_COMM_WORLD);

  if ((debug_level == 1 || debug_level == 2) && process_id == 0) {
    printf("rows: %d, columns: %d, iterations: %d, num_processes: %d\n\n", rows, columns, num_iterations, num_processes);
  }

  // Exchange row data for the stencil
  exchange_row_striped_matix_stencil_data((void **)arr2D, MPI_DOUBLE, rows, columns, MPI_COMM_WORLD);
  exchange_row_striped_matix_stencil_data((void **)new_arr2D, MPI_DOUBLE, rows, columns, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);

  // Write matrix data to raw file (if opted)
  if (raw_file_name != NULL) {
    type_of_write = 1;
    write_row_striped_matrix_stencil_noMeta(raw_file_name, (void**)arr2D, MPI_DOUBLE, rows, columns, MPI_COMM_WORLD, type_of_write);
  }

  // Execute 9-point stencil calculations 
  elapsed_compute_time = 0.00;
  for (int i = 0; i < num_iterations; i++) {
    // Process zero executes performance calculations
    if (process_id == 0) { GET_TIME(compute_time_start); }
    stencil_2D_MPI(arr2D, new_arr2D, rows, columns);
    if (process_id == 0) { 
      GET_TIME(compute_time_end); 
      elapsed_compute_time += compute_time_end - compute_time_start;
    }
    exchange_row_striped_matix_stencil_data((void **)new_arr2D, MPI_DOUBLE, rows, columns, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    // Swap matrix pointers
    double **temp = new_arr2D;
    new_arr2D = arr2D;
    arr2D = temp;
    // Append data to stacked raw file (if opted)
    if (raw_file_name != NULL) {
      type_of_write = 2;
      write_row_striped_matrix_stencil_noMeta(raw_file_name, (void**)arr2D, MPI_DOUBLE, rows,   columns, MPI_COMM_WORLD, type_of_write);
    }
  }

  // Persist final results to a file
  write_row_striped_matrix_stencil(output_file_name, (void**)arr2D, MPI_DOUBLE, rows, columns, MPI_COMM_WORLD);

  if ((debug_level == 1 || debug_level == 2) && process_id == 0) {
    getFileSize(input_file_name);
    getFileSize(output_file_name);
    if (raw_file_name != NULL) { getFileSize(raw_file_name); }
  }
  
  // Time/Performace Calculations
  if (process_id == 0) {
    GET_TIME(total_time_end);
    elapsed_total_time = total_time_end - total_time_start;
    elapsed_other_time = elapsed_total_time - elapsed_compute_time;

    printf("\nTotal Elapsed Time: %f seconds\n", elapsed_total_time);
    printf("Computation Time:   %f seconds\n", elapsed_compute_time);
    printf("Other Time:         %f seconds\n\n", elapsed_other_time);

    if (summary_file_name != NULL) {
      write_stencil_summary_data(summary_file_name, elapsed_total_time, elapsed_compute_time, num_processes, rows, columns, num_iterations);
    }
  }

  free(arr2D);
  free(arr2D_data);
  free(new_arr2D);
  free(new_arr2D_data);
  MPI_Finalize();
  return 0;
}

// @helper stencil_2D_MPI: executes a 9-point stencil on a matrix with MPI
double **stencil_2D_MPI(double **arr2D, double **new_arr2D, int rows, int columns) {
  int process_id;             
  int num_processes;              
  int local_rows;     
  int additional_halo_rows = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
  MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
  local_rows = BLOCK_SIZE(process_id, num_processes, rows);

  // Determine additional # of rows for stencil
  if (process_id == 0 || process_id == num_processes - 1) { 
    additional_halo_rows = 1;
  } else { 
    additional_halo_rows = 2;
  }

  int i, j;
  for (i = 1; i < (local_rows + additional_halo_rows) - 1; i++) {
    for (j = 1; j < columns -1; j++) {
      new_arr2D[i][j] = (arr2D[i - 1][j - 1] + arr2D[i - 1][j] + arr2D[i - 1][j + 1] + 
      arr2D[i][j + 1] + arr2D[i + 1][j + 1] + arr2D[i + 1][j] + arr2D[i + 1][j - 1] + 
      arr2D[i][j - 1] + arr2D[i][j]) / 9.0;
    }
  }
  return (new_arr2D);
}

// @helper Usage: displays usage data to the console
void Usage(char *prog_name) {
  printf("usage: %s <num iterations> <input file> <output file> <debug level> <summary-file-name (optional)> <stacked-file-name.raw (optional)>\n", prog_name);
  MPI_Finalize();
  exit(EXIT_FAILURE);
} 
