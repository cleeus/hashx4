#include <stdint.h>

#include "hashx4.h"
#include "hx4_util.h"

static int ptr_in_buffer(const void *ptr, const void *buffer, size_t buffer_size) {
  return  (ptr >= buffer) && ((const uint8_t*)ptr < ((const uint8_t*)buffer + buffer_size));
}

static int buffers_overlapping(const void *buffer1, size_t buffer1_size, const void *buffer2, size_t buffer2_size) {
  return ptr_in_buffer(buffer1, buffer2, buffer2_size) ||
    ptr_in_buffer((const uint8_t*)buffer1+buffer1_size-1, buffer2, buffer2_size) ||
    ptr_in_buffer(buffer2, buffer1, buffer1_size) ||
    ptr_in_buffer((const uint8_t*)buffer2+buffer2_size-1, buffer1, buffer1_size);
}

int hx4_check_params(size_t sizeof_state, const void *in, size_t in_sz, const void *cookie, size_t cookie_sz, void *out, size_t out_sz) {
  if(!in || !cookie || !out) {
    return HX4_ERR_PARAM_INVALID;
  }
  if(out_sz < sizeof_state) {
    return HX4_ERR_BUFFER_TOO_SMALL;
  }
  if(cookie_sz < 128/8) {
    return HX4_ERR_COOKIE_TOO_SMALL;
  }
  if(buffers_overlapping(in, in_sz, out, out_sz)) {
    return HX4_ERR_OVERLAP;
  }
  if(buffers_overlapping(in, in_sz, cookie, cookie_sz)) {
    return HX4_ERR_OVERLAP;
  }
  if(buffers_overlapping(out, out_sz, cookie, cookie_sz)) {
    return HX4_ERR_OVERLAP;
  }

  return HX4_ERR_SUCCESS;
}

int hx4_bytes_to_aligned(const void *ptr) {
  return ((size_t)ptr) % 16 == 0 ? 0 : 16 - (((size_t)ptr) % 16);
}

void hx4_xor_cookie_32(void *target, const void *cookie) {
  int i; 
  for(i=0; i<4; i++) {
    ((uint8_t*)target)[i] ^= ((uint8_t*)cookie)[i];
  }
}
void hx4_xor_cookie_128(void *target, const void *cookie) {
  int i;
  for(i=0; i<16; i++) {
    ((uint8_t*)target)[i] ^= ((uint8_t*)cookie)[i];
  }
}



