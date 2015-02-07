hashx4
======

A collection of SIMD optimized general purpose hash functions.

This is a demonstration of a simple method to speed up any general
purpose hash function by using SSE or other SIMD instructions:
Instead of driving all the input through
one instance of the hash function, you instantiate four hash
function states and while reading the input you iterate over
all four instances. This can then be implemented very efficiently
in most SIMD instruction sets.

Copyright Kai Dietrich <mail@cleeus.de> 2015.
Hashx4 is licensed under the GPL v3 or later.


benchmarks
==========

hash-rates in MiB/s

| cpu/abi/compiler+options | djb2\_32 ref | djb2\_32 copt | djb2x4\_128 cref | djb2x4\_128 copt | djb2x4\_128 sse2 | djb2x4\_128 ssse3 |
|--------------------------|--------------|---------------|------------------|------------------|------------------|-------------------|
| Atom N450 2x1.6GHz / amd64 / gcc-4.8 -O3 -march=native      |  376 |  360 |   91 |  376 |  370 |  949 |
| Atom N450 2x1.6GHz / amd64 / gcc-4.8 -O2 -march=native      |  374 |  234 |   97 |  360 |  370 | 1040 |
| Atom N450 2x1.6GHz / amd64 / gcc-4.8 -O2 -mssse3            |  218 |  221 |   96 |  319 |  186 |  910 |
| Core i7-4960HQ 4x2.6GHz / x64 / msvc 12.0 /O2 /Ob2          | | | 1008 | 3276 | 2016 | 5957 |
| Core i7-4960HQ 4x2.6GHz / amd64 / gcc-4.8 -O3 -march=native | | | 1071 | 2629 | 2022 | 6368 |
| Core i7-4960HQ 4x2.6GHz / amd64 / gcc-4.8 -O2 -march=native | | | 1073 | 2541 | 2021 | 6513 |
| Core i7-3632QM 4x2.2GHz / amd64 / gcc-4.8 -O3 -march=native | | |  730 | 2066 | 2284 | 4874 |
| Core i7-3632QM 4x2.2GHz / amd64 / gcc-4.8 -O2 -march=native | | |  724 | 2091 | 2270 | 4834 |
| Core i7-3632QM 4x2.2GHz / amd64 / gcc-4.8 -O2 -mssse3       | | |  731 | 2096 | 1937 | 5162 |


