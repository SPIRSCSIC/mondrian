#include <stdlib.h>
#include "dataframe.h"

/* Definition of the DataFrame struct */
/* Support every datatype that can be interpeted as integer (integer, date,
 categorical...)*/
struct DataFrame {
    int** data;
    int num_cols;
    int num_rows;
};

/* Function to create a new dataframe */
DataFrame* dataframe_create(int num_cols, int num_rows) {
    /* Allocate memory for the dataframe struct */
    DataFrame* df = malloc(sizeof(DataFrame));
    if (df == NULL) {
        return NULL;
    }

    /* Allocate memory for the data array */
    df->data = malloc(num_rows * sizeof(int*));
    if (df->data == NULL) {
        free(df);
        return NULL;
    }
    for (int i = 0; i < num_rows; i++) {
        df->data[i] = malloc(num_cols * sizeof(int));
        if (df->data[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(df->data[j]);
            }
            free(df->data);
            free(df);
            return NULL;
        }
    }

    /* Set the number of columns and rows */
    df->num_cols = num_cols;
    df->num_rows = num_rows;

    return df;
}

/* Function to free memory used by a dataframe */
void dataframe_free(DataFrame* df) {
    for (int i = 0; i < df->num_rows; i++) {
        free(df->data[i]);
    }
    free(df->data);
    free(df);
}

/* Function to get the number of columns in a dataframe */
int dataframe_num_cols(DataFrame* df) {
    return df->num_cols;
}

/* Function to get the number of rows in a dataframe */
int dataframe_num_rows(DataFrame* df) {
    return df->num_rows;
}

/* Function to get the value at a particular row and column in a dataframe */
int dataframe_get(DataFrame* df, int row, int col) {
    return df->data[row][col];
}

/* Function to set the value at a particular row and column in a dataframe */
void dataframe_set(DataFrame* df, int row, int col, int value) {
    df->data[row][col] = value;
}
