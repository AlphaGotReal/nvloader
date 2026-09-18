#include <stdio.h>
#include <cuda_runtime.h>

#include "nv12_to_rgb.cuh"

// cuda kernel to convert nv12 to rgb
// so that everything remains in VRAM itself
__global__ void nv12_to_rgb_kernel(
    const uint8_t *y_plane,
    const uint8_t *uv_plane,
    int y_pitch,
    int uv_pitch,
    uint8_t *rgb,
    int width,
    int height) {

  int x = blockIdx.x * blockDim.x + threadIdx.x;
  int y = blockIdx.y * blockDim.y + threadIdx.y;

  if (x >= width || y >= height)
      return;

  uint8_t Y = y_plane[y * y_pitch + x];

  int uv_x = (x >> 1) * 2;
  int uv_y = (y >> 1);

  uint8_t U = uv_plane[uv_y * uv_pitch + uv_x + 0];

  uint8_t V = uv_plane[uv_y * uv_pitch + uv_x + 1];

  float yf = (float)Y;
  float uf = (float)U - 128.0f;
  float vf = (float)V - 128.0f;

  int r = (int)(yf + 1.402f * vf);
  int g = (int)(yf - 0.344136f * uf
                   - 0.714136f * vf);
  int b = (int)(yf + 1.772f * uf);

  r = min(255, max(0, r)); // these are __device__ methods
  g = min(255, max(0, g));
  b = min(255, max(0, b));

  int idx = (y * width + x) * 3;

  rgb[idx + 0] = r;
  rgb[idx + 1] = g;
  rgb[idx + 2] = b;
}

extern "C" void nv12_to_rgb(
    const uint8_t *y_plane,
    const uint8_t *uv_plane,
    int y_pitch,
    int uv_pitch,
    uint8_t *rgb,
    int width,
    int height,
    cudaStream_t stream) {

  // 16 * 16 = 256 threads/block 
  // good enough for a single pass kernel
  dim3 block(16, 16);
  dim3 grid(
      (width  + block.x - 1) / block.x,
      (height + block.y - 1) / block.y);

  nv12_to_rgb_kernel<<<grid, block, 0, stream>>>(
      y_plane,
      uv_plane,
      y_pitch,
      uv_pitch,
      rgb,
      width,
      height);

  cudaError_t err = cudaGetLastError();
  if (err != cudaSuccess) {
    fprintf(stderr, "nv12_to_rgb_kernel launch failed: %s\n", cudaGetErrorString(err));
  }

  // blocks CPU so that GPU finishes before
//   err = cudaDeviceSynchronize();
//   if (err != cudaSuccess) {
//     fprintf(stderr, "kernel execution failed: %s\n", cudaGetErrorString(err));
//   }
}
