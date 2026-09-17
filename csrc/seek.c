#include <stdio.h>
#include <stdlib.h>

dec_state_t *dec_open(const char *video_f) {

  dec_state_t *ctx = (dec_state_t *) malloc(sizeof(dec_state_t));
  if (!ctx) {
    fprintf(stderr, "failed to allocate memory for decoder state\n");
    return NULL;
  }

  ctx->cursor = -1;
  if (avformat_open_input(&ctx->fmt, video_f, NULL, NULL) < 0) {
    fprintf(stderr, "failed to open '%s'\n", video_f);
    goto fail;
  }

  ctx->video_stream_idx = av_find_best_stream(ctx->fmt, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
  if (ctx->video_stream_idx < 0) {
    fprintf(stderr, "no video stream found\n");
    goto fail;
  }

  ctx->stream = ctx->fmt->streams[ctx->video_stream_idx];
  const AVCodec *codec = avcodec_find_decoder(ctx->stream->codecpar->codec_id);

  if (!codec) {
    fprintf(stderr, "decoder not found\n");
    goto fail;
  }

  ctx->dec = avcodec_alloc_context3(codec);
  if (!ctx->dec)
    goto fail;

  if (avcodec_parameters_to_context(ctx->dec, ctx->stream->codecpar) < 0) {
    fprintf(stderr, "failed to copy codec params\n");
    goto fail;
  }

  if (avcodec_open2(ctx->dec, codec, NULL) < 0) {
    fprintf(stderr, "failed to open decoder\n");
    goto fail;
  }

  ctx->frame = av_frame_alloc();
  ctx->pkt   = av_packet_alloc();

  if (!ctx->frame || !ctx->pkt)
      goto fail;

  ctx->time_base = ctx->stream->time_base;
  ctx->frame_rate = av_guess_frame_rate(ctx->fmt, ctx->stream, NULL);

  return ctx;

fail:

  if (ctx) {
    av_frame_free(&ctx->frame);
    av_packet_free(&ctx->pkt);
    avcodec_free_context(&ctx->dec);
    if (ctx->fmt)
      avformat_close_input(&ctx->fmt);
    free(ctx);
  }

  return NULL;
}

void dec_close(dec_state_t *ctx) {
  if (!ctx)
    return;

  av_frame_free(&ctx->frame);
  av_packet_free(&ctx->pkt);
  avcodec_free_context(&ctx->dec);
  avformat_close_input(&ctx->fmt);

  free(ctx);
}
