#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sm_lib.h"

SM_float_array SM_new_float_array(size_t initial_cap) {
  SM_float_array retval = (SM_float_array) {
    .capacity = initial_cap,
    .length = 0,
    .data = malloc(initial_cap * sizeof(float))
  };

  if (retval.data == NULL) {
    fprintf(stderr, "Could not allocate memory for a new SM_float_array.");
    exit(1);
  }

  return retval;
}

SM_double_array SM_new_double_array(size_t initial_cap) {
  SM_double_array retval = (SM_double_array) {
    .capacity = initial_cap,
    .length = 0,
    .data = malloc(initial_cap * sizeof(double))
  };

  if (retval.data == NULL) {
    fprintf(stderr, "Could not allocate memory for a new SM_float_array.");
    exit(1);
  }

  return retval;
}

size_t load_two_column_csv(char *fname, SM_float_array *xs, SM_float_array *ys, size_t ignore_lines) {
  int fd = open(fname, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "Could not read %s: %s\n", fname, strerror(errno));
    return 1;
  }

  // Figure out the size of the file
  struct stat filestats;
  int res = fstat(fd, &filestats);
  if (res != 0) {
    fprintf(stderr, "Could not stat %s: %s\n", fname, strerror(errno));
    close(fd);
    return 1;
  }

  size_t file_size = (size_t)filestats.st_size;

  // Map the file into writeable memory
  char *file_contents = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (file_contents == MAP_FAILED) {
    fprintf(stderr, "Could not mmap %s: %s\n", fname, strerror(errno));
    return 1;
  }

  // char *copy = strdup(file_contents);
  char *copy = malloc(file_size * sizeof(char));
  if (memcpy(copy, file_contents, file_size) == NULL) {
    fprintf(stderr, "Could not allocate memory for %s: %s\n", fname, strerror(errno));
    return 1;
  }

  // Parse the file
  size_t N = 0;
  char *line = NULL;
  line = strtok(copy, "\n");
  while (line) {
    float x, y;
    int processed = sscanf(line, "%f, %f", &x, &y);
    if (ignore_lines > 0) {
      ignore_lines--;
    } else if (processed != 2) {
      fprintf(stdout, "Line could not be processed: '%s'\n", line);
    } else {
      SM_add_to_array(xs, x);
      SM_add_to_array(ys, y);
      N++;
    }
    line = strtok(NULL, "\n");
  }

  free(copy);
  munmap(file_contents, file_size);
  close(fd);

  return N;
}

int read_file_into_mem(char *fname, char **buff_addr) {
  int fd = open(fname, O_RDONLY);
  if (fd<0) {
    fprintf(stderr, "Could not open %s: %s\n", fname, strerror(errno));
    return -1;
  }

  struct stat file_stats;
  if (fstat(fd, &file_stats) < 0) {
    fprintf(stderr, "Could not stat %s: %s\n", fname, strerror(errno));
    return -1;
  }

  size_t file_size = (size_t)file_stats.st_size;

  char *fc = (char*)mmap(NULL, file_size, PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (fc == NULL) {
    fprintf(stderr, "Could not memory-map %s: %s\n", fname, strerror(errno));
    return -1;
  }

  *buff_addr = malloc(file_size * sizeof(char));
  *buff_addr = memcpy(*buff_addr, fc, file_size);
  if (buff_addr == NULL) {
    return -1;
  }

  return (int)file_size;
}

int SM_count_file_lines(char *fname) {
  char *buff = NULL;

  int N = read_file_into_mem(fname, &buff);
  if (N < 0) {
    return -1;
  }

  int linecntr = 0;
  for (int i=0; i<N; i++) {
    if (buff[i] == '\n') linecntr++;
  }

  free(buff);

  return linecntr;
}

