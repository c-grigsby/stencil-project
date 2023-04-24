/*
team: Christopher Grigsby & Jordan Drakos
author: c-grigsby
print-2d: displays a 2D array from a file to the console
*/

// @packages
#include <stdlib.h>
#include <stdio.h>
// @scripts
#include "utilities.h"

int main(int argc, char *argv[]) {
  char *file_name;
  int rows, columns;

  if (argc != 2) {
    printf("usage: ./print-2d <input data file>\n");
    exit(EXIT_FAILURE);
  }
  // Store usage data
  file_name = argv[1];

  // Get rows & columns of 2DArray from .dat file
  int *arr = readRowsAndColumns(file_name);
  rows = arr[0];
  columns = arr[1];

  // Get 2DArray from .dat file
  double **arr2D;
  arr2D = read2DArray(file_name);

  // Print 2DArray
  print2DArray(rows, columns, (double **)arr2D);

  free(arr2D);
  exit(EXIT_SUCCESS);
}
