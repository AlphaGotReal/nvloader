#include <stdio.h>
#include <stdlib.h>

#include "seek.h"
#include "nv12_to_rgb.cuh"

#include <unistd.h>
#include <time.h>

double get_wall_time() {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return (double)time.tv_sec + (double)time.tv_nsec * 1e-9;
}

#define cnow (clock() * 1e-6)
#define rnow get_wall_time()

dec_state_t *dec_open(const char *video_f) {

  dec_state_t *ctx = (dec_state_t *) malloc(sizeof(dec_state_t));
  if (!ctx) {
    fprintf(stderr, "failed to allocate memory for decoder state\n");
    return NULL;
  }
  memset(ctx, 0, sizeof(*ctx));

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
  const AVCodec *codec = avcodec_find_decoder_by_name("h264_cuvid");

  if (!codec) {
    fprintf(stderr, "decoder not found\n");
    goto fail;
  }

  ctx->dec = avcodec_alloc_context3(codec);
  if (!ctx->dec)
    goto fail;

  // cuda device
  if (av_hwdevice_ctx_create(&ctx->hw_device_ctx, AV_HWDEVICE_TYPE_CUDA, NULL, NULL, 0) < 0) {
    fprintf(stderr, "failed to create CUDA device\n");
    goto fail;
  }

  ctx->dec->hw_device_ctx = av_buffer_ref(ctx->hw_device_ctx);
  if (avcodec_parameters_to_context(ctx->dec, ctx->stream->codecpar) < 0) {
    fprintf(stderr, "failed to copy codec params\n");
    goto fail;
  }

  if (avcodec_open2(ctx->dec, codec, NULL) < 0) {
    fprintf(stderr, "failed to open decoder\n");
    goto fail;
  }

  // cuda alloc
  cudaStreamCreate(&ctx->cuda_stream);
  ctx->rgb_size = ctx->dec->width * ctx->dec->height * 3;
  cudaMalloc((void **)&ctx->rgb_buffer, ctx->rgb_size); // device memory alloc

  ctx->frame = av_frame_alloc();
  ctx->pkt   = av_packet_alloc();
  if (!ctx->frame || !ctx->pkt)
      goto fail;
  return ctx;

fail:
  if (ctx) {
    cudaFree(ctx->rgb_buffer);
    cudaStreamDestroy(ctx->cuda_stream);
    av_frame_free(&ctx->frame);
    av_packet_free(&ctx->pkt);
    avcodec_free_context(&ctx->dec);
    if (ctx->hw_device_ctx)
      av_buffer_unref(&ctx->hw_device_ctx);
    if (ctx->fmt)
      avformat_close_input(&ctx->fmt);

    free(ctx);
  }
  return NULL;
}

void dec_free(dec_state_t *ctx) {
  if (!ctx)
    return;

  cudaFree(ctx->rgb_buffer);
  cudaStreamDestroy(ctx->cuda_stream);
  av_frame_free(&ctx->frame);
  av_packet_free(&ctx->pkt);
  avcodec_free_context(&ctx->dec);
  avformat_close_input(&ctx->fmt);
  av_buffer_unref(&ctx->hw_device_ctx);
  free(ctx);
}

static decode_status_t decode_next_frame(dec_state_t *ctx) {
  while (true) {
    int ret = avcodec_receive_frame(ctx->dec, ctx->frame);
    if (ret == 0)
      return DECODE_OK;
    if (ret == AVERROR_EOF)
      return DECODE_EOF;
    if (ret != AVERROR(EAGAIN))
      return DECODE_ERROR;
    while ((ret = av_read_frame(ctx->fmt, ctx->pkt)) >= 0) {
      if (ctx->pkt->stream_index == ctx->video_stream_idx) {
        if (avcodec_send_packet(ctx->dec, ctx->pkt) < 0) {
          av_packet_unref(ctx->pkt);
          return DECODE_ERROR;
        }
      }
      av_packet_unref(ctx->pkt);
      break;
    }
    if (ret == AVERROR_EOF) {
      /* flush decoder as EOF is not the end */
      avcodec_send_packet(ctx->dec, NULL);
      continue;
    }
    if (ret < 0)
      return DECODE_ERROR;
  }
}

int seek_pts(dec_state_t *ctx, int64_t pts) {

  // clamp the pts
  if (pts < 0) pts = 0;
  if (ctx->stream->duration != AV_NOPTS_VALUE && pts > ctx->stream->duration)
    pts = ctx->stream->duration;

  int seek_ret = 1; // cold

  // jump or not ??
  if (ctx->frame && ctx->frame->pts != AV_NOPTS_VALUE) {
    int64_t cur_pts = ctx->frame->pts;
    if (pts >= cur_pts) {
      int64_t delta = pts - cur_pts;
      // heuristic: if within 1 sec continue decoding forward
      int64_t threshold = av_rescale_q(3, (AVRational){1, 1}, ctx->stream->time_base);
      if (delta <= threshold) {
        seek_ret = 0; // warm
        goto decode;
      }
    }
  }

  double b = rnow;
  // jump to nearest keyframe (optionally)
  if (av_seek_frame(ctx->fmt, ctx->video_stream_idx, pts, AVSEEK_FLAG_BACKWARD) < 0) {
    return -1;
  }avcodec_flush_buffers(ctx->dec);
  printf("seek : %.2fms", (rnow - b) * 1000);

  // decode forward
decode: {

  double s = rnow;
  while (true) {
    decode_status_t st = decode_next_frame(ctx);
    if (st == DECODE_ERROR)
        return -1;
    if (st == DECODE_EOF)
        break;
    if (ctx->frame->pts == AV_NOPTS_VALUE)
        continue;
    if (ctx->frame->pts >= pts)
        break;
  }
  printf("decode forward: %2fms\n", (rnow - s) * 1000);

}
  // store rgb frames in VRAM
//   nv12_to_rgb(
//     (const uint8_t *)ctx->frame->data[0],
//     (const uint8_t *)ctx->frame->data[1],
//     ctx->frame->linesize[0],
//     ctx->frame->linesize[1],
//     ctx->rgb_buffer,
//     ctx->frame->width,
//     ctx->frame->height,
//     ctx->cuda_stream);
// 
  // download the rgb frames into CPU
//   uint8_t *host = malloc(ctx->rgb_size);
//   cudaMemcpy(
//     host,
//     ctx->rgb_buffer,
//     ctx->rgb_size,
//     cudaMemcpyDeviceToHost
//   );

//   printf("%s\n", av_get_pix_fmt_name(ctx->frame->format));  

//   AVHWFramesContext *hw =
//     (AVHWFramesContext *)ctx->frame->hw_frames_ctx->data;
// 
//   printf("hw format=%s\n", av_get_pix_fmt_name(hw->sw_format));
// 
//   printf("data0=%p\n", ctx->frame->data[0]);
//   printf("data1=%p\n", ctx->frame->data[1]);
//   printf("ls0=%d\n", ctx->frame->linesize[0]);
//   printf("ls1=%d\n", ctx->frame->linesize[1]);
//   printf("format=%s\n",
//          av_get_pix_fmt_name(ctx->frame->format));

  return seek_ret;
}
