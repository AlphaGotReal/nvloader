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
    int fps,
    int gop,
    const char *preset,
    const char *crf,
    int N) {

  // sanity check
  if (file == NULL) {
    fprintf(stderr, "output file is NULL\n");
    return false;
  }
  if (width <= 0) width = 640;
  if (height <= 0) height = 480;
  if (fps <= 0) fps = 30; 
  if (gop <= 0) {
    fprintf(stderr, "invalid gop size '%d'\n", gop);
    return false;
  }
  if (preset == NULL) preset = "fast";
  if (crf == NULL) crf = "23";
  if (N <= 0) N = 1000;

  printf("output: %s (%dx%d, %d fps, %d frames, codec=libx264, gop=%d, preset=%s)\n",
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

  AVStream *out_stream = avformat_new_stream(ofmt_ctx, enc_codec);

  codec_ctx = avcodec_alloc_context3(enc_codec);
  codec_ctx->width = width;
  codec_ctx->height = height;
  codec_ctx->time_base = (AVRational) {1, 1000000}; // ms resolution
  codec_ctx->framerate = (AVRational) {fps, 1};
  codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
  codec_ctx->gop_size = gop;
  codec_ctx->max_b_frames = 0;

  AVDictionary *opts = NULL;
  av_dict_set(&opts, "preset", preset, 0);
  av_dict_set(&opts, "crf", crf, 0);

  if (avcodec_open2(codec_ctx, enc_codec, &opts) < 0) {
    fprintf(stderr, "could not open encoder 'libx264'\n");
    return false;
  }
  av_dict_free(&opts);

  avcodec_parameters_from_context(out_stream->codecpar, codec_ctx);
  out_stream->time_base = codec_ctx->time_base;

  if (avio_open(&ofmt_ctx->pb, file, AVIO_FLAG_WRITE) < 0) {
    fprintf(stderr, "could not open output file '%s'\n", file);
    return false;
  }

  if (avformat_write_header(ofmt_ctx, NULL) < 0) {
    fprintf(stderr, "could not write header\n");
    return false;
  }

  frame = av_frame_alloc();
  frame->format = codec_ctx->pix_fmt;
  frame->width  = codec_ctx->width;
  frame->height = codec_ctx->height;
  av_frame_get_buffer(frame, 0);

  pkt = av_packet_alloc();

  int t = 0;
  int dt = 1e6/fps;

  for (int i = 0; i < N; ++i) {
    av_frame_make_writable(frame);

    for (int y = 0; y < height; y++)
      for (int x = 0; x < width; x++)
        frame->data[0][y * frame->linesize[0] + x] = rand() % 256;

    for (int y = 0; y < height / 2; y++)
      for (int x = 0; x < width / 2; x++)
        frame->data[1][y * frame->linesize[1] + x] = rand() % 256;

    for (int y = 0; y < height / 2; y++)
      for (int x = 0; x < width / 2; x++)
        frame->data[2][y * frame->linesize[2] + x] = rand() % 256;

    frame->pts = t + i * dt - 5000 + rand() % 10000; // 5 ms jitter

    // flush the frames
    avcodec_send_frame(codec_ctx, frame);
    while (avcodec_receive_packet(codec_ctx, pkt) == 0) {
      av_packet_rescale_ts(pkt, codec_ctx->time_base, out_stream->time_base);
      pkt->stream_index = out_stream->index;
      av_interleaved_write_frame(ofmt_ctx, pkt);
      av_packet_unref(pkt);
    }
  }

  // final flush
  avcodec_send_frame(codec_ctx, NULL);
  while (avcodec_receive_packet(codec_ctx, pkt) == 0) {
      av_packet_rescale_ts(pkt, codec_ctx->time_base, out_stream->time_base);
      pkt->stream_index = out_stream->index;
      av_interleaved_write_frame(ofmt_ctx, pkt);
      av_packet_unref(pkt);
  }

  av_write_trailer(ofmt_ctx);

  av_frame_free(&frame);
  av_packet_free(&pkt);
  avcodec_free_context(&codec_ctx);
  avio_closep(&ofmt_ctx->pb);
  avformat_free_context(ofmt_ctx);

  return true;
}
