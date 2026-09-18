#include <stdint.h>
#include <cuda_runtime.h>

// cuda kernel to convert nv12 to rgb
// so that everything remains in VRAM itself
__global__ void nv12_to_rgb_kernel(
    const uint8_t *y_plane,
    const uint8_t *uv_plane,
    int y_pitch,
    int uv_pitch,
    uint8_t *rgb,
    int width,
    int height);

// a __host__ launcher function
void nv12_to_rgb(
    const uint8_t *y_plane,
    const uint8_t *uv_plane,
    int y_pitch,
    int uv_pitch,
    uint8_t *rgb,
    int width,
    int height);
