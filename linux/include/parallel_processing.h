#ifndef __PARALLEL_PROCESSING_H__
#define __PARALLEL_PROCESSING_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <CL/cl.h>
#include <time.h>

// Define the property array entry pass into the GPU
#define L_PROPERTY_NUM 7

// Array row number
#define PL_ARR_ROW_NUM 0
// Array column number
#define PL_ARR_COL_NUM 1
// Relative number of rows related in terms of the central point
#define PL_RELATIVE_ROW 2
// Relative number of columns related in terms of the central point
#define PL_RELATIVE_COL 3
// Reciprocal of the drag coefficient
#define PL_CENTRAL_DRAG 4
// Distance coefficient for weight laplacian distance
#define WL_DISTANCE_COEFFICIENT 5
// Threshold
#define SL_THRESHOLD 6


typedef struct
{
    char *kernel_source_str,
    size_t kernel_source_size,
    cl_int cl_ret,
    cl_context context,
    cl_command_queue command_queue
} GPU_COM_STRUCT;

typedef struct
{
    unsigned int row_num,
    unsigned int col_num,
    unsigned int data_num,
    unsigned int data_size,
    float *input_data_arr,
    float *output_data_arr
} GPU_DATA_2D_ARR;

extern int init_gpu_state(GPU_COM_STRUCT *gpu_com_struct_ptr);
extern int init_gpu_2d_arr(GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr, unsigned int row_num, unsigned int col_num, float *data_arr);
extern int gpu_primitive_laplacian(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr);
extern int gpu_weighed_laplacian(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr);
extern int gpu_statistic_laplacian(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr);
extern int destroy_gpu_state(GPU_COM_STRUCT *gpu_com_struct_ptr);
extern int destroy_gpu_2d_arr(GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr);

#endif