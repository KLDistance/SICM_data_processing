#include <stdio.h>
#include "parallel_processing.h"
#include "csv_ops.h"

int smooth_function(char *file_path, char *proc_type)
{
    GPU_COM_STRUCT gpu_com_struct;
    GPU_DATA_2D_ARR gpu_data_2d_arr;
    CSV_STRUCT csv_struct;

    // Read from csv file
    init_csv_struct(&csv_struct, file_path, 0);
    csv_reader(&csv_struct);

    CURVE_PROPERTY curve_property;

    // Get parameter and feedback from users
    do
    {
        printf("Use default or custom parameters to process the surface? < d(default) / c(custom) / e(exit) > ");
        char input_key = 0;
        scanf("%c", &input_key);

        if(input_key == 'd')
        {
            curve_property.a_learning_rate = 1E-5;
            curve_property.b_learning_rate = 1E-5;
            curve_property.c_learning_rate = 1E-5;
            curve_property.init_a = -2.0f;
            curve_property.init_b = 10.0f;
            curve_property.init_c = 100.0f;
            curve_property.iteration_num = 20000;

            break;
        }
        else if(input_key == 'c')
        {
            float tmp_var_a = 0;
            float tmp_var_b = 0;
            float tmp_var_c = 0;
            unsigned int tmp_var = 0;

            printf("Input the initial a b c: ");
            scanf("%f %f %f", &tmp_var_a, &tmp_var_b, &tmp_var_c);
            curve_property.init_a = tmp_var_a;
            curve_property.init_b = tmp_var_b;
            curve_property.init_c = tmp_var_c;

            printf("Input the a b c learning rate: ");
            scanf("%f %f %f", &tmp_var_a, &tmp_var_b, &tmp_var_c);
            curve_property.a_learning_rate = tmp_var_a;
            curve_property.b_learning_rate = tmp_var_b;
            curve_property.c_learning_rate = tmp_var_c;

            printf("Input the max iteration number for surface bending operation (1 ~ 5000000): ");
            scanf("%u", &tmp_var);
            tmp_var = (tmp_var > 5000000) ? 5000000 : tmp_var;
            curve_property.iteration_num = (tmp_var >= 1) ? tmp_var : 1;

            break;
        }
        else if(input_key == 'e')
        {
            exit(1);
        }
        else
        {
            printf("Invalid input, please try again.\n");
            fflush(stdin);
            scanf("%c", &input_key);
        }
    } while(1);
    printf("Data smoothing process starts...\n");

    // Initiate 2d array for GPU calculation
    init_gpu_2d_arr(&gpu_data_2d_arr, csv_struct.row_num, csv_struct.col_num, (float*)csv_struct.data_arr);
    init_gpu_state(&gpu_com_struct);

    // Slope diminish
    gpu_slope_diminish(&gpu_com_struct, &gpu_data_2d_arr);

    // Surface bending
    gpu_1d_curve_bending(&gpu_com_struct, &gpu_data_2d_arr, &curve_property);
    
    // Calculation
    //gpu_primitive_laplacian(&gpu_com_struct, &gpu_data_2d_arr);

    // Write data array into a new csv file
    csv_struct.result_data_arr = gpu_data_2d_arr.input_data_arr;
    csv_writer(&csv_struct);

    destroy_gpu_2d_arr(&gpu_data_2d_arr);
    destroy_gpu_program_array(&gpu_com_struct);
    destroy_gpu_state(&gpu_com_struct);

    printf("Data smoothing process done.\n");

    return 0;
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "-usage:%s [original .csv file]\n", argv[0]);
        exit(1);
    }
    //check_gpu_info();
    smooth_function(argv[1], NULL);
    return 0;
}