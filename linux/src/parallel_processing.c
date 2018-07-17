#include "parallel_processing.h"

extern int init_gpu_state(GPU_COM_STRUCT *gpu_com_struct_ptr)
{
    // Read in the CL kernel file
    FILE *fp_kernel = NULL;
    char *source_str = NULL;
    size_t source_size = 0;

    fp_kernel = fopen("src/processing_kernel.cl", "r");
    if(!fp_kernel)
    {
        fprintf(stderr, "Unable to open the kernel file!\n");
        exit(1);
    }
    fseek(fp_kernel, 0, SEEK_END);
    source_size = ftell(fp_kernel);
    fseek(fp_kernel, 0, SEEK_SET);

    source_ptr = (char*)malloc(source_size);
    if(!source_ptr)
    {
        fprintf(stderr, "Unable to assign the space for CL kernel file!\n");
        fclose(fp_kernel);
        exit(1);
    }
    memset(source_ptr, 0, source_size);

    fread(source_str, source_size, 1, fp_kernel);
    fclose(fp_kernel);

    gpu_com_struct_ptr->kernel_source_size = source_size;

    // Obtain the gpu platform and devices information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platform;
    gpu_com_struct_ptr->cl_ret = clGetPlatformIDs(1, &platform_id, &ret_num_platform);
    gpu_com_struct_ptr->cl_ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
    
    // Create the context for OpenCL
    gpu_com_struct_ptr->context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &(gpu_com_struct_ptr->cl_ret));

    // Create the command queue
    gpu_com_struct_ptr->command_queue = clCreateCommandQueueWithProperties(gpu_com_struct_ptr->context, device_id, 0, &(gpu_com_struct_ptr->cl_ret));

    return 0;
}

extern int init_gpu_2d_arr(GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr, unsigned int row_num, unsigned int col_num, float *data_arr)
{
    // Copy in the data into the gpu_data_2d_arr
    if(!data_arr) return -1;
    gpu_data_2d_arr_ptr->input_data_arr = data_arr;
    gpu_data_2d_arr_ptr->row_num = row_num;
    gpu_data_2d_arr_ptr->col_num = col_num;

    // Assign new space for output_data_arr
    gpu_data_2d_arr_ptr->output_data_arr = (float*)malloc(row_num * col_num * sizeof(float));
    return 0;
}

extern int gpu_primitive_laplacian(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr)
{
    // Set basic options to the parallel computation through array
    int[L_PROPERTY_NUM] para_arr = {0};
    
    // Calculate the data size and data number
    gpu_data_2d_arr_ptr->data_num = gpu_data_2d_arr_ptr->row_num * gpu_data_2d_arr_ptr->col_num;
    gpu_data_2d_arr_ptr->data_size = gpu_data_2d_arr_ptr->data_num * sizeof(float);

    // Create buffer for each vector on the device
    cl_mem input_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->input_data_arr, &(gpu_com_struct_ptr->cl_ret));
    cl_mem output_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_WRITE_ONLY, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->output_data_arr, &(gpu_com_struct_ptr->cl_ret));
    cl_mem para_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, L_PROPERTY_NUM * sizeof(int), para_arr, &(gpu_com_struct_ptr->cl_ret));

    // Copy the input and parameter array into the GPU memory
    gpu_data_2d_arr_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, input_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->input_data_arr, 0, NULL, NULL);
    gpu_data_2d_arr_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, parar_arr_mem, CL_TRUE, 0, L_PROPERTY * sizeof(int), para_arr, 0, NULL, NULL);

    // Create program
    cl_program program = clCreateProgramWithSource(gpu_com_struct_ptr->context, 1, (const char**)&(gpu_com_struct_ptr->kernel_source_str), (const size_t*)&(gpu_com_struct_ptr->kernel_source_size), &(gpu_com_struct_ptr->cl_ret));
    // Construct program

    return 0;
}