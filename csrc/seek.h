#ifndef SEEK_H
#define SEEK_H

#include <stdbool.h>
#include <stdint.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/dict.h>

typedef struct dec_state dec_state_t;
typedef enum decode_status decode_status_t;

enum decode_status {
  DECODE_OK,
  DECODE_EOF,
  DECODE_ERROR
};

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

  // hardware decoder state
  AVBufferRef *hw_device_ctx;
};

/* init decoder */
dec_state_t *dec_open(const char *video_f);

/* deallocate decoder */
void dec_free(dec_state_t *ctx);

/* this seek is built around 
 * both random and sequentil 
 * seek, this will intelligently
 * choose to find the next keyframe
 * or to continue decoding 
 * return val:
 *   -1 => err
 *   0  => warm seek (O(G))
 *   1  => cold seek (O(logK + G))
 */
int seek_pts(dec_state_t *ctx, int64_t pts);

#endif
