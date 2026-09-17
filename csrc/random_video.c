#include <stdio.h>
#include <stdlib.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/dict.h>

#include "random_video.h"

bool generate_rv(
    const char *file,
    int width,
    int height,
    double fps,
    int gop,
    const char *preset,
    const char *crf,
    int N) {

  // sanity check
  if (file == NULL) {
    fprintf(stderr, "file '%s' does not exist\n", file);
    return false;
  }
  if (width <= 0) width = 640;
  if (height <= 0) width = 480;
  if (fps <= 0) fps = 30.0; 
  if (gop <= 0) {
    fprintf(stderr, "invalid gop size '%d'\n", gop);
    return false;
  }
  if (preset == NULL) preset = "fast";
  if (crf == NULL) crf = "23";
  if (N <= 0) N = 1000;

  printf("output: %s (%dx%d, %.2f fps, %d frames, codec=libx264, gop=%d, preset=%s)\n",
    file, width, height, fps, N, gop, preset);

  // encoding structs
  AVFormatContext *ofmt_ctx = NULL;
  AVCodecContext *codec_ctx = NULL;
  AVFrame *frame = NULL;
  AVPacket *pkt = NULL;

  avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, file);

  const AVCodec *enc_codec = avcodec_find_encoder_by_name("libx264");
  if (!enc_codec) {
    fprintf(stderr, "could not find 'libx264'\n");
    return false;
  }
}

