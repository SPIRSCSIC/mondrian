#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "dataframe.h"

/* Function to calculate the range of a column in a dataframe */
void dataframe_column_range(DataFrame* df, int col, int* min, int* max) {
    *min = *(int*)dataframe_get(df, 0, col);
    *max = *(int*)dataframe_get(df, 0, col);
    for (int i = 1; i < dataframe_num_rows(df); i++) {
        int val = *(int*)dataframe_get(df, i, col);
        if (val < *min) {
            *min = val;
        }
        if (val > *max) {
            *max = val;
        }
    }
}

/* Function to calculate the width of a partition */
int partition_width(DataFrame* df, int col, int partition_start, int partition_end) {
    int partition_min, partition_max;
    dataframe_column_range(df, col, &partition_min, &partition_max);
    for (int i = partition_start; i < partition_end; i++) {
        int val = *(int*)dataframe_get(df, i, col);
        if (val < partition_min) {
            partition_min = val;
        }
        if (val > partition_max) {
            partition_max = val;
        }
    }
    return partition_max - partition_min;
}

/* Function to find the column with the largest partition */
int find_largest_partition(DataFrame* df, int* partition_start, int* partition_end) {
    int largest_partition_width = -1;
    int largest_partition_col = -1;
    for (int col = 0; col < dataframe_num_cols(df); col++) {
        for (int i = 0; i < dataframe_num_rows(df); i++) {
            int j = i + 1;
            while (j < dataframe_num_rows(df) && *(int*)dataframe_get(df, j, col) == *(int*)dataframe_get(df, i, col)) {
                j++;
            }
            int partition_width = partition_width(df, col, i, j);
            if (partition_width > largest_partition_width) {
                largest_partition_width = partition_width;
                largest_partition_col = col;
                *partition_start = i;
                *partition_end = j;
            }
            i = j - 1;
        }
    }
    return largest_partition_col;
}

/* Function to check if a dataframe satisfies k-anonymity */
bool is_k_anonymous(DataFrame* df, int k) {
    for (int i = 0; i < dataframe_num_rows(df); i++) {
        int count = 0;
        for (int j = 0; j < dataframe_num_rows(df); j++) {
            bool match = true;
            for (int col = 0; col < dataframe_num_cols(df); col++) {
                if (*(int*)dataframe_get(df, i, col) != *(int*)dataframe_get(df, j, col)) {
                    match = false;
                    break;
                }
            }
            if (match) {
                count++;
            }
        }
        if (count < k) {
            return false;
        }
    }
    return true;
}

/* Function to apply the Mondrian algorithm to a dataframe to achieve k-anonymity */
void mondrian(DataFrame* df, int k) {
    while (!is_k_anonymous(df, k)) {
        int partition_start, partition_end;
        int col = find_largest_partition(df, &partition_start, &partition_end);
        if (col == -1) {
            fprintf(stderr, "Error: could not find largest partition\n");
            return;
        }
        int num_rows = dataframe_num_rows(df);
        int* indices = malloc(num_rows * sizeof(int));
        for (int i = 0; i < num_rows; i++) {
            indices[i] = i;
        }
        for (int i = 0; i < num_rows; i++) {
            int j = i + 1;
            while (j < num_rows && *(int*)dataframe_get(df, indices[j], col) == *(int*)dataframe_get(df, indices[i], col)) {
                j++;
            }
            int partition_width = partition_width(df, col, i, j);
            if (partition_width > 0) {
                int partition_size = j - i;
                if (partition_size > k) {
                    int split_index = i + (partition_size / 2);
                    int split_val = *(int*)dataframe_get(df, indices[split_index], col);
                    while (split_index < j && *(int*)dataframe_get(df, indices[split_index], col) == split_val) {
                        split_index++;
                    }
                    if (split_index < j) {
                        partition_end = split_index;
                    } else {
                        partition_end = j - 1;
                    }
                } else {
                    partition_end = j - 1;
                }
                for (int k = i; k <= partition_end; k++) {
                    indices[k] |= (1 << col);
                }
                i = partition_end;
            }
        }
    }
}
