#include <stdio.h>
#include "parallel_processing.h"
#include "csv_ops.h"

int smooth_function()
{
    GPU_COM_STRUCT gpu_com_struct;
    GPU_DATA_2D_ARR gpu_data_2d_arr;
    CSV_STRUCT csv_struct;

    // Read from csv file
    init_csv_struct(&csv_struct, "data_files/OverallGrid.csv", 0);
    csv_reader(&csv_struct);

    // Initiate 2d array for GPU calculation
    init_gpu_2d_arr(&gpu_data_2d_arr, csv_struct.row_num, csv_struct.col_num, (float*)csv_struct.data_arr);
    init_gpu_state(&gpu_com_struct);

    // Calculation
    gpu_primitive_laplacian(&gpu_com_struct, &gpu_data_2d_arr);

    // Write data array into a new csv file
    csv_struct.result_data_arr = gpu_data_2d_arr.output_data_arr;
    csv_writer(&csv_struct);

    destroy_gpu_2d_arr(&gpu_data_2d_arr);
    destroy_gpu_state(&gpu_com_struct);

    return 0;
}

int main(void)
{
    //check_gpu_info();
    smooth_function();
    return 0;
}