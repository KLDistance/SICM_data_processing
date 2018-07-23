#ifndef __PARALLEL_PROCESSING_H__
#define __PARALLEL_PROCESSING_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <time.h>

// Define the property array entry pass into the GPU
#define L_PROPERTY_NUM 8

// Array row number
#define PL_ARR_ROW_NUM 0
// Array column number
#define PL_ARR_COL_NUM 1
// Total number of element in array
#define PL_ARR_TOTAL_NUM 2
// Relative number of rows related in terms of the central point
#define PL_RELATIVE_ROW 3
// Relative number of columns related in terms of the central point
#define PL_RELATIVE_COL 4
// Reciprocal of the drag coefficient
#define PL_CENTRAL_DRAG 5
// Distance coefficient for weight laplacian distance
#define WL_DISTANCE_COEFFICIENT 6
// Threshold
#define SL_THRESHOLD 7

// Define max number of programs in the kernel
#define MAX_PROGRAM_NUM 256


typedef struct
{
    char *kernel_source_str;
    size_t kernel_source_size;
    cl_int cl_ret;
    cl_platform_id platform_id;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue command_queue;
    size_t program_num;
    char *func_name[MAX_PROGRAM_NUM];
    cl_program program_arr[MAX_PROGRAM_NUM];
    cl_kernel kernel_arr[MAX_PROGRAM_NUM];
} GPU_COM_STRUCT;

typedef struct
{
    unsigned int row_num;
    unsigned int col_num;
    unsigned int data_num;
    unsigned int data_size;
    float *input_data_arr;
    float *output_data_arr;
} GPU_DATA_2D_ARR;

extern int check_gpu_info();
extern int init_gpu_state(GPU_COM_STRUCT *gpu_com_struct_ptr);
extern int init_gpu_2d_arr(GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr, unsigned int row_num, unsigned int col_num, float *data_arr);
extern int append_gpu_program(GPU_COM_STRUCT *gpu_com_struct_ptr, char *func_name);
extern int gpu_slope_diminish(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr);
extern int gpu_primitive_laplacian(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr);
extern int gpu_weighed_laplacian(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr);
extern int gpu_statistic_laplacian(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr);
extern int destroy_gpu_state(GPU_COM_STRUCT *gpu_com_struct_ptr);
extern int destroy_gpu_program_array(GPU_COM_STRUCT *gpu_com_struct_ptr);
extern int destroy_gpu_2d_arr(GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr);

#endif