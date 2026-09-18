cmake_minimum_required(VERSION 3.16)
project(nvloader LANGUAGES C CUDA)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# set(CMAKE_CUDA_ARCHITECTURES 90) # h200
set(CMAKE_CUDA_ARCHITECTURES 120) # 5060RTX

find_package(CUDAToolkit REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavformat libavcodec libavutil libswscale)

# episode generator exe
add_executable(episode_gen
  csrc/episode_gen.c
  csrc/random_video.c)
target_link_libraries(episode_gen PkgConfig::FFMPEG)

add_library(${PROJECT_NAME} SHARED
  csrc/seek.c
  cusrc/nv12_to_rgb.cu)
target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/csrc
    ${CMAKE_CURRENT_SOURCE_DIR}/cusrc)
target_link_libraries(${PROJECT_NAME} 
  PUBLIC
    PkgConfig::FFMPEG
    CUDA::cudart)

set_target_properties(${PROJECT_NAME}
  PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

# tests
add_executable(test_sequential_seek
  csrc/test_sequential_seek.c)
target_link_libraries(test_sequential_seek PkgConfig::FFMPEG ${PROJECT_NAME})

add_executable(test_random_seek
  csrc/test_random_seek.c)
target_link_libraries(test_random_seek PkgConfig::FFMPEG ${PROJECT_NAME})
