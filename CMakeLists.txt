cmake_minimum_required(VERSION 2.8)
project(hashx4)

add_definitions(
  -Wall
  -O3
  #-g
  #-march=atom
  -march=native
  #-mssse3
  #-ftree-vectorize
  #-ftree-vectorizer-verbose=1
)

add_executable(testhx4
  testhx4.c
  hashx4.c
  hashx4.h
)
