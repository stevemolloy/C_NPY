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

typedef struct {
  unsigned int major;
  unsigned int minor;
  char *data_location;
  DescrDict description;
} NumpyFileRepr;

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

int get_numpy_file_repr(char *buff_addr, NumpyFileRepr* nfr) {
  char *cursor;
  char *magic_string = "\x93NUMPY";
  if (strncmp(buff_addr, magic_string, strlen(magic_string)) != 0) {
    fprintf(stderr, "Not a valid NPY file\n");
    return -1;
  }

  cursor = buff_addr + strlen(magic_string);
  unsigned int major_version = (unsigned int)*cursor++;
  unsigned int minor_version = (unsigned int)*cursor++;

  size_t hdr_len = (size_t)*(unsigned short int*)(cursor);

  char* data_start = cursor + hdr_len;

  cursor += 2;

  // Now to scan through the python-style dictionary
  if (*cursor++ != '{') {
    fprintf(stderr, "Not a valid NPY file\n");
    return -1;
  }

  char *dict_start = cursor;
  char *dict_end = cursor;
  while (*dict_end != '}') dict_end++;
  size_t dict_len = (size_t)(dict_end - dict_start);
  char *dict = calloc(dict_len + 1, sizeof(char));
  memcpy(dict, cursor, dict_len);

  DescrDict descr_dict = parse_dict(dict);
  free(dict);

  nfr->major = major_version;
  nfr->minor = minor_version;
  nfr->data_location = data_start;
  nfr->description = descr_dict;

  return 0;
}

int main(void) {
  char *filename = "examples/vector.npy";
  
  char *buff_addr;
  if (read_file_into_mem(filename, &buff_addr) < 0) {
    fprintf(stderr, "Could not read file: '%s'\n", filename);
    return 1;
  }

  NumpyFileRepr numpy_file = {0};
  get_numpy_file_repr(buff_addr, &numpy_file);

  printf("Numpy version string: %d.%d\n", numpy_file.major, numpy_file.minor);
  printf("Data_location: %p\n", (void*)numpy_file.data_location);
  printf("Data type: %s\n", numpy_file.description.data_type);
  printf("Fortran order?: %s\n", numpy_file.description.fortran_order ? "True" : "False");
  printf("Data dimensions: %zu\n", numpy_file.description.shape.dims);
  for (size_t i=0; i<numpy_file.description.shape.dims; i++) {
    printf("    Dim[%zu] has size: %zu\n", i, numpy_file.description.shape.eles[i]);
  }

  free(numpy_file.description.data_type);
  free(numpy_file.description.shape.eles);
  free(buff_addr);

  return 0;
}
