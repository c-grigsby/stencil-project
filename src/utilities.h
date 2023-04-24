/*
author: c-grigsby
utilities.h - header for utility functions
*/

#ifndef UTILITIES_H_
#define UTILITIES_H_

// print2DArray: prints a 2D array
void print2DArray(int rows, int cols, double *arr2D[]);

// malloc2D: 2D array allocation
void malloc2D(double ***a, int jmax, int imax);

// read2DArray: creates a 2DArray from a .dat file and returns the addy
double **read2DArray(char *file_name);

// getRowsAndColumns: returns the meta data for rows & columns from a .dat file
int *readRowsAndColumns(char *file_name);

// write2DArray: writes a 2D Array to a .dat file w/ meta data for rows & columns
void write2DArray(char *file_name, int rows, int columns, double *arr2D[]);

// write2DArray_NoMeta: writes a 2D Array to a file w/o meta data
void write2DArray_NoMeta(char *file_name, int rows, int columns, double *arr2D[]);

// appendWrite2DArray: appends data of a 2D Array to a file 
void appendWrite2DArray(char *file_name, int rows, int columns, double *arr2D[]);

// getFileSize: returns the size of a file in bytes
void getFileSize(char *file_name);

// write_summary_data: writes a summary of stencil calculation data to a file
void write_stencil_summary_data(char *file_name, double time_overall, double cpu_time, int num_processes, int rows, int columns, int num_iterations);

#endif