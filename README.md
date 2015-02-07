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

| compiler+options/platform/cpu | djb2\_128\_cref | djb2\_128\_copt | djb2\_128\_sse2 | djb2\_128\_ssse3 |
|-------------------------------|-----------------|-----------------|-----------------|------------------|
| gcc-4.8.3 -O3 --march=native / amd64 / Atom N450 2x1.6GHz | 91 MiB/s | 376 MiB/s | 370 MiB/s | 949 MiB/s |
| gcc-4.8.3 -O2 --march=native / amd64 / Atom N450 2x1.6GHz | 97 MiB/s | 360 MiB/s | 370 MiB/s | 1040 MiB/s |

