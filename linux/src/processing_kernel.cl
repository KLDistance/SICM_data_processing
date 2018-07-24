__kernel void slope_diminish(__global float *input_data_arr, __global int *para_arr, __global float *output_data_arr)
{
    unsigned int gpu_global_index = get_global_id(0);

    // Present row index for the data
    unsigned int presRow = gpu_global_index / para_arr[1];
    // Present column index for the data
    unsigned int presCol = gpu_global_index % para_arr[1];
    // Define the columns in each pieces for the segmentation in y direction
    unsigned int seg_col = para_arr[1] / 10;
    // Segmentation of the y direction into pieces, each with 16 columns
    unsigned int pres_slot = presCol / seg_col;

    // Left edge point entry
    unsigned int left_entry = pres_slot * seg_col;
    // Right edge point entry
    unsigned int right_entry = (pres_slot + 1) * seg_col > para_arr[1] ? (para_arr[1] - 1) : ((pres_slot + 1) * seg_col - 1);
    // Decide the slot length
    unsigned int slot_length = right_entry - left_entry + 1;

    // Left entry noise handle
    unsigned int ref_pts_num = 30;
    float tmp_left_ref_pt = 0;
    float tmp_right_ref_pt = 0;
    unsigned int i;
    unsigned int j;
    unsigned int i_init = (left_entry < ref_pts_num) ? 0 : (left_entry - ref_pts_num);
    unsigned int i_end = (left_entry + ref_pts_num > para_arr[1]) ? para_arr[1] : (left_entry + ref_pts_num);
    for(i = i_init, j = 0; i < i_end; i++)
    {
        tmp_left_ref_pt += input_data_arr[i];
        j++;
    }
    tmp_left_ref_pt /= j;

    // Right entry noise handle
    i_init = (right_entry < ref_pts_num) ? 0 : (right_entry - ref_pts_num);
    i_end = (right_entry + ref_pts_num > para_arr[1]) ? para_arr[1] : (right_entry + ref_pts_num);
    for(i = i_init, j = 0; i < i_end; i++)
    {
        tmp_right_ref_pt += input_data_arr[i];
        j++;
    }
    tmp_right_ref_pt /= j;

    // Calculate the local slope
    float local_x_slope = (tmp_right_ref_pt - tmp_left_ref_pt) / slot_length;
    // Relocate each point in the local slot
    output_data_arr[gpu_global_index] = input_data_arr[gpu_global_index] - (presCol % seg_col * local_x_slope) - tmp_left_ref_pt + 10000;
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