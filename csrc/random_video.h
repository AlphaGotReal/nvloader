#ifndef RANDOM_VIDEO_H
#define RANDOM_VIDEO_H

#include <stdbool.h>

/* generates a h264 video where 
 * every pixel is sampled from 
 * a discrete uniform distribution 
 * U(0, 255) with a continuous time
 * jitter sampled from U(0, 5) ms */
bool generate_rv(
    const char *file,
    int width,
    int height,
    int fps,
    int gop,
    const char *preset,
    const char *crf,
    int N); 

#endif
