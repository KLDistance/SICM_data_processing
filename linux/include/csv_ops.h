#ifndef __CSV_OPS_H__
#define __CSV_OPS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// .csv file path length
#define CSV_PATH_LEN 256

// buffer length
#define BUF_LEN 32

// csv data type
#define CSV_TYPE_FLOAT 0
#define CSV_TYPE_INT 1
#define CSV_TYPE_SHORT 2

// Thread identifier
typedef struct
{
    unsigned int *common_arr;
    unsigned int def_thread_id;
    char *txt_area;
} CSV_OPS_MULTITHREAD_PARAMETER;

// Define .csv file struct
typedef struct
{
    char csv_path[CSV_PATH_LEN];
    unsigned int row_num;
    unsigned int col_num;
    void *data_arr;
    float *result_data_arr;
    char csv_data_type;
} CSV_STRUCT;

int get_csv_arr_size(char *data_str, CSV_STRUCT *csv_struct_ptr);

int from_file_float(char *file_str, CSV_STRUCT *csv_struct_ptr);
int from_file_int(char *file_str, CSV_STRUCT *csv_struct_ptr);
int from_file_short(char *file_str, CSV_STRUCT *csv_struct_ptr);

extern int csv_reader(CSV_STRUCT *csv_struct_ptr);
extern int csv_writer(CSV_STRUCT *csv_struct_ptr);

void* single_file_slice_proc(void *arg);

#endif