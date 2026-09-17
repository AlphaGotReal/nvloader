cmake_minimum_required(VERSION 3.16)
project(nvloader C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavformat libavcodec libavutil libswscale)

add_executable(episode_gen
  csrc/episode_gen.c
  csrc/random_video.c
)
target_link_libraries(episode_gen PkgConfig::FFMPEG)

add_library(${PROJECT_NAME} SHARED csrc/seek.c)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC PkgConfig::FFMPEG)

add_executable(test_sequential_seek
  csrc/test_sequential_seek.c
)
target_link_libraries(test_sequential_seek PkgConfig::FFMPEG ${PROJECT_NAME})
