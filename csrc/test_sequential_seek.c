#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "seek.h"

int main(int argc, char **argv) {

  if (argc < 2) {
    fprintf(stderr, "usage: test_sequential_seek <video_file>\n");
    return 1;
  }

  dec_state_t *ctx = dec_open(argv[1]);
  if (!ctx) {
    fprintf(stderr, "failed to open decoder\n");
    return 1;
  }

  srand((unsigned)time(NULL));

  int64_t duration = ctx->stream->duration;

  if (duration == AV_NOPTS_VALUE) {
    fprintf(stderr, "stream duration unavailable\n");
    dec_free(ctx);
    return 1;
  }

  int64_t three_sec = av_rescale_q(3, (AVRational){1, 1}, ctx->stream->time_base);
  int64_t pts = 0;

  printf("running sequential seek test...\n");
  for (int i = 0; i < 1000; ++i) {

    /* move forward by < 3 seconds */
    int64_t step = rand() % (three_sec - 1) + 1;
    pts += step;

    if (pts >= duration)
      break;

    int ret = seek_pts(ctx, pts);

    if (ret < 0) {
      fprintf(stderr, "ERROR: seek failed at pts=%lld\n", (long long)pts);
      dec_free(ctx);
      return 1;
    }

    if (i == 0) continue;

    if (ret == 1) {
      fprintf(stderr, "FAILED: cold seek detected at pts=%lld\n", (long long)pts);
      dec_free(ctx);
      return 1;
    }
  }

  printf("PASS: all seeks were warm\n");

  dec_free(ctx);
  return 0;
}
