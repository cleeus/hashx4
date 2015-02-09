#include <stdint.h>

#include "hashx4.h"
#include "hashx4_util.h"

static int ptr_in_buffer(const void *ptr, const void *buffer, size_t buffer_size) {
  return  (ptr >= buffer) && ((const uint8_t*)ptr < ((const uint8_t*)buffer + buffer_size));
}

static int buffers_overlapping(const void *buffer1, size_t buffer1_size, const void *buffer2, size_t buffer2_size) {
  return ptr_in_buffer(buffer1, buffer2, buffer2_size) ||
    ptr_in_buffer((const uint8_t*)buffer1+buffer1_size-1, buffer2, buffer2_size) ||
    ptr_in_buffer(buffer2, buffer1, buffer1_size) ||
    ptr_in_buffer((const uint8_t*)buffer2+buffer2_size-1, buffer1, buffer1_size);
}

int hx4_check_params(size_t sizeof_state, const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size) {
  if(!buffer || !out_hash) {
    return HX4_ERR_PARAM_INVALID;
  }
  if(out_hash_size < sizeof_state) {
    return HX4_ERR_BUFFER_TOO_SMALL;
  }
  if(buffers_overlapping(buffer, buffer_size, out_hash, out_hash_size)) {
    return HX4_ERR_OVERLAP;
  }

  return HX4_ERR_SUCCESS;
}

int hx4_bytes_to_aligned(const void *ptr) {
  return ((size_t)ptr) % 16 == 0 ? 0 : 16 - (((size_t)ptr) % 16);
}


