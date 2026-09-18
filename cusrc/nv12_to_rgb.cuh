#ifndef NV12_TO_RGB_CUH
#define NV12_TO_RGB_CUH

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// a __host__ launcher function
void nv12_to_rgb(
    const uint8_t *y_plane,
    const uint8_t *uv_plane,
    int y_pitch,
    int uv_pitch,
    uint8_t *rgb,
    int width,
    int height);

#ifdef __cplusplus
}
#endif

#endif
