#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "c_npy.h"
#include "sm_lib.h"

int main(void) {
  char *filename = "examples/matrix.npy";
  SM_double_array data = SM_new_double_array(256);

  if (get_data_from_npy_file(filename, &data) < 0) return 1;

  for (size_t i=0; i<data.length; i++) printf("%e\n", data.data[i]);

  SM_free(data);

  return 0;
}

