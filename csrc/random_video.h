#ifndef RANDOM_VIDEO_H
#define RANDOM_VIDEO_H

/* generates a h264 video where 
 * every pixel is sampled from 
 * a discrete uniform distribution 
 * U(0, 255) */
bool generate_rv(
    const char *file,
    int width,
    int height,
    double fps,
    int gop,
    const char *preset,
    const char *crf,
    int N); 

#endif
