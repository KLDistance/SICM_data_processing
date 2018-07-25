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

static float f_abs(float input)
{
    if(input >= 0) return input;
    else return -input;
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
    // Since there is OpenCL 1.1, use "clCreateCommandQueue"
    // After OpenCL 2.0, use "clCreateCommandQueueWithProperties" !
    gpu_com_struct_ptr->command_queue = clCreateCommandQueue(gpu_com_struct_ptr->context, gpu_com_struct_ptr->device_id, 0, &(gpu_com_struct_ptr->cl_ret));

    // Init the program and kernel array
    gpu_com_struct_ptr->program_num = 0;
    memset(gpu_com_struct_ptr->program_arr, 0, sizeof(cl_program) * MAX_PROGRAM_NUM);
    memset(gpu_com_struct_ptr->kernel_arr, 0, sizeof(cl_kernel) * MAX_PROGRAM_NUM);
    memset(gpu_com_struct_ptr->func_name, 0, sizeof(char*));
    
    return 0;
}

int append_gpu_program(GPU_COM_STRUCT *gpu_com_struct_ptr, char *func_name)
{
    // Build program
    gpu_com_struct_ptr->program_arr[gpu_com_struct_ptr->program_num] = clCreateProgramWithSource(gpu_com_struct_ptr->context, 1, (const char**)&(gpu_com_struct_ptr->kernel_source_str), (const size_t*)&(gpu_com_struct_ptr->kernel_source_size), &(gpu_com_struct_ptr->cl_ret));
    gpu_com_struct_ptr->cl_ret = clBuildProgram(gpu_com_struct_ptr->program_arr[gpu_com_struct_ptr->program_num], 1, &(gpu_com_struct_ptr->device_id), NULL, NULL, NULL);
    
    // Generate kernel
    gpu_com_struct_ptr->kernel_arr[gpu_com_struct_ptr->program_num] = clCreateKernel(gpu_com_struct_ptr->program_arr[gpu_com_struct_ptr->program_num], func_name, &(gpu_com_struct_ptr->cl_ret));

    // Copy the name
    gpu_com_struct_ptr->func_name[gpu_com_struct_ptr->program_num] = (char*)malloc(64);
    strncpy(gpu_com_struct_ptr->func_name[gpu_com_struct_ptr->program_num], func_name, strlen(func_name));

    // Increment the program number but first to return it as an id
    return gpu_com_struct_ptr->program_num++;
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

    // Calculate the data size and data number
    gpu_data_2d_arr_ptr->data_num = row_num * col_num;
    gpu_data_2d_arr_ptr->data_size = gpu_data_2d_arr_ptr->data_num * sizeof(float);

    // Assign new space for output_data_arr
    gpu_data_2d_arr_ptr->output_data_arr = (float*)malloc(gpu_data_2d_arr_ptr->data_size);
    if(!gpu_data_2d_arr_ptr->output_data_arr)
    {
        fprintf(stderr, "Unable to allocate space for output array!\n");
        exit(1);
    }
    memset(gpu_data_2d_arr_ptr->output_data_arr, 0, gpu_data_2d_arr_ptr->data_size);
    
    return 0;
}

