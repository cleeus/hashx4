cmake_minimum_required(VERSION 2.8)
project(hashx4)

include_directories(
  "${CMAKE_SOURCE_DIR}/inc/"
  "${CMAKE_SOURCE_DIR}/src/"
)

if(NOT MSVC)
add_definitions(
  -Wall
  -O2
  #-g
  #-march=atom
  #-march=native
  -mssse3
  #-ftree-vectorize
  #-ftree-vectorizer-verbose=1
)
endif()

add_executable(testhx4
  util/testhx4.c
 
  src/hx4_util.h
  src/hx4_util.c
  src/hx4_djbx33a.c
  src/siphash24.c
  src/hx4_siphash24.c

  inc/hashx4.h
  inc/hashx4_config.h
)
