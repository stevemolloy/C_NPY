#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sm_lib.h"

typedef struct {
  size_t *eles;
  size_t dims;
} PythonTuple;

typedef struct {
  char *data_type;
  bool fortran_order;
  PythonTuple shape;
} DescrDict;

void move_cursor_to_next_key(char **cursor) {
  (*cursor)++;
  while (**cursor!='\'') (*cursor)++;
}

char* get_python_string(char **pystr) {
  assert(*pystr[0]=='\'');

  char *start = *pystr + 1;
  char *cursor = *pystr + 1;
  size_t len = 0;
  while (*cursor != '\'') {
    len++;
    cursor++;
  }

  char *result = calloc(len+1, sizeof(char));
  memcpy(result, start, len);

  *pystr = cursor + 1;

  return result;
}

bool get_python_bool(char **pystr) {
  if (strncmp(*pystr, "True", 4) == 0) {
    *pystr += 4;
    return true;
  } else if (strncmp(*pystr, "False", 5) == 0) {
    *pystr += 5;
    return false;
  }
  assert(0 && "Unreachable");
}

PythonTuple get_python_tuple(char **pystr) {
  PythonTuple result = {0};
  char *cursor = *pystr;
  char *tuple_start;
  char *tuple_end;

  while (*cursor != '(') cursor++;
  tuple_start = cursor + 1;

  while (*cursor != ')') cursor++;
  tuple_end = cursor;

  cursor = tuple_start;
  result.dims = 1;
  while (cursor != tuple_end) {
    if (*cursor == ',') result.dims++;
    cursor++;
  }

  result.eles = calloc(result.dims, sizeof(size_t));

  char *tuplestr = calloc((size_t)(tuple_end - tuple_start) + 1, sizeof(char));
  memcpy(tuplestr, tuple_start, (size_t)(tuple_end - tuple_start));

  size_t dim = 0;
  char *tok = strtok(tuplestr, ",");
  while (tok != NULL) {
    result.eles[dim] = (size_t)atoi(tok);
    dim++;
    tok = strtok(NULL, ",");
  }

  while (result.eles[result.dims-1] == 0) {
    result.dims--;
    result.eles = realloc(result.eles, result.dims * sizeof(size_t));
  }

  free(tuplestr);

  return result;
}

DescrDict parse_dict(char *dictstr) {
  DescrDict result = {0};

  char* new_key;

  char *cursor = dictstr;
  while (*cursor!='\'') cursor++;

  for (size_t i=0; i<3; i++) {
    new_key = get_python_string(&cursor);

    while (*cursor==':' || *cursor==' ') cursor++;

    if (strcmp(new_key, "descr") == 0) {
      result.data_type = get_python_string(&cursor);
    } else if (strcmp(new_key, "fortran_order") == 0) {
      result.fortran_order = get_python_bool(&cursor);
    } else if (strcmp(new_key, "shape") == 0) {
      result.shape = get_python_tuple(&cursor);
    } else {
      assert(0 && "unreachable");
    }

    if (i<2)
      move_cursor_to_next_key(&cursor);
    free(new_key);
  }

  return result;
}

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

  char *dict_start = cursor;
  char *dict_end = cursor;
  while (*dict_end != '}') dict_end++;
  size_t dict_len = (size_t)(dict_end - dict_start);
  char *dict = calloc(dict_len + 1, sizeof(char));
  memcpy(dict, cursor, dict_len);

  DescrDict descr_dict = parse_dict(dict);
  free(dict);

  printf("data_type = %s\n", descr_dict.data_type);
  printf("fortran_order = %s\n", descr_dict.fortran_order ? "True" : "False");
  printf("Num of dims = %zu\n", descr_dict.shape.dims);
  for (size_t dim=0; dim<descr_dict.shape.dims; dim++) {
    printf("    Dim %zu has size %zu\n", dim, descr_dict.shape.eles[dim]);
  }
  
  free(descr_dict.data_type);
  free(descr_dict.shape.eles);
  free(buff_addr);

  return 0;
}