int gpu_slope_diminish(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr)
{
    // Set basic options to the parallel computation through array
    int para_arr[L_PROPERTY_NUM] = {0};
    para_arr[PL_ARR_ROW_NUM] = gpu_data_2d_arr_ptr->row_num;
    para_arr[PL_ARR_COL_NUM] = gpu_data_2d_arr_ptr->col_num;
    para_arr[PL_ARR_TOTAL_NUM] = gpu_data_2d_arr_ptr->data_num;

    // Init a empty array for the output
    float *tmp_output_arr = (float*)malloc(gpu_data_2d_arr_ptr->data_size);
    if(!tmp_output_arr)
    {
        fprintf(stderr, "Unable to allocate space for the tmp array!\n");
        exit(1);
    }
    memset(tmp_output_arr, 0, gpu_data_2d_arr_ptr->data_size);

    // Create buffer for each vector on the device
    cl_mem input_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, gpu_data_2d_arr_ptr->data_size, NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem para_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, L_PROPERTY_NUM * sizeof(int), NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem output_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_WRITE_ONLY, gpu_data_2d_arr_ptr->data_size, NULL, &(gpu_com_struct_ptr->cl_ret));

    // Copy the input and parameter into the GPU memory
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, input_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->input_data_arr, 0, NULL, NULL);
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, para_arr_mem, CL_TRUE, 0, L_PROPERTY_NUM * sizeof(int), para_arr, 0, NULL, NULL);

    // Get program id
    int program_id = append_gpu_program(gpu_com_struct_ptr, "slope_diminish");

    // Set kernel parameters
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[program_id], 0, sizeof(cl_mem), (void*)&input_arr_mem);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[program_id], 1, sizeof(cl_mem), (void*)&para_arr_mem);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[program_id], 2, sizeof(cl_mem), (void*)&output_arr_mem);

    // Execute the kernel 
    size_t global_item_size = gpu_data_2d_arr_ptr->data_num;
    size_t local_item_size = 100;
    gpu_com_struct_ptr->cl_ret = clEnqueueNDRangeKernel(gpu_com_struct_ptr->command_queue, gpu_com_struct_ptr->kernel_arr[program_id], 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read RAM buffer
    gpu_com_struct_ptr->cl_ret = clEnqueueReadBuffer(gpu_com_struct_ptr->command_queue, output_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, tmp_output_arr, 0, NULL, NULL);
    
    // Swap the array pointer
    free(gpu_data_2d_arr_ptr->input_data_arr);
    gpu_data_2d_arr_ptr->input_data_arr = tmp_output_arr;
    
    // Close and flush the CL local structures
    gpu_com_struct_ptr->cl_ret = clFlush(gpu_com_struct_ptr->command_queue);
    gpu_com_struct_ptr->cl_ret = clFinish(gpu_com_struct_ptr->command_queue);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(input_arr_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(para_arr_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(output_arr_mem);

    return 0;
}

int gpu_1d_curve_bending(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr, CURVE_PROPERTY *curve_property_ptr)
{
    // Set basic options through array
    int para_arr[L_PROPERTY_NUM] = {0};
    para_arr[PL_ARR_ROW_NUM] = gpu_data_2d_arr_ptr->row_num;
    para_arr[PL_ARR_COL_NUM] = gpu_data_2d_arr_ptr->col_num;
    para_arr[PL_ARR_TOTAL_NUM] = gpu_data_2d_arr_ptr->data_num;
    para_arr[PL_CENTRAL_DRAG] = 10000;      // Here central drag is abort, using elevating property of the whole image instead

    // Set learning rate for the a b c
    float ml_property[ML_PROPERTY_NUM] = {0.0f, 0.0f, 0.0f};
    ml_property[0] = curve_property_ptr->a_learning_rate;      // Learning rate for a 
    ml_property[1] = curve_property_ptr->b_learning_rate;      // Learning rate for b
    ml_property[2] = curve_property_ptr->c_learning_rate;      // Learning rate for c

    float abc_para_input[3] = {0};
    abc_para_input[0] = curve_property_ptr->init_a;                     // Parameter of initial a in fx
    abc_para_input[1] = curve_property_ptr->init_b;                     // Parameter of initial b in fx
    abc_para_input[2] = curve_property_ptr->init_c;                       // Parameter of initial c in fx

    // Parameters
    unsigned int i, j, k;
    unsigned int max_iteration = curve_property_ptr->iteration_num;      // Max iteration number for regression model

    // Allocate the space for x vector input and copy in the data
    float *x_vector = (float*)malloc(gpu_data_2d_arr_ptr->col_num * sizeof(float));
    if(!x_vector)
    {
        fprintf(stderr, "Unable to allocate space for x input vector!\n");
        exit(1);
    }
    for(i = 0; i < gpu_data_2d_arr_ptr->col_num; i++)
    {
        x_vector[i] = gpu_data_2d_arr_ptr->input_data_arr[i];
    }

    // Allocate three lists for a, b, c tmp vector output
    float *tmp_vector_output[3] = {NULL, NULL, NULL};
    for(i = 0; i < 3; i++)
    {
        tmp_vector_output[i] = (float*)malloc(gpu_data_2d_arr_ptr->col_num * sizeof(float));
        if(!tmp_vector_output[i])
        {
            fprintf(stderr, "Error in allocate the %u memory for the tmp output abc array.", i);
            exit(1);
        }
    }
    
    // Append the regression cl kernel into the array
    int regression_id = append_gpu_program(gpu_com_struct_ptr, "one_dimension_regression");

    // Create the constant GPU block memory
    cl_mem x_vector_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, gpu_data_2d_arr_ptr->col_num * sizeof(float), NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem abc_para_input_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, 3 * sizeof(float), NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem para_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, L_PROPERTY_NUM * sizeof(int), NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem learning_rate_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, ML_PROPERTY_NUM * sizeof(float), NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem tmp_vector_output_mem[3];
    for(i = 0; i < 3; i++)
    {
        tmp_vector_output_mem[i] = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_WRITE_ONLY, gpu_data_2d_arr_ptr->col_num * sizeof(float), NULL, &(gpu_com_struct_ptr->cl_ret));
    }

    // Write the constant memory into the GPU buffer (Under test!)
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, x_vector_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->col_num * sizeof(float), x_vector, 0, NULL, NULL);
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, para_arr_mem, CL_TRUE, 0, L_PROPERTY_NUM * sizeof(int), para_arr, 0, NULL, NULL);
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, learning_rate_mem, CL_TRUE, 0, ML_PROPERTY_NUM * sizeof(float), ml_property, 0, NULL, NULL);

    // First to iterate the quadratic regression for the 1d curve model
    // Set the error permitted
    float err_tolerant = 2E-5;
    for(i = 0; i < max_iteration; i++)
    {
        // printf("iter: %u, a: %f, b: %f, c: %f %f\n", i, abc_para_input[0], abc_para_input[1], abc_para_input[2], x_vector[0]);
        // Store the tmp a, b, c
        float tmp_abc[3] = {0, 0, 0};
        tmp_abc[0] = abc_para_input[0];
        tmp_abc[1] = abc_para_input[1];
        tmp_abc[2] = abc_para_input[2];

        gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, abc_para_input_mem, CL_TRUE, 0, 3 * sizeof(float), abc_para_input, 0, NULL, NULL);

        // Set kernel arg (Do not clear whether exception exists)
        gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[regression_id], 0, sizeof(cl_mem), (void*)&x_vector_mem);
        gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[regression_id], 1, sizeof(cl_mem), (void*)&abc_para_input_mem);
        gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[regression_id], 2, sizeof(cl_mem), (void*)&para_arr_mem);
        gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[regression_id], 3, sizeof(cl_mem), (void*)&learning_rate_mem);
        for(j = 0; j < 3; j++)
        {
            gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[regression_id], j + 4, sizeof(cl_mem), (void*)&(tmp_vector_output_mem[j]));
        }

        size_t global_item_size = gpu_data_2d_arr_ptr->col_num;
        size_t local_item_size = 1;
        gpu_com_struct_ptr->cl_ret = clEnqueueNDRangeKernel(gpu_com_struct_ptr->command_queue, gpu_com_struct_ptr->kernel_arr[regression_id], 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

        // Read the data out
        for(j = 0; j < 3; j++)
        {
            gpu_com_struct_ptr->cl_ret = clEnqueueReadBuffer(gpu_com_struct_ptr->command_queue, tmp_vector_output_mem[j], CL_TRUE, 0, gpu_data_2d_arr_ptr->col_num * sizeof(float), tmp_vector_output[j], 0, NULL, NULL);
        }
        
        // Change the value of a, b, c
        float tmp_sum;
        for(j = 0; j < 3; j++)
        {
            tmp_sum = 0;
            for(k = 0; k < gpu_data_2d_arr_ptr->col_num; k++)
            {
                tmp_sum += tmp_vector_output[j][k];
            }
            tmp_abc[j] -= tmp_sum;
        }

        // Absolute value of a, b, c change under threshold will stop the iteration
        int a_slope_exceed = f_abs((tmp_abc[0] - abc_para_input[0]) / tmp_abc[0]) < err_tolerant;
        int b_slope_exceed = f_abs((tmp_abc[1] - abc_para_input[1]) / tmp_abc[1]) < err_tolerant;
        int c_slope_exceed = f_abs((tmp_abc[2] - abc_para_input[2]) / tmp_abc[2]) < err_tolerant;
        // printf("%d %d %d\n", a_slope_exceed, b_slope_exceed, c_slope_exceed);

        // Swap value with the original parameters
        abc_para_input[0] = tmp_abc[0];
        abc_para_input[1] = tmp_abc[1];
        abc_para_input[2] = tmp_abc[2];

        // Judge whether break the loop
        if(a_slope_exceed && b_slope_exceed && c_slope_exceed) break;
    }

    printf("Last iter: %u, a: %f, b: %f, c: %f %f\n", i, abc_para_input[0], abc_para_input[1], abc_para_input[2], x_vector[0]);

    // Then process the data in the 2d array
    // Allocate for the tmp memory for the output
    float *tmp_output_arr = (float*)malloc(gpu_data_2d_arr_ptr->data_size);
    if(!tmp_output_arr)
    {
        fprintf(stderr, "Unable to allocate space for tmp output data array!\n");
        exit(1);
    }

    // Append bending function kernel
    int bending_id = append_gpu_program(gpu_com_struct_ptr, "surface_bending");

    // Create GPU memory for bending process
    cl_mem input_2d_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, gpu_data_2d_arr_ptr->data_size, NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem output_2d_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_WRITE_ONLY, gpu_data_2d_arr_ptr->data_size, NULL, &(gpu_com_struct_ptr->cl_ret));

    // Write data into GPU memory
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, input_2d_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->input_data_arr, 0, NULL, NULL);

    // Set kernel arg
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[bending_id], 0, sizeof(cl_mem), (void*)&input_2d_arr_mem);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[bending_id], 1, sizeof(cl_mem), (void*)&para_arr_mem);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[bending_id], 2, sizeof(cl_mem), (void*)&abc_para_input_mem);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[bending_id], 3, sizeof(cl_mem), (void*)&output_2d_arr_mem);

    size_t global_item_size_for_2d_arr = gpu_data_2d_arr_ptr->data_num;
    size_t local_item_size_for_2d_arr = 100;
    gpu_com_struct_ptr->cl_ret = clEnqueueNDRangeKernel(gpu_com_struct_ptr->command_queue, gpu_com_struct_ptr->kernel_arr[bending_id], 1, NULL, &global_item_size_for_2d_arr, &local_item_size_for_2d_arr, 0, NULL, NULL);

    // Read the data out
    gpu_com_struct_ptr->cl_ret = clEnqueueReadBuffer(gpu_com_struct_ptr->command_queue, output_2d_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, tmp_output_arr, 0, NULL, NULL);

    // Swap the array pointer
    free(gpu_data_2d_arr_ptr->input_data_arr);
    gpu_data_2d_arr_ptr->input_data_arr = tmp_output_arr;

    // Free memory
    free(x_vector);
    for(i = 0; i < 3; i++)
    {
        free(tmp_vector_output[i]);
    }

    // Close and flush the local cl structures
    gpu_com_struct_ptr->cl_ret = clFlush(gpu_com_struct_ptr->command_queue);
    gpu_com_struct_ptr->cl_ret = clFinish(gpu_com_struct_ptr->command_queue);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(x_vector_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(abc_para_input_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(para_arr_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(learning_rate_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(input_2d_arr_mem);
    gpu_com_struct_ptr->cl_ret = clReleaseMemObject(output_2d_arr_mem);
    for(i = 0; i < 3; i++)
    {
        gpu_com_struct_ptr->cl_ret = clReleaseMemObject(tmp_vector_output_mem[i]);
    }
    
    return 0;
}

int gpu_primitive_laplacian(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr)
{
    // Set basic options to the parallel computation through array
    int para_arr[L_PROPERTY_NUM] = {0};
    para_arr[PL_ARR_ROW_NUM] = gpu_data_2d_arr_ptr->row_num;
    para_arr[PL_ARR_COL_NUM] = gpu_data_2d_arr_ptr->col_num;
    para_arr[PL_ARR_TOTAL_NUM] = gpu_data_2d_arr_ptr->data_num;
    para_arr[PL_RELATIVE_ROW] = 5;
    para_arr[PL_RELATIVE_COL] = 5;
    para_arr[PL_CENTRAL_DRAG] = 1;

    // Create buffer for each vector on the device
    cl_mem input_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, gpu_data_2d_arr_ptr->data_size, NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem para_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, L_PROPERTY_NUM * sizeof(int), NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem output_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_WRITE_ONLY, gpu_data_2d_arr_ptr->data_size, NULL, &(gpu_com_struct_ptr->cl_ret));
    
    // Copy the input and parameter array into the GPU memory
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, input_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->input_data_arr, 0, NULL, NULL);
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, para_arr_mem, CL_TRUE, 0, L_PROPERTY_NUM * sizeof(int), para_arr, 0, NULL, NULL);

    // Create program and kernel
    int program_id = append_gpu_program(gpu_com_struct_ptr, "primitive_laplacian");

    // Set kernel parameters
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[program_id], 0, sizeof(cl_mem), (void*)&input_arr_mem);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[program_id], 1, sizeof(cl_mem), (void*)&para_arr_mem);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(gpu_com_struct_ptr->kernel_arr[program_id], 2, sizeof(cl_mem), (void*)&output_arr_mem);

    // Execute the kernel
    size_t global_item_size = gpu_data_2d_arr_ptr->data_num;
    size_t local_item_size = 100;
    gpu_com_struct_ptr->cl_ret = clEnqueueNDRangeKernel(gpu_com_struct_ptr->command_queue, gpu_com_struct_ptr->kernel_arr[program_id], 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
    
    // Read RAM buffer output_arr_mem input the local buffer output_data_arr
    gpu_com_struct_ptr->cl_ret = clEnqueueReadBuffer(gpu_com_struct_ptr->command_queue, output_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->output_data_arr, 0, NULL, NULL);

    // Close and flush the CL local structures
    gpu_com_struct_ptr->cl_ret = clFlush(gpu_com_struct_ptr->command_queue);
    gpu_com_struct_ptr->cl_ret = clFinish(gpu_com_struct_ptr->command_queue);
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
    para_arr[PL_ARR_TOTAL_NUM] = gpu_data_2d_arr_ptr->data_num;
    para_arr[PL_RELATIVE_ROW] = 5;
    para_arr[PL_RELATIVE_COL] = 5;
    para_arr[PL_CENTRAL_DRAG] = 1;
    para_arr[WL_DISTANCE_COEFFICIENT] = 2;

    // Create buffer for each vector on the device
    cl_mem input_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, gpu_data_2d_arr_ptr->data_size, NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem para_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, L_PROPERTY_NUM * sizeof(int), NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem output_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_WRITE_ONLY, gpu_data_2d_arr_ptr->data_size, NULL, &(gpu_com_struct_ptr->cl_ret));
    
    // Copy the input and parameter array into the GPU memory
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, input_arr_mem, CL_TRUE, 0, gpu_data_2d_arr_ptr->data_size, gpu_data_2d_arr_ptr->input_data_arr, 0, NULL, NULL);
    gpu_com_struct_ptr->cl_ret = clEnqueueWriteBuffer(gpu_com_struct_ptr->command_queue, para_arr_mem, CL_TRUE, 0, L_PROPERTY_NUM * sizeof(int), para_arr, 0, NULL, NULL);

    // Create program
    cl_program program = clCreateProgramWithSource(gpu_com_struct_ptr->context, 1, (const char**)&(gpu_com_struct_ptr->kernel_source_str), (const size_t*)&(gpu_com_struct_ptr->kernel_source_size), &(gpu_com_struct_ptr->cl_ret));
    // Construct program
    gpu_com_struct_ptr->cl_ret = clBuildProgram(program, 1, &(gpu_com_struct_ptr->device_id), NULL, NULL, NULL);
    // Create OpenCL kernel (ERROR!)
    cl_kernel kernel = clCreateKernel(program, "weighed_laplacian", &(gpu_com_struct_ptr->cl_ret));

    // Set kernel parameters
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&input_arr_mem);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&para_arr_mem);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&output_arr_mem);

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

