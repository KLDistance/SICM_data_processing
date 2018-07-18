#include "csv_ops.h"

int get_csv_arr_size(char *data_str, CSV_STRUCT *csv_struct_ptr)
{
    unsigned int dataStrLen = strlen(data_str);
    char *tmp = data_str;
    unsigned int i;

    // Detect the column number
    for(i = 0; i < dataStrLen; i++)
    {
        if(*(tmp++) == ',')
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
        if(*(tmp++) == '\n')
        {
            csv_struct_ptr->row_num++;
        }
        else if(*tmp == 0)
        {
            break;
        }
    }
    return 0;
}

int from_file_float(char *file_str, CSV_STRUCT *csv_struct_ptr)
{
    char *tmp_front = file_str;
    char *tmp_back = file_str;
    char tmp_buf[BUF_LEN] = {0};
    unsigned int i, j;
    unsigned int tmp_len = 0;

    // Assign the space for csv_struct_ptr data array
    float *data_arr = (float*)malloc(csv_struct_ptr->row_num * csv_struct_ptr->col_num * sizeof(float));
    if(!data_arr)
    {
        fprintf(stderr, "Unable to assign the space for data array!\n");
        return -1;
    }

    for(i = 0; i < csv_struct_ptr->row_num; i++)
    {
        for(j = 0; j < csv_struct_ptr->row_num; j++)
        {
            memset(tmp_buf, 0, BUF_LEN);
            for(tmp_back = tmp_front; *tmp_front != 0; tmp_front++)
            {
                if(*tmp_front = ',' || *tmp_front == '\n')
                {
                    tmp_len = (unsigned int)(tmp_front++ - tmp_back);
                    strncpy(tmp_buf, tmp_back, tmp_len);
                    data_arr[csv_struct_ptr->col_num * i + j] = atof(tmp_buf);
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
    char *tmp_front = file_str;
    char *tmp_back = file_str;
    char tmp_buf[BUF_LEN] = {0};
    unsigned int i, j;
    unsigned int tmp_len = 0;

    // Assign the space for csv_struct_ptr data array
    int *data_arr = (int*)malloc(csv_struct_ptr->row_num * csv_struct_ptr->col_num * sizeof(int));
    if(!data_arr)
    {
        fprintf(stderr, "Unable to assign the space for data array!\n");
        return -1;
    }

    for(i = 0; i < csv_struct_ptr->row_num; i++)
    {
        for(j = 0; j < csv_struct_ptr->row_num; j++)
        {
            memset(tmp_buf, 0, BUF_LEN);
            for(tmp_back = tmp_front; *tmp_front != 0; tmp_front++)
            {
                if(*tmp_front = ',' || *tmp_front == '\n')
                {
                    tmp_len = (unsigned int)(tmp_front++ - tmp_back);
                    strncpy(tmp_buf, tmp_back, tmp_len);
                    data_arr[csv_struct_ptr->col_num * i + j] = atof(tmp_buf);
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
    char *tmp_front = file_str;
    char *tmp_back = file_str;
    char tmp_buf[BUF_LEN] = {0};
    unsigned int i, j;
    unsigned int tmp_len = 0;

    // Assign the space for csv_struct_ptr data array
    short *data_arr = (short*)malloc(csv_struct_ptr->row_num * csv_struct_ptr->col_num * sizeof(short));
    if(!data_arr)
    {
        fprintf(stderr, "Unable to assign the space for data array!\n");
        return -1;
    }

    for(i = 0; i < csv_struct_ptr->row_num; i++)
    {
        for(j = 0; j < csv_struct_ptr->row_num; j++)
        {
            memset(tmp_buf, 0, BUF_LEN);
            for(tmp_back = tmp_front; *tmp_front != 0; tmp_front++)
            {
                if(*tmp_front = ',' || *tmp_front == '\n')
                {
                    tmp_len = (unsigned int)(tmp_front++ - tmp_back);
                    strncpy(tmp_buf, tmp_back, tmp_len);
                    data_arr[csv_struct_ptr->col_num * i + j] = atof(tmp_buf);
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

extern int csv_reader(CSV_STRUCT *csv_struct_ptr)
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

static int csv_conv_thread_init(THREAD_PARAMETER *lpParameter, unsigned int thread_id, unsigned int *common_arr, char *txt_area)
{
    lpParameter->common_arr = common_arr;
    lpParameter->def_thread_id = thread_id;
    lpParameter->txt_area = txt_area;
    return 0;
}

void* single_file_slice_proc(void *arg)
{

}

extern int csv_writer(CSV_STRUCT *csv_struct_ptr)
{
    // File name process in case of name duplicates
    char new_file_name[CSV_PATH_LEN] = {0};
    sprintf(new_file_name, "proc_%s", csv_struct_ptr->csv_path);

    // Open empty file
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
        unsigned int gap_index[5] = {0};
        unsigned int data_arr_num = csv_struct_ptr->row_num * csv_struct_ptr->col_num;
        unsigned int single_space = 8 * data_arr_num;
        pthread_t csv_conv_thread_arr[4];
        CSV_OPS_MULTITHREAD_PARAMETER thread_parameter_arr[4];
        
        // Set terminal array position
        gap_index[4] = data_arr_num;

        for(i = 0; i < 4; i++)
        {
            // Allocate space for the 4 threads processing char arrays
            txt_area_sets[i] = (char*)malloc(single_space);
            
            // Calculate the gap indexes for the 4 threads in the data array
            gap_index[i] = (unsigned int)(csv_struct_ptr->row_num / 4) * i * csv_struct_ptr->col_num;
        }

        // Initiate the thread parameter
        for(i = 0; i < 4; i++)
        {
            csv_conv_thread_init(&thread_parameter_arr[i], i, gap_index, txt_area_sets[i]);
        }

        // Execute the threads
        for(i = 0; i < 4; i++)
        {
            pthread_create(&csv_conv_thread_arr[i], NULL, single_file_slice_proc, (void*)&(thread_parameter_arr[i]));
        }

        usleep(100000);

        // Wait for the joints of the subthreads
        for(i = 0; i < 4; i++)
        {
            pthread_join(csv_conv_thread_arr[i], NULL);
        }

        // Write contents into file
        for(i = 0; i < 4; i++)
        {
            fseek(fp_new_csv, gap_index[i], SEEK_SET);
            fwrite(txt_area_sets[i], gap_index[i + 1] - gap_index[i], 1, fp_new_csv);
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

    }

    return 0;
}