#ifndef HASHX4_H
#define HASHX4_H

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
