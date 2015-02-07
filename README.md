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

