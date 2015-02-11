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
* *x4djbx33a\_128_mmx* - MMX intrinsics implementations.
* *x4djbx33a\_128 sse2* - SSE2 intrinsics implementation.
* *x4djbx33a\_128 ssse3* - SSSE3 intrinsics implementation. SSSE3 has many useful new instructions, among them a mighty \_mm\_shuffle\_epi8
	which solves a problem in the SSE2 implementation.

benchmarks
----------

hashrates in MiB/s

| cpu/abi | compiler+options | djb x33a 32 ref | djb x33a 32 copt | djb x33a 128 ref | x4djb x33a 128 copt | x4djb x33a 128 mmx | x4djb x33a 128 sse2 | x4djb x33a 128 ssse3 |
|---------------------------------|-------------------------------|------|------|------|------|------|------|------|
| Atom N450 2x1.6GHz / amd64      | gcc-4.8 -O3 -march=native     |  374 |  623 |  128 |  590 |  478 |  452 |  954 |
| "                               | gcc-4.8 -O2 -march=native     |  374 |  627 |  128 |  606 |  478 |  452 | 1024 |
| "                               | gcc-4.8 -O2 -mssse3           |  303 |  585 |  139 |  508 |  428 |  414 |  915 |
| Core i7 4960HQ 4x2.6GHz / x64   | msvc 18.0 /O2 /Ob2 /arch:sse2 |  884 | 3315 | 1014 | 3340 |  | 2581 | 6272 |
| Core i7 4960HQ 4x2.6GHz / x86   | msvc 18.0 /O2 /Ob2 /arch:sse2 | 1168 | 3054 | 1168 | 1638 |  | 2586 | 6297 |
| Core i7 4960HQ 4x2.6GHz / amd64 | gcc-4.8 -O3 -march=native     | 1186 | 1581 |  963 | 2958 |  | 2723 | 6829 |
| Core i7 4960HQ 4x2.6GHz / amd64 | gcc-4.8 -O2 -march=native     | 1190 | 1609 | 1046 | 2782 |  | 2714 | 6869 |
| Core i7 4960HQ 4x2.6GHz / amd64 | gcc-4.8 -O2 -mssse3           | 1194 | 1618 | 1047 | 2950 |  | 2643 | 7044 |
| Core i7 3632QM 4x2.2GHz / amd64 | gcc-4.8 -O3 -march=native     | 1008 | 1312 |  754 | 2365 |  | 2371 | 5053 |
| Core i7 3632QM 4x2.2GHz / amd64 | gcc-4.8 -O2 -march=native     | 1007 | 1312 |  753 | 2438 |  | 2371 | 5058 |
| Core i7 3632QM 4x2.2GHz / amd64 | gcc-4.8 -O2 -mssse3           | 1007 | 1312 |  754 | 2445 |  | 2251 | 5188 |


lessons learned
---------------

As one can see in the benchmarks: beating a modern compiler is hard. Both gcc and msvc do a good job optimizing
the C implementations. The author has not yet found a SSE2 implementation that beats the C reference on all
platforms. While experimenting with the SSE2 instruction set, the author recognized that the data layout was not very
well suited for the instruction set and algorithm.
Alot of SSE2 performance gains come from reading input data in
aligned 128bit chunks (movdqa). If the data to be processed is layed out such that this is possible,
all performance follows naturally.
In case of the x4djb algorithms, the bytes have to be reordered from sequential to parallel
(0123012301230123 -> 0000111122223333).
This is similar to a matrix-transpose, but on a single 128bit register across the four 32bit dwords
(or in case of MMX across the two 32bit dwords in a 64bit register).
SSE2 has shuffle instructions, but for 16bit words, not for single bytes. MMX has no shuffle at all.
This means the shuffle needs to be emulated.
The author tried many variations of reading and shuffling inputs in SSE2 but it
remains the problematic part in the SSE2 implementation. SSSE3 added the \_mm\_shuffle\_epi8 instruction.
This solvs the problem in a single opcode and enables a consciese and very fast
vectorized solution that stands up to the expecations.


