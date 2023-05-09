#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

int compare(const void *a, const void *b) {
    int f_a = *((int*)a);
    int f_b = *((int*)b);

    return f_a - f_b;
}

void anonymized_partition(int **data, int size, int num_attributes) {
  int* min_vals = malloc(sizeof(int*) * num_attributes);
  int* max_vals = malloc(sizeof(int*) * num_attributes);
  for(int i = 0; i < num_attributes; i++) {
    min_vals[i] = INT_MAX;
    max_vals[i] = INT_MIN;
  }
  for(int i = 0; i < num_attributes; i++){
    for(int j = 0; j < size; j++) {
      if (data[j][i] < min_vals[i]) min_vals[i] = data[j][i];
      if (data[j][i] > max_vals[i]) max_vals[i] = data[j][i];
    }
  }
  for(int i = 0; i < size; i++) {
    for(int j = 0; j < num_attributes; j++) {
      if(min_vals[j] == max_vals[j]){
        printf("%d ", data[i][j]);
      }
      else {
        printf("[%d, %d] ", min_vals[j], max_vals[j]);
      }
    }
    printf("\n");
  }
}

void find_largest_range(int **data, int size, int num_attributes, int *attribute_idx, int *range) {
    int max_range = INT_MIN;
    *attribute_idx = -1;

    for (int attr = 0; attr < num_attributes; attr++) {
        int min = INT_MAX;
        int max = INT_MIN;

        for (int i = 0; i < size; i++) {
            if (data[i][attr] < min) min = data[i][attr];
            if (data[i][attr] > max) max = data[i][attr];
        }

        int current_range = max - min;
        if (current_range > max_range) {
            max_range = current_range;
            *attribute_idx = attr;
        }
    }

    *range = max_range;
}

void mondrian(int **data, int num_attributes, int k, int size) {
    if (size < 2*k) {
        anonymized_partition(data, size, num_attributes);
        return;
    }

    int attribute_idx;
    int range;
    find_largest_range(data, size, num_attributes, &attribute_idx, &range);

    if (attribute_idx == -1) {
        return;
    }

    int allowable_cut = 0;
    int iterator = 0;
    int l_size;
    int r_size = 0;
    int median;
    while(!allowable_cut) {
      int *values = (int *)malloc(size * sizeof(int));
      for (int i = 0; i < size; i++) {
        values[i] = data[i][(attribute_idx + iterator) % num_attributes];
      }

      qsort(values, size, sizeof(int), compare);
      median = values[size / 2];
      // printf("Cut by: %d, %d\n", (attribute_idx + iterator) % num_attributes, median);
      free(values);

      r_size = 0;

      for (int i = 0; i < size; i++) {
        if(data[i][(attribute_idx + iterator) % num_attributes] >= median) {
          r_size += 1;
        }
      }
      l_size = size - r_size;
      if(r_size == size || l_size == size || r_size < k || l_size < k) {
        iterator += 1;
        if(iterator == num_attributes) {
          anonymized_partition(data, size, num_attributes);
          return;
        }
        continue;
      }
      allowable_cut = 1;
      attribute_idx = (attribute_idx + iterator) % num_attributes;
    }
    int** r_part = (int**)malloc(sizeof(int*)*r_size);
    int** l_part = (int**)malloc(sizeof(int*)*l_size);
    int r_count = 0;
    int l_count = 0;
    for (int i = 0; i < size; i++) {
      if(data[i][attribute_idx] >= median) {
        r_part[r_count] = data[i];
        r_count++;
      }
      else {
        l_part[l_count] = data[i];
        l_count++;
      }
    }

    mondrian(r_part, num_attributes, k, r_size);
    mondrian(l_part, num_attributes, k, l_size);
}

int main() {
    int num_records = 10;
    int num_attributes = 3;
    int k = 2;

    int **data = (int**)malloc(num_records * sizeof(int*));
    for (int i = 0; i < num_records; i++) {
        data[i] = (int*)malloc(num_attributes * sizeof(int));
    }

    // Example dataset
    data[0][0] = 34; data[0][1] = 12; data[0][2] = 1;
    data[1][0] = 20; data[1][1] = 9;  data[1][2] = 5;
    data[2][0] = 25; data[2][1] = 7;  data[2][2] = 3;
    data[3][0] = 50; data[3][1] = 15; data[3][2] = 6;
    data[4][0] = 55; data[4][1] = 20; data[4][2] = 7;
    data[5][0] = 40; data[5][1] = 12; data[5][2] = 2;
    data[6][0] = 30; data[6][1] = 10; data[6][2] = 4;
    data[7][0] = 45; data[7][1] = 18; data[7][2] = 6;
    data[8][0] = 60; data[8][1] = 22; data[8][2] = 8;
    data[9][0] = 15; data[9][1] = 5;  data[9][2] = 3;

    mondrian(data, num_attributes, k, num_records);

    // Free the memory allocated for the data array
    for (int i = 0; i < num_records; i++) {
        free(data[i]);
    }
    free(data);

    return 0;
}
