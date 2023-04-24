/*
team: Christopher Grigsby & Jordan Drakos
author: c-grigsby
make-2d.c: creates an initial matrix for computation
*/

// @packages
#include <stdlib.h>
#include <stdio.h>
// @scripts
#include "utilities.h"

int main(int argc, char *argv[]) {
  char *file_name;
  int rows, columns;

  if (argc != 4) {
    printf("usage: ./make-2d <rows> <cols> <output_file>\n");
    exit(EXIT_FAILURE);
  }
  // Store usage data
  rows = atoi(argv[1]);
  columns = atoi(argv[2]);
  file_name = argv[3];

  // Make 2DArray
  double **arr2D;
  malloc2D(&arr2D, rows, columns);
  int i, j;
  for (i = 0; i < rows; i++) {
    for (j = 0; j < columns; j++) {

      if (j == 0 || j == columns - 1) { arr2D[i][j] = 1.00; }
      else { arr2D[i][j] = 0.00; }

    }
  }
  // Write 2D Array to .dat file
  write2DArray(file_name, rows, columns, (double **)arr2D);

  free(arr2D);
  exit(EXIT_SUCCESS);
}
