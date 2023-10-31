#include <stdio.h>

#include "c_npy.h"
#include "sm_lib.h"

int main(void) {
  char *filename = "examples/vector.npy";
  
  char *buff_addr;
  if (read_file_into_mem(filename, &buff_addr) < 0) {
    fprintf(stderr, "Could not read file: '%s'\n", filename);
    return 1;
  }

  NumpyFileRepr numpy_file = {0};
  if (get_numpy_file_repr(buff_addr, &numpy_file) < 0) {
    return 1;
  }

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
