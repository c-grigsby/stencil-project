/*
author: c-grigsby
utilities.c: implementation for utility functions 
*/

// @packages
#include <stdlib.h>
#include <stdio.h>
// @scripts
#include "utilities.h"

// print2DArray: prints a 2D array
void print2DArray(int rows, int cols, double *arr2D[]) {
  int i, j;
  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++) {
      printf("%.2f    ", arr2D[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

// malloc2D: 2D array allocation
void malloc2D(double ***a, int jmax, int imax) {
  // first allocate a block of memory for the row pointers and 2D array
  double **x = (double **)malloc(jmax * sizeof(double *) + jmax * imax * sizeof(double));
  // Now assign the start of the block of memory for the 2D array after the row pointers
  x[0] = (double *)x + jmax;
  // Last, assign the memory location to point to the for each row pointer
  for (int j = 1; j < jmax; j++) {
    x[j] = x[j - 1] + imax;
  }
  *a = x;
}

// read2DArray: returns a 2DArray from a file
double **read2DArray(char *file_name) {
  int rows, columns;
  FILE *fp;
  fp = fopen(file_name, "rb");
  if (fp == NULL) {
    printf("Error opening file\n");
    exit(EXIT_FAILURE);
  }
  // ...rows & columns of 2DArray from .dat file
  fread(&rows, sizeof(int), 1, fp);
  fread(&columns, sizeof(int), 1, fp);
  // ...2DArray
  double **arr2D;
  malloc2D(&arr2D, rows, columns);
  int i, j;
  for (i = 0; i < rows; i++) {
    for (j = 0; j < columns; j++) {
      fread(&arr2D[i][j], sizeof(double), 1, fp);
    }
  }
  fclose(fp);
  return arr2D;
}

// getRowsAndColumns: returns the meta data for rows & columns from .dat file
int *readRowsAndColumns(char *file_name) {
  static int arr[2];
  FILE *fp;
  fp = fopen(file_name, "rb");
  if (fp == NULL) {
    printf("Error opening file\n");
    exit(EXIT_FAILURE);
  }
  // rows
  fread(&arr[0], sizeof(int), 1, fp);
  // columns
  fread(&arr[1], sizeof(int), 1, fp);
  fclose(fp);
  return arr;
}

// write2DArray: writes a 2D Array to a .dat file w/ meta data for rows & columns
void write2DArray(char *file_name, int rows, int columns, double *arr2D[]) {
  FILE *fp;
  fp = fopen(file_name, "wb");
  if (fp == NULL) {
    printf("Error opening file\n");
    exit(EXIT_FAILURE);
  }
  // ...metadata for rows, columns
  fwrite(&rows, sizeof(int), 1, fp);
  fwrite(&columns, sizeof(int), 1, fp);
  // ...2DArray
  int i, j;
  for (i = 0; i < rows; i++) {
    for (j = 0; j < columns; j++) {
      fwrite(&arr2D[i][j], sizeof(double), 1, fp);
    }
  }
  fclose(fp);
}

// write2DArray_NoMeta: writes a 2D Array to a file w/o meta data
void write2DArray_NoMeta(char *file_name, int rows, int columns, double *arr2D[]) {
  FILE *fp;
  fp = fopen(file_name, "wb");
  if (fp == NULL) {
    printf("Error opening file\n");
    exit(EXIT_FAILURE);
  }
  // Write the 2DArray
  int i, j;
  for (i = 0; i < rows; i++) {
    for (j = 0; j < columns; j++) {
      fwrite(&arr2D[i][j], sizeof(double), 1, fp);
    }
  }
  fclose(fp);
}

// appendWrite2DArray: appends data of a 2D Array to a file
void appendWrite2DArray(char *file_name, int rows, int columns, double *arr2D[]) {
  FILE *fp;
  fp = fopen(file_name, "ab");
  if (fp == NULL) {
    printf("Error opening file\n");
    exit(EXIT_FAILURE);
  }
  // ...2DArray
  int i, j;
  for (i = 0; i < rows; i++) {
    for (j = 0; j < columns; j++) {
      fwrite(&arr2D[i][j], sizeof(double), 1, fp);
    }
  }
  fclose(fp);
}

// write_summary_data: writes a summary of stencil calculation data to a file
void write_stencil_summary_data(char *file_name, double time_overall, double cpu_time, int num_processes, int rows, int columns, int num_iterations) {
  FILE *fp;
  fp = fopen(file_name, "a");
  if (fp == NULL) {
    printf("Error: Failed to open file\n");
    exit(EXIT_FAILURE);
  }
  fprintf(fp, "%f %f %d %d %d %d\n", time_overall, cpu_time, num_processes, rows, columns, num_iterations);
  fclose(fp);
}

// getFileSize: returns the size of a file in bytes
void getFileSize(char *file_name) {
  long size;
  FILE *fp;
  fp = fopen(file_name, "r");

  if (fp == NULL) {
    printf("Error opening file\n");
    exit(EXIT_FAILURE);
  }
  // Seek to end of the file
  fseek(fp, 0, SEEK_END);
  // Get position of end of the file
  size = ftell(fp);
  // Display
  printf("The file size of %s is: %ld bytes\n", file_name, size);
  fclose(fp);
}
