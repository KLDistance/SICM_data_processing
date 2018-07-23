__kernel void slope_diminish(__global float *input_data_arr, __global int *para_arr, __global float *output_data_arr)
{
    unsigned int gpu_global_index = get_global_id(0);

    // Present row index for the data
    unsigned int presRow = gpu_global_index / para_arr[1];
    // Present column index for the data
    unsigned int presCol = gpu_global_index % para_arr[1];
    // Segmentation of the y direction into pieces, each with 16 columns
    unsigned int pres_slot = (presCol) / 16;
    unsigned int real_num_slot_column = ((pres_slot << 4) > para_arr[1]) ? (para_arr[1]) : (pres_slot << 4);

    // Left edge point entry
    unsigned int left_entry = pres_slot;
    // Right edge point entry
    unsigned int right_entry = pres_slot + 1;
    // Decide the slot length


}

__kernel void primitive_laplacian(__global float *input_data_arr, __global int *para_arr, __global float *output_data_arr)
{
    unsigned int gpu_global_index = get_global_id(0);
    /*
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
            if((i == presRow && j == presCol) || (abs_value / input_data_arr[gpu_global_index] < 0.01f)) continue;
            k++;
            sum_surrounding += input_data_arr[tmp_product + j];
        }
        tmp_product += ((para_arr[4] << 1) + 1);
    }
    if(k <= para_arr[3] * para_arr[4] / 2)
    {
        output_data_arr[gpu_global_index] = input_data_arr[gpu_global_index];
    }
    else
    {
        output_data_arr[gpu_global_index] = (sum_surrounding / (float)k);
    }
    */
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