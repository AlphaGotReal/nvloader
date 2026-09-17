#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <pthread.h>

#include "random_video.h"

int main(int argc, char **argv) {

  if (argc < 4) {
    fprintf(stderr, "usage: ./episode_gen <output-dir> <count> <gop>\n");
    return 1;
  }

  char nest[200];
  memset(nest, 0, sizeof(char) * 200);

  int l = strlen(argv[1]);
  snprintf(nest, sizeof(char) * 200, "%s", argv[1]);

  int GOP = atoi(argv[3]);
  int N = atoi(argv[2]);

  for (int t = 0; t < N; ++t) {
    snprintf(nest+l, sizeof(char) * (200-l), "/episode%d/data", t);
    int r = strlen(nest);

    char cmd[200];
    snprintf(cmd, sizeof(char) * 200, "mkdir -p %s", nest);
    system(cmd);

    // left video
    snprintf(nest+r, sizeof(char) * (200-r), "/left.perception_interface.camera.state.mp4");
    (void) generate_rv(nest, -1, -1, -1, GOP, NULL, NULL, 995 + rand()%10);

    // center video
    snprintf(nest+r, sizeof(char) * (200-r), "/middle.perception_interface.camera.state.mp4");
    (void) generate_rv(nest, -1, -1, -1, GOP, NULL, NULL, 995 + rand()%10);

    // right video
    snprintf(nest+r, sizeof(char) * (200-r), "/right.perception_interface.camera.state.mp4");
    (void) generate_rv(nest, -1, -1, -1, GOP, NULL, NULL, 995 + rand()%10);
  }
  
  return 0;
}
