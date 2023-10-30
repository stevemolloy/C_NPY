#include <stdio.h>
#include <string.h>

#include "sm_lib.h"

int main(void) {
  char *magic_string = "\x93NUMPY";
  char *filename = "examples/vector.npy";
  
  char *buff_addr;
  char *cursor;
  if (read_file_into_mem(filename, &buff_addr) < 0) {
    fprintf(stderr, "Could not read file: '%s'\n", filename);
    return 1;
  }

  if (strncmp(buff_addr, magic_string, strlen(magic_string)) != 0) {
    fprintf(stderr, "Not a valid NPY file: '%s'\n", filename);
    return 1;
  }

  cursor = buff_addr + strlen(magic_string);
  unsigned int major_version = (unsigned int)*cursor++;
  unsigned int minor_version = (unsigned int)*cursor++;

  printf("Major.minor = %d.%d\n", major_version, minor_version);

  size_t hdr_len = (size_t)*(unsigned short int*)(cursor);
  printf("header length = %zu\n", hdr_len);

  char* data_start = cursor + hdr_len;
  (void)data_start;

  cursor += 2;

  // Now to scan through the python-style dictionary
  if (*cursor++ != '{') {
    fprintf(stderr, "Not a valid NPY file: '%s'\n", filename);
    return 1;
  }

  char *descr_str = "'descr'";
  char *data_type_start = NULL;
  size_t data_type_len = 0;
  // char *fortran_order_str = "'fortran_order'";
  // char *shape_str = "'shape'";
  if (strncmp(cursor, descr_str, strlen(descr_str)) == 0) {
    cursor += strlen(descr_str);
    while (*cursor++ != '\'');
    data_type_start = cursor;
    while (*cursor++ != '\'') {
      data_type_len++;
    }
  }

  char *data_type = calloc(data_type_len + 1, sizeof(char));
  if (data_type == NULL) {
    fprintf(stderr, "Couldn't allocate memory.\n");
    return 1;
  }
  memcpy(data_type, data_type_start, data_type_len);
  
  printf("data_type = %s\n", data_type);

  free(data_type);
  free(buff_addr);

  return 0;
}
