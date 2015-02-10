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


algorithms
----------

* *djbx33a\_32 ref* - This is the well known DJBX33A hash function from Daniel Bernstein (h\_i+1 = h\_i * 33 + c\_i+1, h\_0 = 5381).
	It is included to show a baseline in benchmarks. The implementation is a naive loop you might find in any example code.
* *djbx33a\_32 copt* - The same DJBX33A hash function with an alignment seek
	to give the compiler a chance to vectorize and/or optimize better.
* *x4djbx33a\_128 ref* - Interleaved input on 4 djbx33a functions (hence x4djbx33a), 128bit output.
	A naive loop implementation that can serve as
	a reference to check the correctness of the optimized implementations.
* *x4djbx33a\_128 copt* - The same x4djbx33a function with some alignment hints for the compiler.
* *x4djbx33a\_128 sse2* - SSE2 intrinsics implementation.
* *x4djbx33a\_128 ssse3* - SSSE3 intrinsics implementation. SSSE3 has many useful new instructions, among them a mighty \_mm\_shuffle\_epi8
	which solves a problem in the SSE2 implementation.

benchmarks
----------

hashrates in MiB/s

| cpu/abi/compiler+options | djb x33a 32 ref | djb x33a 32 copt | djb x33a 128 ref | x4djb x33a 128 copt | x4djb x33a 128 sse2 | x4djb x33a 128 ssse3 |
|--------------------------|--------------|---------------|-----------------|------------------|------------------|-------------------|
| Atom N450 2x1.6GHz / amd64 / gcc-4.8 -O3 -march=native      |  376 |  360 |   91 |  376 |  370 |  949 |
| Atom N450 2x1.6GHz / amd64 / gcc-4.8 -O2 -march=native      |  374 |  234 |   97 |  360 |  370 | 1040 |
| Atom N450 2x1.6GHz / amd64 / gcc-4.8 -O2 -mssse3            |  218 |  221 |   96 |  319 |  186 |  910 |
| Core i7 4960HQ 4x2.6GHz / x64 / msvc 18.0 /O2 /Ob2          |  885 |  891 | 1016 | 3357 | 2017 | 6225 |
| Core i7 4960HQ 4x2.6GHz / x86 / msvc 18.0 /O2 /Ob2          | 1170 | 1202 | 1170 | 1702 | 1956 | 6243 |
| Core i7 4960HQ 4x2.6GHz / amd64 / gcc-4.8 -O3 -march=native | 1015 | 1165 | 1071 | 2672 | 2031 | 6368 |
| Core i7 4960HQ 4x2.6GHz / amd64 / gcc-4.8 -O2 -march=native | 1195 | 1195 | 1073 | 2541 | 2021 | 6513 |
| Core i7 4960HQ 4x2.6GHz / amd64 / gcc-4.8 -O2 -mssse3       | 1195 | 1196 | 1073 | 2555 | 2708 | 6491 |
| Core i7 3632QM 4x2.2GHz / amd64 / gcc-4.8 -O3 -march=native |  975 |  977 |  730 | 2066 | 2284 | 4874 |
| Core i7 3632QM 4x2.2GHz / amd64 / gcc-4.8 -O2 -march=native |  990 |  980 |  724 | 2091 | 2270 | 4834 |
| Core i7 3632QM 4x2.2GHz / amd64 / gcc-4.8 -O2 -mssse3       |  987 |  987 |  731 | 2096 | 1937 | 5162 |


