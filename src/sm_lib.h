#ifndef _SM_LIB_H
#define _SM_LIB_H

#include <stdlib.h>

typedef struct {
  size_t capacity;
  size_t length;
  float *data;
} SM_float_array;

size_t load_two_column_csv(char *fname, SM_float_array *xs, SM_float_array *ys, size_t ignore_lines);
int read_file_into_mem(char *fname, char **buff_addr);
SM_float_array SM_new_float_array(size_t capacity);
int SM_count_file_lines(char *fname);

#define MAXLINELENGTH 100

#define SM_add_to_array(arr, val)                                                      \
  do {                                                                                 \
       if ((arr)->length >= (arr)->capacity) {                                         \
         (arr)->capacity *= 2;                                                         \
         (arr)->data = realloc((arr)->data, (arr)->capacity * sizeof((arr)->data[0])); \
         if ((arr)->data == NULL) {                                                    \
            fprintf(stderr, "Could not allocate memory for a new SM_float_array.");    \
            exit(1);                                                                   \
         }                                                                             \
       }                                                                               \
       (arr)->data[(arr)->length++] = val;                                             \
  } while (0)

#define SM_free(arr) free((arr).data)

#endif // !_SM_LIB_H

