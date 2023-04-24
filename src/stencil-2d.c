/*
team: Christopher Grigsby & Jordan Drakos
author: c-grigsby
stencil-2d.c: performs stencil computations on a 2D matrix from a file
*/

// @packages
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
// @scripts
#include "utilities.h"

int main(int argc, char *argv[]) {
  char *input_file_name, *output_file_name, *all_iterations, *summary_file_name;
  clock_t total_time_start, total_time_end;
  clock_t io_time_start, io_time_end;
  double elapsed_total_time, elapsed_compute_time, elapsed_io_time;
  int num_iterations, rows, columns;
  int loop_count = 0;
  all_iterations = NULL;
  summary_file_name = NULL;

  if (argc < 4) {
    printf("usage: %s <num iterations> <input file> <output file> <summary.txt (optional)> <all-iterations.raw (optional)>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // Start total clock time
  total_time_start = clock();

  // Store usage vars
  num_iterations = atoi(argv[1]);
  input_file_name = argv[2];
  output_file_name = argv[3];
  if (argc == 5) { summary_file_name = argv[4]; }
  if (argc == 6) { 
    summary_file_name = argv[4];
    all_iterations = argv[5]; 
  }

  // Get initial 2DArray rows & columns from .dat file
  io_time_start = clock();
  int *arr = readRowsAndColumns(input_file_name);
  rows = arr[0];
  columns = arr[1];

  // Get initial 2DArray from .dat file
  double **arr2D;
  arr2D = read2DArray(input_file_name);

  // Create a second 2DArray for computations
  double **new_arr2D;
  new_arr2D = read2DArray(input_file_name);

  if (all_iterations != NULL) {
    // Write initial data to raw file
    write2DArray_NoMeta(all_iterations, rows, columns, (double **)arr2D);
  }
  io_time_end = clock();
  elapsed_io_time = ((double)(io_time_end - io_time_start)) / CLOCKS_PER_SEC;

  // @Compute 9-Point Stencil
  while (loop_count < num_iterations) {
    // Perform stencil calculations
    int i, j;
    for (i = 1; i < rows - 1; i++) {
      for (j = 1; j < columns - 1; j++) {
        new_arr2D[i][j] = (arr2D[i - 1][j - 1] + arr2D[i - 1][j] + arr2D[i - 1][j + 1] +
                           arr2D[i][j + 1] + arr2D[i + 1][j + 1] + arr2D[i + 1][j] +
                           arr2D[i + 1][j - 1] + arr2D[i][j - 1] + arr2D[i][j]) /
                          9.0;
      }
    }
    // Swap pointers to the arr's
    double **temp = new_arr2D;
    new_arr2D = arr2D;
    arr2D = temp;
    // Append data to raw file (if opted)
    if (all_iterations != NULL) {
      io_time_start = clock();
      appendWrite2DArray(all_iterations, rows, columns, (double **)arr2D);
      io_time_end = clock();
      elapsed_io_time = elapsed_io_time + ((double)(io_time_end - io_time_start)) / CLOCKS_PER_SEC;
    }
    // Increment count
    loop_count++;
  }

  // Write final 2DArray
  io_time_start = clock();
  write2DArray(output_file_name, rows, columns, (double **)arr2D);
  io_time_end = clock();
  elapsed_io_time = elapsed_io_time + ((double)(io_time_end - io_time_start)) / CLOCKS_PER_SEC;

  // Performace Calculations
  total_time_end = clock();
  elapsed_total_time = ((double)(total_time_end - total_time_start)) / CLOCKS_PER_SEC;
  elapsed_compute_time = elapsed_total_time - elapsed_io_time;

  getFileSize(input_file_name);
  getFileSize(output_file_name);
  if (all_iterations != NULL) { getFileSize(all_iterations); }

  // Display Time
  printf("\nTotal Elapsed Time: %f seconds\n", elapsed_total_time);
  printf("Total Compute Time: %f seconds\n", elapsed_compute_time);
  printf("Total IO Time:      %f seconds\n\n", elapsed_io_time);

  if (summary_file_name != NULL) {
    write_stencil_summary_data(summary_file_name, elapsed_total_time, elapsed_compute_time, 1, rows, columns, num_iterations);
  }

  free(arr2D);
  free(new_arr2D);
  exit(EXIT_SUCCESS);
}