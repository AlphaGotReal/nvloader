#include <stdio.h>
#include "seek.h"

int main(int argc, char **argv) {

  if (argc < 2) {
    fprintf(stderr, "usage: test_sequential_seek <video_file>\n");
    return 1;
  }

  return 0;
}
