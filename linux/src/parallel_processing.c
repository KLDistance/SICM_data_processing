#include "parallel_processing.h"

int check_gpu_info()
{
    // Parameters
    cl_platform_id *platforms = NULL;
    cl_device_id *devices = NULL;
    cl_uint num_platforms;
    cl_uint num_devices, addr_data;
    cl_int err;

    int i, j;
    char name_data[64], ext_data[4096];

    err = clGetPlatformIDs(5, NULL, &num_platforms);
    if(err < 0)
    {
        fprintf(stderr, "Could not find any available platform.\n");
        exit(1);
    }

    // Pick all the available platforms
    platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
    if(!platforms)
    {
        fprintf(stderr, "Unable to allocate space for platform info array!\n");
        exit(1);
    }
    err = clGetPlatformIDs(num_platforms, platforms, NULL);
    if(err < 0)
    {
        fprintf(stderr, "Could not find any available platform.\n");
        exit(1);
    }

    // Loops to see the device info
    printf("\nAvailable platforms and devices info in CL:\n");
    for(i = 0; i < (int)num_platforms; i++)
    {
        printf("\n---------------------------------------------------------------------------\nplatform %d:\n", j + 1);
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 1, NULL, &num_devices);
        if(err < 0)
        {
            fprintf(stderr, "Could not find any available devices!\n");
            exit(1);
        }

        // Access connected devices
        devices = (cl_device_id*)malloc(sizeof(cl_device_id) * num_devices);
        clGetDeviceIDs(platforms[j], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);

        // Loops to display all device info (contains all the CPU and GPU info) within the given platform
        for(j = 0; j < (int)num_devices; j++)
        {
            // Get device name
            memset(name_data, 0, sizeof(name_data));
            err = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(name_data), name_data, NULL);
            if(err < 0)
            {
                fprintf(stderr, "Cound not read any data.\n");
                exit(1);
            }
            
            // Get extensive data
            memset(ext_data, 0, sizeof(ext_data));
            clGetDeviceInfo(devices[i], CL_DEVICE_ADDRESS_BITS, sizeof(ext_data), &addr_data, NULL);
            clGetDeviceInfo(devices[i], CL_DEVICE_EXTENSIONS, sizeof(ext_data), ext_data, NULL);
            printf("Name: %s\nAddress_width: %u\nExtensions: %s\n", name_data, addr_data, ext_data);
        }
        free(devices);
    }
    printf("---------------------------------------------------------------------------\n\n");
    free(platforms);

    return 0;
}

int init_gpu_state(GPU_COM_STRUCT *gpu_com_struct_ptr)
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

    source_str = (char*)malloc(source_size);
    if(!source_str)
    {
        fprintf(stderr, "Unable to assign the space for CL kernel file!\n");
        fclose(fp_kernel);
        exit(1);
    }
    memset(source_str, 0, source_size);

    fread(source_str, source_size, 1, fp_kernel);
    fclose(fp_kernel);

    gpu_com_struct_ptr->kernel_source_size = source_size;
    gpu_com_struct_ptr->kernel_source_str = source_str;
    
    // Obtain the gpu platform and devices information
    cl_uint ret_num_devices;
    cl_uint ret_num_platform;
    gpu_com_struct_ptr->cl_ret = clGetPlatformIDs(1, &(gpu_com_struct_ptr->platform_id), &ret_num_platform);
    gpu_com_struct_ptr->cl_ret = clGetDeviceIDs(gpu_com_struct_ptr->platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &(gpu_com_struct_ptr->device_id), &ret_num_devices);
    
    // Create the context for OpenCL
    gpu_com_struct_ptr->context = clCreateContext(NULL, 1, &(gpu_com_struct_ptr->device_id), NULL, NULL, &(gpu_com_struct_ptr->cl_ret));

    // Create the command queue
    gpu_com_struct_ptr->command_queue = clCreateCommandQueueWithProperties(gpu_com_struct_ptr->context, gpu_com_struct_ptr->device_id, 0, &(gpu_com_struct_ptr->cl_ret));

    return 0;
}

