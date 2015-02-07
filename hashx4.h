#ifndef HASHX4_H
#define HASHX4_H
/* 
 * Copyright 2015 Kai Dietrich <mail@cleeus.de>
 *
 * This file is part of hashx4.
 *
 * Hashx4 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hashx4 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hashx4.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>

#define HX4_ERR_SUCCESS (0)
#define HX4_ERR_PARAM_INVALID (-1)
#define HX4_ERR_BUFFER_TOO_SMALL (-2)
#define HX4_ERR_OVERLAP (-3)

int hashx4_djb2_128_ref  (const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size);
int hashx4_djb2_128_copt (const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size);
int hashx4_djb2_128_sse2 (const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size);
int hashx4_djb2_128_ssse3(const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size);

int hashx4_djb2_128      (const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size);

#endif
