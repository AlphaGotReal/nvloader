cmake_minimum_required(VERSION 3.16)
project(nvloader LANGUAGES C CXX CUDA)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# set(CMAKE_CUDA_ARCHITECTURES 90) # h200
set(CMAKE_CUDA_ARCHITECTURES 120) # 5060RTX

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)
find_package(Torch REQUIRED)
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

# python bindings
pybind11_add_module(_nvloader
  cppsrc/bindings.cpp
  cppsrc/dataset.cpp)

target_include_directories(_nvloader
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/cppsrc
    ${CMAKE_CURRENT_SOURCE_DIR}/csrc
    ${CMAKE_CURRENT_SOURCE_DIR}/cusrc)

target_link_libraries(_nvloader
  PRIVATE
    nvloader
    ${TORCH_LIBRARIES}
    pybind11::module)

set_target_properties(_nvloader
  PROPERTIES
    PREFIX "")
