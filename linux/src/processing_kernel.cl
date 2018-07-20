#pragma OPENCL EXTENSION cl_khr_byte_addressable_store:enable\n

__kernel void primitive_laplacian(__global float *input_data_arr, __global int *para_arr__global, float *output_data_arr)
{
    int gpu_global_index = get_global_id(0);

    // Calculation

    // Present row index for the data
    unsigned int presRow = gpu_global_index / para_arr[1];
    // Present column index for the data
    unsigned int presCol = gpu_global_index % para_arr[i];

    // Judge whether present point is able to execute a thorough laplacian process
    if(presRow < para_arr[2] || presRow > para_arr[0] - para_arr[2] || presCol < para_arr[3] || presCol > para_arr[1] - para_arr[3])
    {
        // On the edge region, go without any handle
        output_data_arr[gpu_global_index] = input_data_arr[gpu_global_index];
    }
    else
    {
        // Inner region goes primitive laplacian
        unsigned int i, j;
        unsigned int k = 0;
        float delta_surrounding = 0;
        for(i = presRow - para_arr[2]; i < presRow + para_arr[2]; i++)
        {
            unsigned int tmp_product = i * (para_arr[3] * 2 + 1);
            for(j = presCol - para_arr[3]; j < presCol + para_arr[3]; j++)
            {
                if(i == presRow && j == presCol) continue;
                k++;
                delta_surrounding += (input_data_arr[tmp_product + j] - input_data_arr[gpu_global_index]);
            }
        }
        output_data_arr[gpu_global_index] -= (delta_surrounding / (float)k / (float)para_arr[4]);
    }
}

__kernel void weighed_laplacian(__global float *input_data_arr, __global int *para_arr__global, float *output_data_arr)
{
    int gpu_global_index = get_global_id(0);

    // Calculation

    // Present row index for the data
    unsigned int presRow = gpu_global_index / para_arr[1];
    // Present column index for the data
    unsigned int presCol = gpu_global_index % para_arr[i];

    // Judge whether present point is able to execute a thorough laplacian process
    if(presRow < para_arr[2] || presRow > para_arr[0] - para_arr[2] || presCol < para_arr[3] || presCol > para_arr[1] - para_arr[3])
    {
        // On the edge region, go without any handle
        output_data_arr[gpu_global_index] = input_data_arr[gpu_global_index];
    }
    else
    {
        // Inner region goes primitive laplacian
        unsigned int i, j;
        unsigned int k = 0;
        float delta_surrounding = 0;
        float destance_denominator = 0;
        for(i = presRow - para_arr[2]; i < presRow + para_arr[2]; i++)
        {
            unsigned int tmp_product = i * (para_arr[3] * 2 + 1);
            for(j = presCol - para_arr[3]; j < presCol + para_arr[3]; j++)
            {
                if(i == presRow && j == presCol) continue;
                k++;
                destance_denominator = sqrt((i - presRow) * (i - presRow) + (j - presCol) * (j - presCol));
                delta_surrounding += ((input_data_arr[tmp_product + j] - input_data_arr[gpu_global_index]) / destance_denominator);
            }
        }
        output_data_arr[gpu_global_index] -= (delta_surrounding / (float)k / (float)para_arr[4]);
    }
}

// Unfinished!
__kernel void statistic_laplacian(__global float *input_data_arr, __global int *para_arr__global, float *output_data_arr)
{
    int gpu_global_index = get_global_id(0);

    // Calculation

    // Present row index for the data
    unsigned int presRow = gpu_global_index / para_arr[1];
    // Present column index for the data
    unsigned int presCol = gpu_global_index % para_arr[i];
    // Statistical indicator for the exceptional points
    unsigned int exc_pts = 0;

    // Judge whether present point is able to execute a thorough laplacian process
    if(presRow < para_arr[2] || presRow > para_arr[0] - para_arr[2] || presCol < para_arr[3] || presCol > para_arr[1] - para_arr[3])
    {
        // On the edge region, go without any handle
        output_data_arr[gpu_global_index] = input_data_arr[gpu_global_index];
    }
    else
    {
        // Inner region goes primitive laplacian
        unsigned int i, j;
        unsigned int k = 0;
        float delta_surrounding = 0;
        float destance_denominator = 0;
        for(i = presRow - para_arr[2]; i < presRow + para_arr[2]; i++)
        {
            unsigned int tmp_product = i * (para_arr[3] * 2 + 1);
            for(j = presCol - para_arr[3]; j < presCol + para_arr[3]; j++)
            {
                if(i == presRow && j == presCol) continue;
                k++;
                destance_denominator = sqrt((i - presRow) * (i - presRow) + (j - presCol) * (j - presCol));
                delta_surrounding += ((input_data_arr[tmp_product + j] - input_data_arr[gpu_global_index]) / destance_denominator);
            }
        }
        output_data_arr[gpu_global_index] -= (delta_surrounding / (float)k / (float)para_arr[4]);
    }
}