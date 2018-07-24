__kernel void one_dimension_regression(__global float *x_vector, __global float *input_abc_para, __global float *output_abc_para, __global int *para_arr)
{

}

__kernel void surface_bending(__global float *input_data_arr, __global int *para_arr, __global float *abc_input_arr, __global float *output_data_arr)
{

}

__kernel void slope_diminish(__global float *input_data_arr, __global int *para_arr, __global float *output_data_arr)
{
    unsigned int gpu_global_index = get_global_id(0);

    // Present column index for the data
    unsigned int presCol = gpu_global_index % para_arr[1];
    // Find the column direction of slope
    float zero_to_max_slope = (float)(input_data_arr[para_arr[1] - 1] - input_data_arr[0]) / (float)para_arr[1];
    // Compensate the present point with specific value according to the slope
    output_data_arr[gpu_global_index] = input_data_arr[gpu_global_index] - zero_to_max_slope * presCol;
}

__kernel void primitive_laplacian(__global float *input_data_arr, __global int *para_arr, __global float *output_data_arr)
{
    unsigned int gpu_global_index = get_global_id(0);
    
    // Calculation

    // Present row index for the data
    unsigned int presRow = gpu_global_index / para_arr[1];
    // Present column index for the data
    unsigned int presCol = gpu_global_index % para_arr[1];
    
    // Define the border of the edge points
    unsigned int left_border = (presRow < para_arr[3]) ? 0 : (presRow - para_arr[3]);
    unsigned int right_border = (presRow >= para_arr[0] - para_arr[3]) ? para_arr[0] : (presRow + para_arr[3]);
    unsigned int upper_border = (presCol < para_arr[4]) ? 0 : (presCol - para_arr[4]);
    unsigned int bottom_border = (presCol >= para_arr[1] - para_arr[4]) ? para_arr[1] : (presCol + para_arr[4]);
    
    // Inner region goes primitive laplacian
    unsigned int i, j;
    unsigned int k = 0;
    float sum_surrounding = 0;
    unsigned int tmp_product = 0;
    for(i = left_border; i < right_border; i++)
    {
        for(j = upper_border; j < bottom_border; j++)
        {
            float abs_value = (input_data_arr[tmp_product + j] - input_data_arr[gpu_global_index] > 0) ? (input_data_arr[tmp_product + j] - input_data_arr[gpu_global_index]) : (input_data_arr[gpu_global_index] - input_data_arr[tmp_product + j]);
            if((i == presRow && j == presCol) || (abs_value / input_data_arr[gpu_global_index] < 0.02f)) continue;
            k++;
            sum_surrounding += input_data_arr[tmp_product + j];
        }
        tmp_product += ((para_arr[4] << 1) + 1);
    }
    if(k <= ((para_arr[3] * para_arr[4]) >> 1))
    {
        output_data_arr[gpu_global_index] = input_data_arr[gpu_global_index];
    }
    else
    {
        output_data_arr[gpu_global_index] = (sum_surrounding / (float)k);
    }
    
    output_data_arr[gpu_global_index] = input_data_arr[gpu_global_index];
}

// Unfinished!
__kernel void weighed_laplacian(__global float *input_data_arr, __global int *para_arr, __global float *output_data_arr)
{
    
}

// Unfinished!
__kernel void statistic_laplacian(__global float *input_data_arr, __global int *para_arr, __global float *output_data_arr)
{
    
}