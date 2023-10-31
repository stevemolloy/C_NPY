#ifndef _C_NPY_H
#define _C_NPY_H

#include <stdbool.h>
#include <stdio.h>

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

void move_cursor_to_next_key(char **cursor);

char* get_python_string(char **pystr);

bool get_python_bool(char **pystr);

PythonTuple get_python_tuple(char **pystr);

DescrDict parse_dict(char *dictstr);

int get_numpy_file_repr(char *buff_addr, NumpyFileRepr* nfr);

#endif // !_C_NPY_H

