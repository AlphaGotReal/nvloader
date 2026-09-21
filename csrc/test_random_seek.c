#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "seek.h"

static double now_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int main(int argc, char **argv) {

  if (argc < 2) {
    fprintf(stderr, "usage: test_random_seek <video_file>\n");
    return 1;
  }

  dec_state_t *ctx = dec_open(argv[1]);
  if (!ctx) {
    fprintf(stderr, "failed to open decoder\n");
    return 1;
  }

  if (ctx->stream->duration == AV_NOPTS_VALUE) {
    fprintf(stderr, "stream duration unavailable\n");
    dec_free(ctx);
    return 1;
  }

  srand((unsigned)time(NULL));

  const int N = 30;
  int warm = 0;
  int cold = 0;

  double total_ms = 0.0;
  double min_ms = 1e30;
  double max_ms = 0.0;

  for (int i = 0; i < N; ++i) {

    int64_t pts =
      ((double)rand() / RAND_MAX) * ctx->stream->duration;

    double t0 = now_ms();

    int ret = seek_pts(ctx, pts);

    double t1 = now_ms();
    double dt = t1 - t0;
    
    printf("duration=%lld target=%lld\n",
       (long long)ctx->stream->duration,
       (long long)pts);

    if (ret < 0) {
      fprintf(stderr,
              "seek failed at pts=%lld\n",
              (long long)pts);
      dec_free(ctx);
      return 1;
    }

    if (ret == 0)
      warm++;
    else
      cold++;

    total_ms += dt;

    if (dt < min_ms) min_ms = dt;
    if (dt > max_ms) max_ms = dt;
  }

  printf("Random seek benchmark\n");
  printf("---------------------\n");
  printf("seeks      : %d\n", N);
  printf("warm seeks : %d\n", warm);
  printf("cold seeks : %d\n", cold);
  printf("avg time   : %.3f ms\n", total_ms / N);
  printf("min time   : %.3f ms\n", min_ms);
  printf("max time   : %.3f ms\n", max_ms);

  dec_free(ctx);
  return 0;
}
