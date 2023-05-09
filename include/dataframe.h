#ifndef DATAFRAME_H
#define DATAFRAME_H

typedef struct DataFrame DataFrame;

/* Function to create a new dataframe */
DataFrame* dataframe_create(int num_cols, int num_rows);

/* Function to free memory used by a dataframe */
void dataframe_free(DataFrame* df);

/* Function to get the number of columns in a dataframe */
int dataframe_num_cols(DataFrame* df);

/* Function to get the number of rows in a dataframe */
int dataframe_num_rows(DataFrame* df);

/* Function to get the value at a particular row and column in a dataframe */
int dataframe_get(DataFrame* df, int row, int col);

/* Function to set the value at a particular row and column in a dataframe */
void dataframe_set(DataFrame* df, int row, int col, int value);

#endif
