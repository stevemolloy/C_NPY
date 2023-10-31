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

int get_numpy_file_repr(char *buff_addr, NumpyFileRepr* nfr);

#endif // !_C_NPY_H

