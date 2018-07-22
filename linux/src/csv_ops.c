#include "csv_ops.h"

int get_csv_arr_size(char *data_str, CSV_STRUCT *csv_struct_ptr)
{
    unsigned int dataStrLen = strlen(data_str);
    char *tmp = data_str;
    unsigned int i;

    // Detect the column number
    for(i = 0; i < dataStrLen; i++)
    {
        tmp++;
        if(*tmp == ',')
        {
            csv_struct_ptr->col_num++;
        }
        else if (*tmp == '\n')
        {
            csv_struct_ptr->col_num++;
            break;
        }
    }
    
    // Detect the row number
    for(i = 0; i < dataStrLen; i++)
    {
        if(*tmp == '\n')
        {
            csv_struct_ptr->row_num++;
        }
        else if(*tmp == 0)
        {
            break;
        }
        tmp++;
    }
    printf(".csv contains %u rows %u columns of data.\n", csv_struct_ptr->row_num, csv_struct_ptr->col_num);
    return 0;
}

int from_file_float(char *file_str, CSV_STRUCT *csv_struct_ptr)
{
    char *tmp_front = file_str + 3;
    char *tmp_back = file_str + 3; // The header of csv is EF BB BF, which should be jumped over!
    char tmp_buf[BUF_LEN] = {0};
    unsigned int i, j;
    unsigned int tmp_len = 0;
    unsigned int tmp_product = 0;

    // Assign the space for csv_struct_ptr data array
    float *data_arr = (float*)malloc(csv_struct_ptr->row_num * csv_struct_ptr->col_num * sizeof(float));
    if(!data_arr)
    {
        fprintf(stderr, "Unable to assign the space for data array!\n");
        return -1;
    }

    for(i = 0; i < csv_struct_ptr->row_num; i++)
    {
        tmp_product = csv_struct_ptr->col_num * i;
        for(j = 0; j < csv_struct_ptr->col_num; j++)
        {
            memset(tmp_buf, 0, BUF_LEN);
            for(tmp_back = tmp_front; *tmp_front != '\0'; tmp_front++)
            {
                if(*tmp_front == ',' || *tmp_front == '\n' || *tmp_front == 0)
                {
                    tmp_len = (unsigned int)(tmp_front++ - tmp_back);
                    strncpy(tmp_buf, tmp_back, tmp_len);
                    data_arr[tmp_product + j] = atof(tmp_buf);
                    tmp_back = tmp_front;
                    break;
                }
            }
        }
    }

    // Let struct ptr points to the data array
    csv_struct_ptr->data_arr = data_arr;

    return 0;
}

int from_file_int(char *file_str, CSV_STRUCT *csv_struct_ptr)
{
    char *tmp_front = file_str + 3;
    char *tmp_back = file_str + 3;
    char tmp_buf[BUF_LEN] = {0};
    unsigned int i, j;
    unsigned int tmp_len = 0;
    unsigned int tmp_product = 0;

    // Assign the space for csv_struct_ptr data array
    int *data_arr = (int*)malloc(csv_struct_ptr->row_num * csv_struct_ptr->col_num * sizeof(int));
    if(!data_arr)
    {
        fprintf(stderr, "Unable to assign the space for data array!\n");
        return -1;
    }

    for(i = 0; i < csv_struct_ptr->row_num; i++)
    {
        tmp_product = csv_struct_ptr->col_num * i;
        for(j = 0; j < csv_struct_ptr->col_num; j++)
        {
            memset(tmp_buf, 0, BUF_LEN);
            for(tmp_back = tmp_front; *tmp_front != 0; tmp_front++)
            {
                if(*tmp_front == ',' || *tmp_front == '\n' || *tmp_front == 0)
                {
                    tmp_len = (unsigned int)(tmp_front++ - tmp_back);
                    strncpy(tmp_buf, tmp_back, tmp_len);
                    data_arr[tmp_product + j] = atof(tmp_buf);
                    tmp_back = tmp_front;
                    break;
                }
            }
        }
    }

    // Let struct ptr points to the data array
    csv_struct_ptr->data_arr = data_arr;
    
    return 0;
}

int from_file_short(char *file_str, CSV_STRUCT *csv_struct_ptr)
{
    char *tmp_front = file_str + 3;
    char *tmp_back = file_str + 3;
    char tmp_buf[BUF_LEN] = {0};
    unsigned int i, j;
    unsigned int tmp_len = 0;
    unsigned int tmp_product = 0;

    // Allocate space for csv_struct_ptr data array
    short *data_arr = (short*)malloc(csv_struct_ptr->row_num * csv_struct_ptr->col_num * sizeof(short));
    if(!data_arr)
    {
        fprintf(stderr, "Unable to assign the space for data array!\n");
        return -1;
    }

    for(i = 0; i < csv_struct_ptr->row_num; i++)
    {
        tmp_product = csv_struct_ptr->col_num * i;
        for(j = 0; j < csv_struct_ptr->col_num; j++)
        {
            memset(tmp_buf, 0, BUF_LEN);
            for(tmp_back = tmp_front; *tmp_front != 0; tmp_front++)
            {
                if(*tmp_front == ',' || *tmp_front == '\n' || *tmp_front == 0)
                {
                    tmp_len = (unsigned int)(tmp_front++ - tmp_back);
                    strncpy(tmp_buf, tmp_back, tmp_len);
                    data_arr[tmp_product + j] = atof(tmp_buf);
                    tmp_back = tmp_front;
                    break;
                }
            }
        }
    }

    // Let struct ptr points to the data array
    csv_struct_ptr->data_arr = data_arr;
    
    return 0;
}

