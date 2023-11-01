#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "c_npy.h"
#include "sm_lib.h"

int main(int argc, char **argv) {
  bool verbose = false;
  if (argc>1) {
    if (strcmp(argv[1], "-v")==0 || strcmp(argv[1], "--verbose")==0) {
      verbose = true;
    }
  }
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

  if (verbose) {
    printf("Numpy version string: %d.%d\n", numpy_file.major, numpy_file.minor);
    printf("Data_location: %p\n", (void*)numpy_file.data_location);
    switch (numpy_file.description.data_type) {
      case CNPY_DOUBLE:
        printf("Data type: DOUBLE\n");
        break;
      case CNPY_FLOAT:
        printf("Data type: FLOAT\n");
        break;
      case CNPY_INT:
        printf("Data type: INT\n");
        break;
      default:
        assert(0 && "Unreachable");
    }
    printf("Fortran order?: %s\n", numpy_file.description.fortran_order ? "True" : "False");
    printf("Dimensionality of data: %zu\n", numpy_file.description.shape.dims);
    for (size_t i=0; i<numpy_file.description.shape.dims; i++) {
      printf("    Dim[%zu] has size: %zu\n", i, numpy_file.description.shape.eles[i]);
    }
  }

  SM_double_array data = get_numpy_data(numpy_file);

  for (size_t i=0; i<data.length; i++) {
    printf("%e\n", data.data[i]);
  }

  SM_free(data);
  free(numpy_file.description.shape.eles);
  free(buff_addr);

  return 0;
}
