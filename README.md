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
	which is used to avoid unpacking and uses fewer registers (but seems to be a bit slower).

benchmarks
----------

The micro benchmark used here repeatedly hashes 4k of data until 10s are elapsed.
It then calculates the hashrate in MiB/s.
Gcc version used is 4.8.2 or 4.8.3.
Msvc version is 18.0 (Visual Studio 2013).

| cpu | abi | compiler | djb x33a 32 ref | djb x33a 32 copt | djb x33a 128 ref | x4djb x33a 128 copt | x4djb x33a 128 mmx | x4djb x33a 128 sse2 | x4djb x33a 128 ssse3 |
|-----------------------|-------|---------------------------|-----:|-----:|-----:|-----:|-----:|-----:|-----:|
| Core i7 4960HQ 2.6GHz | x64   | msvc /O2 /Ob2 /arch:sse2  |  898 | 3520 | 1013 | 3510 |    - | 6820 | 6680 |
| "                     | x86   | msvc /O2 /Ob2 /arch:sse2  | 1121 | 2970 | 1184 | 1654 | 2200 | 6770 | 6610 |
| "                     | amd64 | gcc -O3 -march= native    | 1192 | 1620 | 1058 | 2984 | 3534 | 6802 | 6683 |
| "                     | amd64 | gcc -O2 -march= native    | 1200 | 1616 | 1018 | 3116 | 3492 | 6707 | 6598 |
| "                     | amd64 | gcc -O2 -mssse3           | 1198 | 1614 | 1039 | 3119 | 3484 | 6196 | 6658 |
| "                     | x86   | gcc -O3 -march= native    | 1183 | 1597 | 1188 | 2916 | 3549 | 6524 | 6461 |
| "                     | x86   | gcc -O2 -march= native    | 1195 | 1613 | 1191 | 2792 | 3511 | 6486 | 6377 |
| "                     | x86   | gcc -O2 -mssse3           | 1196 | 1611 | 1191 | 2792 | 3521 | 6520 | 5862 |
| Core i7 3632QM 2.2GHz | amd64 | gcc -O3 -march= native    | 1008 | 1309 |  753 | 2386 | 2805 | 5806 | 5084 |
| "                     | amd64 | gcc -O2 -march= native    | 1007 | 1308 |  751 | 2426 | 2775 | 5648 | 4981 |
| "                     | amd64 | gcc -O2 -mssse3           | 1005 | 1307 |  745 | 2455 | 2721 | 5570 | 5071 |
| "                     | x86   | gcc -O3 -march= native    | 1010 | 1319 |  758 | 2366 | 2813 | 5642 | 4978 |
| "                     | x86   | gcc -O2 -march= native    | 1007 | 1314 |  755 | 2336 | 2778 | 5527 | 4875 |
| "                     | x86   | gcc -O2 -mssse3           | 1007 | 1306 |  755 | 2337 | 2805 | 5479 | 4943 |
| Athlon XP 2800 2.0GHz | x86   | gcc -O3 -march= native    |  451 |  878 |  388 |  923 | 1264 |    - |    - |
| "                     | x86   | gcc -O2 -march= native    |  451 |  875 |  390 |  893 | 1256 |    - |    - |
| "                     | x86   | gcc -O2 -msse             |  488 |  830 |  356 |  918 | 1253 |    - |    - |
| Atom N450 1.6GHz      | amd64 | gcc -O3 -march= native    |  388 |  656 |  130 |  623 |  743 | 1453 | 1075 |
| "                     | amd64 | gcc -O2 -march= native    |  386 |  651 |  130 |  622 |  728 | 1385 |  919 |
| "                     | amd64 | gcc -O2 -mssse3           |  310 |  604 |  141 |  517 |  657 | 1143 |  931 |
| "                     | x86   | gcc -O3 -march= native    |  391 |  609 |  157 |  541 |  809 | 1423 |  913 |
| "                     | x86   | gcc -O2 -march= native    |  389 |  606 |  156 |  533 |  790 | 1366 |  883 |
| "                     | x86   | gcc -O2 -mssse3           |  312 |  581 |  120 |  466 |  605 | 1085 |  894 |


lessons learned
---------------
* If it's not an emberrassingly parallel algorithm, make it one (and then vectorize).

The DJBX33A algorithm can be implemented very fast but it is hard to vectorize.
The modiefied version which I call X4DJBX33A seems to be well suited for running
on 128bit or 64bit wide vector registers.
When it is not vectorized, it is slower than a well implemented vanilla DJBX33A algorithm.

* The SSSE3 version is an experiment.

The SSE2 version seems to be optimal and on fast machines it is only limited by the RAM bandwidth.
The SSSE3 version does not hit that limit on slower Intel Atom platforms and maxes out
on the CPU.

* Opcode scheduling is important, so use intrinsics.

Benchmarks with -O0 and -O1 builds have shown that even the MMX and SSE implementations
get noticeably slower when not optimized. A look at the disassembled binaries of the
-O2 and -O3 optimized builds shows that the compiler reorders instructions.
It probably does this to enhance the instruction level parallelism and provide the CPU
with useful instructions while it is still waiting for some RAM I/O to complete.
Using intrinsics instead of raw assembler code allows the developer to leverage the
wisdom of the compiler.

* The C implementation performance varies widely with the compiler.

MSVC and GCC seem to produce very different code from the C implementations.
This is not surprising as the research on auto-vectorization and codegen
is still ongoing.
The SSE2 version seems to be much more stable across compilers and platforms.


* Know your instruction set.

The author struggled quite a bit with the SSE2 instruction set and initially
failed to produce a vectorized version that was faster than the scalar one.
This was due to unsufficien knowledge of the instruction set. In the end
learning all the intrinsics and their performance characteristics is what
enables a developer to find a good solution.

* Alignment matters.

Besides the reference C implementations, the author initially produced optimized
(but still C-based) versions of DJBX33A and X4DJBX33A.
A major optimization was to hash the input with a simple implementation until an
alignment boundary of 16 bytes in the input memory block was reached.
Then the compiler gets a hint to assume that the pointer is aligned to a 16 byte boundary.
After the hint, an inner loop which hashes 16 byte chunks and an outer loop which iterates
the inner loop is run. This keeps the alignment assumption.
This assumption allows the compiler to use opcodes that rely on alignment and possibly
enables auto-vectorization.

* SSE2 is everywhere.

If you are on a 64bit X86 processor, you are guaranteed to have SSE2.
On 32bit X86, every processor sold in the last 10 years has SSE2.
From an economic point of view you can probably ignore non-SSE2 x86 CPUs or
just provide one C implementation.