int init_csv_struct(CSV_STRUCT *csv_struct_ptr, char *file_path, char csv_data_type)
{
    unsigned int file_path_len = strlen(file_path);
    csv_struct_ptr->csv_data_type = csv_data_type;
    memset(csv_struct_ptr->csv_path, 0, CSV_PATH_LEN);
    strncpy(csv_struct_ptr->csv_path, file_path, file_path_len < CSV_PATH_LEN ? file_path_len : CSV_PATH_LEN);
    csv_struct_ptr->row_num = 0;
    csv_struct_ptr->col_num = 0;
    return 0;
}

int csv_reader(CSV_STRUCT *csv_struct_ptr)
{
    FILE *pFile = NULL;
    unsigned int file_size = 0;
    void *file_str = NULL;
    pFile = fopen(csv_struct_ptr->csv_path, "r");
    if(!pFile)
    {
        fprintf(stderr, "Unable to open file %s!\n", csv_struct_ptr->csv_path);
        exit(1);
    }
    fseek(pFile, 0, SEEK_END);
    file_size = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    file_str = malloc(file_size);
    if(!file_str)
    {
        fprintf(stderr, "Unable to assign the space!\n");
        fclose(pFile);
        exit(1);
    }
    memset(file_str, 0, file_size);
    fread(file_str, file_size, 1, pFile);
    fclose(pFile);

    // Get row and column number info
    get_csv_arr_size(file_str, csv_struct_ptr);

    switch(csv_struct_ptr->csv_data_type)
    {
        case CSV_TYPE_FLOAT:
            from_file_float(file_str, csv_struct_ptr);
            break;
        case CSV_TYPE_INT:
            from_file_int(file_str, csv_struct_ptr);
            break;
        case CSV_TYPE_SHORT:
            from_file_short(file_str, csv_struct_ptr);
            break;
        default:
            printf("Unresolved data type.\n");
            return -1;
    }
    return 0;
}

static int csv_conv_thread_init(CSV_OPS_MULTITHREAD_PARAMETER *lpParameter, unsigned int thread_id, unsigned int *common_arr, char *txt_area, unsigned int *strlen_indicator, CSV_STRUCT *csv_struct)
{
    lpParameter->common_arr = common_arr;
    lpParameter->def_thread_id = thread_id;
    lpParameter->txt_area = txt_area;
    lpParameter->string_length_indicator = strlen_indicator;
    lpParameter->arr_row_num = csv_struct->row_num;
    lpParameter->arr_col_num = csv_struct->col_num;
    lpParameter->data_arr = csv_struct->result_data_arr;
    return 0;
}

void* single_file_slice_proc(void *arg)
{
    CSV_OPS_MULTITHREAD_PARAMETER *thread_parameter_ptr = (CSV_OPS_MULTITHREAD_PARAMETER*)arg;
    char *tmp_ptr = thread_parameter_ptr->txt_area;
    unsigned int i, j;
    unsigned int tmp_product = 0;
    unsigned int iter_length_indicator = 0;
    for(i = thread_parameter_ptr->common_arr[thread_parameter_ptr->def_thread_id]; i < thread_parameter_ptr->common_arr[thread_parameter_ptr->def_thread_id + 1]; i++)
    {
        tmp_product = i * thread_parameter_ptr->arr_col_num;
        for(j = 0; j < thread_parameter_ptr->arr_col_num; j++)
        {
            if(j < thread_parameter_ptr->arr_col_num - 1)
            {
                sprintf(tmp_ptr, "%.8f,", thread_parameter_ptr->data_arr[tmp_product + j]);
                iter_length_indicator = strlen(tmp_ptr);
                thread_parameter_ptr->string_length_indicator[thread_parameter_ptr->def_thread_id] += iter_length_indicator;
                tmp_ptr += iter_length_indicator;
            }
            else
            {
                sprintf(tmp_ptr, "%.8f\n", thread_parameter_ptr->data_arr[tmp_product + j]);
                iter_length_indicator = strlen(tmp_ptr);
                thread_parameter_ptr->string_length_indicator[thread_parameter_ptr->def_thread_id] += iter_length_indicator;
                tmp_ptr += iter_length_indicator;
            }
        }
    }
    printf("Writer thread %u (%u -- %u) ends.\n", thread_parameter_ptr->def_thread_id, thread_parameter_ptr->common_arr[thread_parameter_ptr->def_thread_id], i);
    return (void*)0;
}

