cmake_minimum_required(VERSION 2.8)
project(hashx4)

include_directories(
  "${CMAKE_SOURCE_DIR}/inc/"
  "${CMAKE_SOURCE_DIR}/src/"
)

add_definitions(
  -Wall
  -O2
  #-g
  #-march=atom
  -march=native
  #-mssse3
  #-ftree-vectorize
  #-ftree-vectorizer-verbose=1
)

add_executable(testhx4
  util/testhx4.c
 
  src/hashx4_util.h
  src/hashx4_util.c
  src/hashx4.c
  src/siphash24.c

  inc/hashx4.h
)