int init_gpu_2d_arr(GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr, unsigned int row_num, unsigned int col_num, float *data_arr)
{
    // Copy in the data into the gpu_data_2d_arr
    if(!data_arr) return -1;
    gpu_data_2d_arr_ptr->input_data_arr = (float*)malloc(row_num * col_num * sizeof(float));
    if(!gpu_data_2d_arr_ptr->input_data_arr)
    {
        fprintf(stderr, "Unable to assign the space for the GPU_DATA_2D_ARR!\n");
        exit(1);
    }
    unsigned int i;
    unsigned int j;
    for(i = 0; i < row_num; i++)
    {
        unsigned int tmp_iter = i * col_num;
        for(j = 0; j < col_num; j++)
        {
            gpu_data_2d_arr_ptr->input_data_arr[tmp_iter + j] = data_arr[tmp_iter + j];
        }
    }
    gpu_data_2d_arr_ptr->row_num = row_num;
    gpu_data_2d_arr_ptr->col_num = col_num;

    // Assign new space for output_data_arr
    gpu_data_2d_arr_ptr->output_data_arr = (float*)malloc(row_num * col_num * sizeof(float));
    return 0;
}

int gpu_primitive_laplacian(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr)
{
    // Set basic options to the parallel computation through array
    int para_arr[L_PROPERTY_NUM] = {0};
    para_arr[PL_ARR_ROW_NUM] = gpu_data_2d_arr_ptr->row_num;
    para_arr[PL_ARR_COL_NUM] = gpu_data_2d_arr_ptr->col_num;
    para_arr[PL_RELATIVE_ROW] = 5;
    para_arr[PL_RELATIVE_COL] = 5;
    para_arr[PL_CENTRAL_DRAG] = 1;
    
    // Calculate the data size and data number
    gpu_data_2d_arr_ptr->data_num = gpu_data_2d_arr_ptr->row_num * gpu_data_2d_arr_ptr->col_num;
    gpu_data_2d_arr_ptr->data_size = gpu_data_2d_arr_ptr->data_num * sizeof(float);

    // Create buffer for each vector on the device
    cl_mem input_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->input_data_arr, &(gpu_com_struct_ptr->cl_ret));
    cl_mem para_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, L_PROPERTY_NUM * sizeof(int), para_arr, &(gpu_com_struct_ptr->cl_ret));
    cl_mem output_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_WRITE_ONLY, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->output_data_arr, &(gpu_com_struct_ptr->cl_ret));

    // Copy the input and parameter array into the GPU memory
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, input_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->input_data_arr, 0, NULL, NULL);
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, para_arr_mem, CL_TRUE, 0, L_PROPERTY_NUM * sizeof(int), para_arr, 0, NULL, NULL);

    // Create program
    cl_program program = clCreateProgramWithSource(gpu_com_struct_ptr->context, 1, (const char**)&(gpu_com_struct_ptr->kernel_source_str), (const size_t*)&(gpu_com_struct_ptr->kernel_source_size), &(gpu_com_struct_ptr->cl_ret));
    // Construct program
    gpu_com_struct_ptr->cl_ret = clBuildProgram(program, 1, &(gpu_com_struct_ptr->device_id), NULL, NULL, NULL);
    // Create OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "primitive_laplacian", &(gpu_com_struct_ptr->cl_ret));

    // Set kernel parameters
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&(gpu_data_2d_arr_ptr->input_data_arr));
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&para_arr);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&(gpu_data_2d_arr_ptr->output_data_arr));

    // Execute the kernel
    size_t global_item_size = gpu_data_2d_arr_ptr->data_num;
    size_t local_item_size = 64;
    gpu_com_struct_ptr->cl_ret = clEnqueueNDRangeKernel(gpu_com_struct_ptr->command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read RAM buffer output_arr_mem input the local buffer output_data_arr
    gpu_com_struct_ptr->cl_ret = clEnqueueReadBuffer(gpu_com_struct_ptr->command_queue, output_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->output_data_arr, 0, NULL, NULL);

    // Close and flush the CL local structures
    gpu_com_struct_ptr->cl_ret = clFlush(gpu_com_struct_ptr->command_queue);
    gpu_com_struct_ptr->cl_ret = clFinish(gpu_com_struct_ptr->command_queue);
    gpu_com_struct_ptr->cl_ret = clReleaseKernel(kernel);
    gpu_com_struct_ptr->cl_ret = clReleaseProgram(program);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(input_arr_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(output_arr_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(para_arr_mem);

    return 0;
}

int gpu_weighed_laplacian(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr)
{
    // Set basic options to the parallel computation through array
    int para_arr[L_PROPERTY_NUM] = {0};
    para_arr[PL_ARR_ROW_NUM] = gpu_data_2d_arr_ptr->row_num;
    para_arr[PL_ARR_COL_NUM] = gpu_data_2d_arr_ptr->col_num;
    para_arr[PL_RELATIVE_ROW] = 5;
    para_arr[PL_RELATIVE_COL] = 5;
    para_arr[PL_CENTRAL_DRAG] = 1;
    para_arr[WL_DISTANCE_COEFFICIENT] = 2;
    
    // Calculate the data size and data number
    gpu_data_2d_arr_ptr->data_num = gpu_data_2d_arr_ptr->row_num * gpu_data_2d_arr_ptr->col_num;
    gpu_data_2d_arr_ptr->data_size = gpu_data_2d_arr_ptr->data_num * sizeof(float);

    // Create buffer for each vector on the device
    cl_mem input_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->input_data_arr, &(gpu_com_struct_ptr->cl_ret));
    cl_mem para_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, L_PROPERTY_NUM * sizeof(int), para_arr, &(gpu_com_struct_ptr->cl_ret));
    cl_mem output_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_WRITE_ONLY, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->output_data_arr, &(gpu_com_struct_ptr->cl_ret));
    
    // Copy the input and parameter array into the GPU memory
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, input_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->input_data_arr, 0, NULL, NULL);
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, para_arr_mem, CL_TRUE, 0, L_PROPERTY_NUM * sizeof(int), para_arr, 0, NULL, NULL);

    // Create program
    cl_program program = clCreateProgramWithSource(gpu_com_struct_ptr->context, 1, (const char**)&(gpu_com_struct_ptr->kernel_source_str), (const size_t*)&(gpu_com_struct_ptr->kernel_source_size), &(gpu_com_struct_ptr->cl_ret));
    // Construct program
    gpu_com_struct_ptr->cl_ret = clBuildProgram(program, 1, &(gpu_com_struct_ptr->device_id), NULL, NULL, NULL);
    // Create OpenCL kernel (ERROR!)
    cl_kernel kernel = clCreateKernel(program, "weighed_laplacian", &(gpu_com_struct_ptr->cl_ret));
    printf("%d\n", gpu_com_struct_ptr->cl_ret);

    // Set kernel parameters
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&(gpu_data_2d_arr_ptr->input_data_arr));
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&para_arr);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&(gpu_data_2d_arr_ptr->output_data_arr));

    // Execute the kernel
    size_t global_item_size = gpu_data_2d_arr_ptr->data_num;
    size_t local_item_size = 128;
    gpu_com_struct_ptr->cl_ret = clEnqueueNDRangeKernel(gpu_com_struct_ptr->command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read RAM buffer output_arr_mem input the local buffer output_data_arr
    gpu_com_struct_ptr->cl_ret = clEnqueueReadBuffer(gpu_com_struct_ptr->command_queue, output_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->output_data_arr, 0, NULL, NULL);

    // Close and flush the CL local structures
    gpu_com_struct_ptr->cl_ret = clFlush(gpu_com_struct_ptr->command_queue);
    gpu_com_struct_ptr->cl_ret = clFinish(gpu_com_struct_ptr->command_queue);
    gpu_com_struct_ptr->cl_ret = clReleaseKernel(kernel);
    gpu_com_struct_ptr->cl_ret = clReleaseProgram(program);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(input_arr_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(output_arr_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(para_arr_mem);
    /*
    unsigned int i, j;
    for(i = 0; i < gpu_data_2d_arr_ptr->row_num; i++)
    {
        unsigned int tmp_product = i * gpu_data_2d_arr_ptr->col_num;
        for(j = 0; j < gpu_data_2d_arr_ptr->col_num; j++)
        {
            printf("\t%f", gpu_data_2d_arr_ptr->output_data_arr[tmp_product + j]);
        }
        printf("\n");
    }
    */
    return 0;
}

int gpu_statistic_laplacian(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr)
{
    // Set basic options to the parallel computation through array
    int para_arr[L_PROPERTY_NUM] = {0};
    para_arr[PL_ARR_ROW_NUM] = gpu_data_2d_arr_ptr->row_num;
    para_arr[PL_ARR_COL_NUM] = gpu_data_2d_arr_ptr->col_num;
    para_arr[PL_RELATIVE_ROW] = 5;
    para_arr[PL_RELATIVE_COL] = 5;
    para_arr[PL_CENTRAL_DRAG] = 1;
    para_arr[WL_DISTANCE_COEFFICIENT] = 2;
    para_arr[SL_THRESHOLD] = 3;
    
    // Calculate the data size and data number
    gpu_data_2d_arr_ptr->data_num = gpu_data_2d_arr_ptr->row_num * gpu_data_2d_arr_ptr->col_num;
    gpu_data_2d_arr_ptr->data_size = gpu_data_2d_arr_ptr->data_num * sizeof(float);

    // Create buffer for each vector on the device
    cl_mem input_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->input_data_arr, &(gpu_com_struct_ptr->cl_ret));
    cl_mem para_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, L_PROPERTY_NUM * sizeof(int), para_arr, &(gpu_com_struct_ptr->cl_ret));
    cl_mem output_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_WRITE_ONLY, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->output_data_arr, &(gpu_com_struct_ptr->cl_ret));

    // Copy the input and parameter array into the GPU memory
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, input_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->input_data_arr, 0, NULL, NULL);
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, para_arr_mem, CL_TRUE, 0, L_PROPERTY_NUM * sizeof(int), para_arr, 0, NULL, NULL);
    // Create program
    cl_program program = clCreateProgramWithSource(gpu_com_struct_ptr->context, 1, (const char**)&(gpu_com_struct_ptr->kernel_source_str), (const size_t*)&(gpu_com_struct_ptr->kernel_source_size), &(gpu_com_struct_ptr->cl_ret));
    // Construct program
    gpu_com_struct_ptr->cl_ret = clBuildProgram(program, 1, &(gpu_com_struct_ptr->device_id), NULL, NULL, NULL);
    // Create OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "statistic_laplacian", &(gpu_com_struct_ptr->cl_ret));

    // Set kernel parameters
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&(gpu_data_2d_arr_ptr->input_data_arr));
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&para_arr);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&(gpu_data_2d_arr_ptr->output_data_arr));

    // Execute the kernel
    size_t global_item_size = gpu_data_2d_arr_ptr->data_num;
    size_t local_item_size = 128;
    gpu_com_struct_ptr->cl_ret = clEnqueueNDRangeKernel(gpu_com_struct_ptr->command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read RAM buffer output_arr_mem input the local buffer output_data_arr
    gpu_com_struct_ptr->cl_ret = clEnqueueReadBuffer(gpu_com_struct_ptr->command_queue, output_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->output_data_arr, 0, NULL, NULL);

    // Close and flush the CL local structures
    gpu_com_struct_ptr->cl_ret = clFlush(gpu_com_struct_ptr->command_queue);
    gpu_com_struct_ptr->cl_ret = clFinish(gpu_com_struct_ptr->command_queue);
    gpu_com_struct_ptr->cl_ret = clReleaseKernel(kernel);
    gpu_com_struct_ptr->cl_ret = clReleaseProgram(program);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(input_arr_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(output_arr_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(para_arr_mem);

    return 0;
}

int destroy_gpu_state(GPU_COM_STRUCT *gpu_com_struct_ptr)
{
    // Destroy the global CL objects
    gpu_com_struct_ptr->cl_ret = clReleaseCommandQueue(gpu_com_struct_ptr->command_queue);
    gpu_com_struct_ptr->cl_ret = clReleaseContext(gpu_com_struct_ptr->context);

    return 0;
}

int destroy_gpu_2d_arr(GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr)
{
    // Clear the dynamic-allocated memory
    if(gpu_data_2d_arr_ptr->input_data_arr)
    {
        free(gpu_data_2d_arr_ptr->input_data_arr);
        gpu_data_2d_arr_ptr->input_data_arr = NULL;
    }
    if(gpu_data_2d_arr_ptr->output_data_arr)
    {
        free(gpu_data_2d_arr_ptr->output_data_arr);
        gpu_data_2d_arr_ptr->output_data_arr = NULL;
    }

    // Zero-set the matrix indicators
    gpu_data_2d_arr_ptr->row_num = 0;
    gpu_data_2d_arr_ptr->col_num = 0;
    gpu_data_2d_arr_ptr->data_size = 0;
    gpu_data_2d_arr_ptr->data_num = 0;

    return 0;
}