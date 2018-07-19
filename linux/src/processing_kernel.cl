__kernel void primitive_laplacian(__global float *input_data_arr, __global float *output_data_arr, __global int *para_arr)
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
        
    }

}