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