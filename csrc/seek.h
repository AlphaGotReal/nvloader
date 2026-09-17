#ifndef SEEK_H
#define SEEK_H

#include <stdbool.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/dict.h>

typedef struct dec_state dec_state_t;

/* unique decoder state
 * initialized for one video */
struct dec_state {
  // codec state
  AVFormatContext *fmt;
  AVCodecContext  *dec;

  // stream state
  AVStream *stream;
  int video_stream_idx;

  // frame state
  AVFrame  *frame;
  AVPacket *pkt;

  // pointer to the video frame
  int cursor;

  AVRational time_base;
  AVRational frame_rate;
};

/* init decoder */
dec_state_t *dec_open(const char *video_f);

/* deallocate decoder */
void dec_free(dec_state_t *ctx);

/* this seek is built around 
 * both random and sequentil 
 * seek, this will intelligently
 * choose to find the next keyframe
 * or to continue decoding */
bool seek_pts(dec_state_t *ctx, int pts);
bool seek_frame(dec_state_t *ctx, int frame_no);

#endif
