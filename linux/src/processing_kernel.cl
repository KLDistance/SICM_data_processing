__kernel void base_locate(__global float *input_arr, __global int *para_arr, __global float *x_vector)
{
    unsigned int index = get_global_id(0);
    unsigned int pres_col = index % para_arr[1];
    x_vector[pres_col] = (x_vector[pres_col] + input_arr[index]) / 2;
}

__kernel void one_dimension_regression(__global float *x_vector, __global float *input_abc_para, __global int *para_arr, __global float *learning_rate_para, __global float *output_vector_a, __global float *output_vector_b, __global float *output_vector_c)
{
    unsigned int index = get_global_id(0);
    float x_square = index * index;
    float single_deviation = input_abc_para[0] * x_square + input_abc_para[1] * index + input_abc_para[2] - x_vector[index];
    float proc_deviation = single_deviation / para_arr[1];
    output_vector_a[index] = proc_deviation * x_square * learning_rate_para[0];
    output_vector_b[index] = proc_deviation * index * learning_rate_para[1];
    output_vector_c[index] = proc_deviation * learning_rate_para[2];
}

__kernel void surface_bending(__global float *input_data_arr, __global int *para_arr, __global float *input_abc_para, __global float *output_data_arr)
{
    unsigned int index = get_global_id(0);
    unsigned int pres_col = index % para_arr[1];
    unsigned int pres_row = index % para_arr[1];
    float x_square = pres_col * pres_col;
    float polynomial_value = input_abc_para[0] * x_square + input_abc_para[1] * pres_col + input_abc_para[2];
    output_data_arr[index] = input_data_arr[index] - polynomial_value + para_arr[5];
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
            if((i == presRow && j == presCol) || (abs_value / input_data_arr[gpu_global_index] < 0.012f)) continue;
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
    
    //output_data_arr[gpu_global_index] = input_data_arr[gpu_global_index];
}

// Unfinished!
__kernel void weighed_laplacian(__global float *input_data_arr, __global int *para_arr, __global float *output_data_arr)
{
    
}

// Unfinished!
__kernel void statistic_laplacian(__global float *input_data_arr, __global int *para_arr, __global float *output_data_arr)
{
    
}