int gpu_statistic_laplacian(GPU_COM_STRUCT *gpu_com_struct_ptr, GPU_DATA_2D_ARR *gpu_data_2d_arr_ptr)
{
    // Set basic options to the parallel computation through array
    int para_arr[L_PROPERTY_NUM] = {0};
    para_arr[PL_ARR_ROW_NUM] = gpu_data_2d_arr_ptr->row_num;
    para_arr[PL_ARR_COL_NUM] = gpu_data_2d_arr_ptr->col_num;
    para_arr[PL_ARR_TOTAL_NUM] = gpu_data_2d_arr_ptr->data_num;
    para_arr[PL_RELATIVE_ROW] = 5;
    para_arr[PL_RELATIVE_COL] = 5;
    para_arr[PL_CENTRAL_DRAG] = 1;
    para_arr[WL_DISTANCE_COEFFICIENT] = 2;
    para_arr[SL_THRESHOLD] = 3;

    // Create buffer for each vector on the device
    cl_mem input_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, gpu_data_2d_arr_ptr->data_size, NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem para_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_READ_ONLY, L_PROPERTY_NUM * sizeof(int), NULL, &(gpu_com_struct_ptr->cl_ret));
    cl_mem output_arr_mem = clCreateBuffer(gpu_com_struct_ptr->context, CL_MEM_WRITE_ONLY, gpu_data_2d_arr_ptr->data_size, NULL, &(gpu_com_struct_ptr->cl_ret));

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
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&input_arr_mem);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&para_arr_mem);
    gpu_com_struct_ptr->cl_ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&output_arr_mem);

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

int destroy_gpu_program_array(GPU_COM_STRUCT *gpu_com_struct_ptr)
{
    // Clear the array
    unsigned int i;
    for(i = 0; i < gpu_com_struct_ptr->program_num; i++)
    {
        gpu_com_struct_ptr->cl_ret = clReleaseKernel(gpu_com_struct_ptr->kernel_arr[i]);
        gpu_com_struct_ptr->cl_ret = clReleaseProgram(gpu_com_struct_ptr->program_arr[i]);
        free(gpu_com_struct_ptr->func_name[i]);
        gpu_com_struct_ptr->func_name[i] = NULL;
    }
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