int csv_writer(CSV_STRUCT *csv_struct_ptr)
{
    // File name process in case of name duplicates
    char new_file_name[CSV_PATH_LEN + 16] = {0};
    memset(new_file_name, 0, CSV_PATH_LEN + 16);

    // Find the "." before the type
    sprintf(new_file_name, "%s", csv_struct_ptr->csv_path);
    memset(new_file_name + strlen(csv_struct_ptr->csv_path) - 4, 0, 4);
    strncat(new_file_name, "_proc.csv", strlen("_proc.csv"));

    // Open empty file
    char csv_header[3] = {0xEF, 0xBB, 0xBF};
    FILE *fp_new_csv = fopen(new_file_name, "w");
    if(!fp_new_csv)
    {
        fprintf(stderr, "Unable to create a new empty csv file for data array!\n");
        exit(1);
    }

    if(csv_struct_ptr->row_num >= 4)
    {
        // Variables
        unsigned int i;
        char *txt_area_sets[4] = {(char*)0};

        // Gap index is specific to the row index, col index not included!
        unsigned int gap_index[5] = {0};
        unsigned int data_arr_num = csv_struct_ptr->row_num * csv_struct_ptr->col_num;
        unsigned int single_space = 8 * data_arr_num;
        unsigned int string_length_indicator[4] = {0};
        pthread_t csv_conv_thread_arr[4];
        CSV_OPS_MULTITHREAD_PARAMETER thread_parameter_arr[4];
        
        // Set terminal array position
        gap_index[4] = csv_struct_ptr->row_num;

        for(i = 0; i < 4; i++)
        {
            if(i == 3)
            {
                // Last set may contain more chars that the former 3 ones.
                txt_area_sets[i] = (char*)malloc(single_space * 2);
                memset(txt_area_sets[i], 0, single_space * 2);
            }
            else
            {
                // Allocate space for the 4 threads processing char arrays
                txt_area_sets[i] = (char*)malloc(single_space);
                memset(txt_area_sets[i], 0, single_space);
            }
            
            // Calculate the gap indexes for the 4 threads in the data array
            gap_index[i] = (unsigned int)(csv_struct_ptr->row_num / 4) * i;
        }
        
        // Initiate the thread parameter
        for(i = 0; i < 4; i++)
        {
            csv_conv_thread_init(&thread_parameter_arr[i], i, gap_index, txt_area_sets[i], string_length_indicator, csv_struct_ptr);
        }

        // Execute the threads
        for(i = 0; i < 4; i++)
        {
            pthread_create(&csv_conv_thread_arr[i], NULL, single_file_slice_proc, (void*)&(thread_parameter_arr[i]));
        }

        usleep(500000);

        // Wait for the joints of the subthreads
        for(i = 0; i < 4; i++)
        {
            pthread_join(csv_conv_thread_arr[i], NULL);
        }

        // Write contents into file
        fwrite(csv_header, 3, 1, fp_new_csv);
        fseek(fp_new_csv, 3, SEEK_SET);
        for(i = 0; i < 4; i++)
        {
            fwrite(txt_area_sets[i], string_length_indicator[i], 1, fp_new_csv);
            fseek(fp_new_csv, string_length_indicator[i], SEEK_CUR);
        }
        fseek(fp_new_csv, 0, SEEK_SET);

        // Release the space
        for(i = 0; i < 4; i++)
        {
            free(txt_area_sets[i]);
        }
    }
    else
    {
        // Variables for iteration on a single processing of a file.
        unsigned int i, j;
        unsigned int data_arr_num = csv_struct_ptr->row_num * csv_struct_ptr->col_num;
        unsigned int string_space = data_arr_num * 32;
        unsigned int string_length_indicator = 0;
        char *txt_area = (char*)malloc(string_space);
        
        if(!txt_area)
        {
            fprintf(stderr, "Unable to allocate space for the data to string buffer!\n");
            exit(1);
        }
        char *tmp_ptr = txt_area;

        // Clear the space
        memset(txt_area, 0, string_space);

        // Write into the memory buffer
        unsigned int iter_length_indicator = 0;
        for(i = 0; i < csv_struct_ptr->row_num; i++)
        {
            unsigned int tmp_product = i * csv_struct_ptr->col_num;
            for(j = 0; j < csv_struct_ptr->col_num; j++)
            {
                if(j < csv_struct_ptr->col_num - 1)
                {
                    sprintf(tmp_ptr, "%.8f,", csv_struct_ptr->result_data_arr[tmp_product + j]);
                    iter_length_indicator = strlen(tmp_ptr);
                    string_length_indicator += iter_length_indicator;
                    tmp_ptr += iter_length_indicator;
                }
                else
                {
                    sprintf(tmp_ptr, "%.8f\n", csv_struct_ptr->result_data_arr[tmp_product + j]);
                    iter_length_indicator = strlen(tmp_ptr);
                    string_length_indicator += iter_length_indicator;
                    tmp_ptr += iter_length_indicator;
                }
            }
        }

        // Push the content in the buffer into the file
        fwrite(csv_header, 3, 1, fp_new_csv);
        fseek(fp_new_csv, 3, SEEK_SET);
        fwrite(txt_area, string_length_indicator, 1, fp_new_csv);
        fseek(fp_new_csv, 0, SEEK_SET);
        free(txt_area);
    }

    fclose(fp_new_csv);

    return 0;
}