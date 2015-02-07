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
#include <stdint.h>
#include <string.h>

#include <emmintrin.h>
#include <tmmintrin.h>

#include "hashx4.h"

static int ptr_in_buffer(const void *ptr, const void *buffer, size_t buffer_size) {
  return  (ptr >= buffer) && ((const uint8_t*)ptr < ((const uint8_t*)buffer + buffer_size));
}

static int buffers_overlapping(const void *buffer1, size_t buffer1_size, const void *buffer2, size_t buffer2_size) {
  return ptr_in_buffer(buffer1, buffer2, buffer2_size) ||
    ptr_in_buffer((const uint8_t*)buffer1+buffer1_size-1, buffer2, buffer2_size) ||
    ptr_in_buffer(buffer2, buffer1, buffer1_size) ||
    ptr_in_buffer((const uint8_t*)buffer2+buffer2_size-1, buffer1, buffer1_size);
}

int check_params(size_t sizeof_state, const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size) {
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

static int num_bytes_to_alignment(const void *ptr) {
  return ((size_t)ptr) % 16 == 0 ? 0 : 16 - (((size_t)ptr) % 16);
}


int hashx4_djbx33a_32_ref(const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  uint32_t state = 5381;
  int rc;

  rc = check_params(sizeof(state), buffer, buffer_size, out_hash, out_hash_size);
  if(rc != HX4_ERR_SUCCESS) {
    return rc;
  }
  
  p = buffer;
  while(p<buffer_end) {
    state = state * 33  + *p;
    p++;
  }

  memcpy(out_hash, &state, sizeof(state));

  return HX4_ERR_SUCCESS;
}

int hashx4_djbx33a_32_copt(const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = num_bytes_to_alignment(buffer);
  uint32_t state = 5381;
  int rc;
  int i;

  rc = check_params(sizeof(state), buffer, buffer_size, out_hash, out_hash_size);
  if(rc != HX4_ERR_SUCCESS) {
    return rc;
  }

  p = buffer;

  //hash input until p is aligned to alignment_target
  for(i=0; p<buffer_end && i<num_bytes_to_seek; i++) {
    state  = state  * 33  + *p;
    p++;
  }

#ifdef __GNUC__
  p = __builtin_assume_aligned(p, 16);
#elif _MSC_VER
  __assume((size_t)p % 16 == 0);
#endif

  //main processing loop
  while(p+15<buffer_end) {
#ifdef __GNUC__
    p = __builtin_assume_aligned(p, 16);
#elif _MSC_VER
    __assume((size_t)p % 16 == 0);
#endif
    for(i=0; i<16;i++) {
    	state = state * 33 + p[i];
    }

    p+=16;
  }
  
  memcpy(out_hash, &state, sizeof(state));

  return HX4_ERR_SUCCESS;
}


int hashx4_djbx33ax4_128_ref(const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  uint32_t state[] = { 5381, 5381, 5381, 5381 };
  int state_i=0;
  int rc;

  rc = check_params(sizeof(state), buffer, buffer_size, out_hash, out_hash_size);
  if(rc != HX4_ERR_SUCCESS) {
    return rc;
  }
  
  p = buffer;
  while(p<buffer_end) {
    state[state_i] = state[state_i] * 33  + *p;
    p++;
    state_i = (state_i+1) % 4;
  }

  memcpy(out_hash, state, sizeof(state));

  return HX4_ERR_SUCCESS;
}

int hashx4_djbx33ax4_128_copt(const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = num_bytes_to_alignment(buffer);
  uint32_t state[] = { 5381, 5381, 5381, 5381 };
  uint32_t state_tmp;
  int state_i=0;
  int rc;
  int i;

  rc = check_params(sizeof(state), buffer, buffer_size, out_hash, out_hash_size);
  if(rc != HX4_ERR_SUCCESS) {
    return rc;
  }

  p = buffer;

  //hash input until p is aligned to alignment_target
  for(i=0; p<buffer_end && i<num_bytes_to_seek; i++) {
    state[state_i] = state[state_i] * 33  + *p;
    p++;
    state_i = (state_i+1) % 4;
  }

#ifdef __GNUC__
  p = __builtin_assume_aligned(p, 16);
#elif _MSC_VER
  __assume((size_t)p % 16 == 0);
#endif

  //rotate states to match position on the input stream
  //so that the main loop can be simple
  for(i=0; i<state_i; i++) {
    state_tmp = state[0];
    state[0] = state[1];
    state[1] = state[2];
    state[2] = state[3];
    state[3] = state_tmp;
  }

  //main processing loop
  while(p+15<buffer_end) {
#ifdef __GNUC__
    p = __builtin_assume_aligned(p, 16);
#elif _MSC_VER
    __assume((size_t)p % 16 == 0);
#endif

    state[0] = state[0] * 33 + p[0];
    state[1] = state[1] * 33 + p[1];
    state[2] = state[2] * 33 + p[2];
    state[3] = state[3] * 33 + p[3];
    
    state[0] = state[0] * 33 + p[4];
    state[1] = state[1] * 33 + p[5];
    state[2] = state[2] * 33 + p[6];
    state[3] = state[3] * 33 + p[7];
    
    state[0] = state[0] * 33 + p[8];
    state[1] = state[1] * 33 + p[9];
    state[2] = state[2] * 33 + p[10];
    state[3] = state[3] * 33 + p[11];
    
    state[0] = state[0] * 33 + p[12];
    state[1] = state[1] * 33 + p[13];
    state[2] = state[2] * 33 + p[14];
    state[3] = state[3] * 33 + p[15];
    
    p+=16;
  }
  
  //rotate back the states so that the result
  for(i=0; i<state_i; i++) {
    state_tmp = state[3];
    state[3] = state[2];
    state[2] = state[1];
    state[1] = state[0];
    state[0] = state_tmp;
  }

  //process any input that is left
  while(p<buffer_end) {
    state[state_i] = state[state_i] * 33  + *p;
    p++;
    state_i = (state_i+1) % 4;
  }

  memcpy(out_hash, state, sizeof(state));

  return HX4_ERR_SUCCESS;
}

int hashx4_djbx33ax4_128_sse2(const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = num_bytes_to_alignment(buffer);

#ifdef __GNUC__
  uint32_t state[] __attribute__((aligned(16))) = { 5381, 5381, 5381, 5381 };
#elif _MSC_VER
  _declspec(align(16)) uint32_t state[] = { 5381, 5381, 5381, 5381 };
#endif

  uint32_t state_tmp;
  __m128i xstate;
  __m128i xp;
  int state_i = 0;
  int rc;
  int i;
#if 0
  __m128i xpin;
  __m128i xtmp;
  int tmp;
#endif


  rc = check_params(sizeof(state), buffer, buffer_size, out_hash, out_hash_size);
  if(rc != HX4_ERR_SUCCESS) {
    return rc;
  }

  p = buffer;

  //hash input until p is aligned to alignment_target
  for(i=0; p<buffer_end && i<num_bytes_to_seek; i++) {
    state[state_i] = state[state_i] * 33  + *p;
    p++;
    state_i = (state_i+1) % 4;
  }

#ifdef __GNUC__
  p = __builtin_assume_aligned(p, 16);
#elif _MSC_VER
  __assume((size_t)p % 16 == 0);
#endif

  //rotate states to match position on the input stream
  //so that the main loop can be simple
  for(i=0; i<state_i; i++) {
    state_tmp = state[0];
    state[0] = state[1];
    state[1] = state[2];
    state[2] = state[3];
    state[3] = state_tmp;
  }
  
  //transfer state into register
  xstate = _mm_load_si128((__m128i*)state);

  //main processing loop
  while(p+15<buffer_end) {
#ifdef __GNUC__
    p = __builtin_assume_aligned(p, 16);
#elif _MSC_VER
    __assume((size_t)p % 16 == 0);
#endif

#if 0
    //load 16 bytes aligned
    xpin = _mm_load_si128((__m128i*)p);
#define H4X_SSE2_DJB2ROUND(round) \
    /* unpack 4 bytes per round */ \
    tmp = _mm_cvtsi128_si32(xpin); \
    xp = _mm_set_epi32( \
      (uint8_t)(tmp >> 8*3), \
      (uint8_t)(tmp >> 8*2), \
      (uint8_t)(tmp >> 8*1), \
      (uint8_t)(tmp >> 8*0)  \
    ); \
    /* advance input by 4 bytes */ \
    xpin = _mm_srli_si128(xpin, 4); \
    /* multiply by shift << 5 and add */ \
    xtmp = _mm_slli_epi32(xstate, 5); \
    xstate = _mm_add_epi32(xtmp, xstate); \
    /* add input */ \
    xstate = _mm_add_epi32(xstate, xp);
#endif

#if 0
    //load 16 bytes aligned
    xpin = _mm_load_si128((__m128i*)p);
#define H4X_SSE2_DJB2ROUND(round) \
    /* unpack 4 bytes per round */ \
    tmp = _mm_cvtsi128_si32(xpin); \
    xp = _mm_set_epi32( \
      (uint8_t)(tmp >> 8*3), \
      (uint8_t)(tmp >> 8*2), \
      (uint8_t)(tmp >> 8*1), \
      (uint8_t)(tmp >> 8*0)  \
    ); \
    xpin = _mm_srli_si128(xpin, 4); \
    xp = _mm_add_epi32(xp, xstate); \
    xstate = _mm_slli_epi32(xstate, 5); \
    xstate = _mm_add_epi32(xstate, xp);
#endif


#if 1
#define H4X_SSE2_DJB2ROUND(round) \
    /* load 4 bytes, expand into xp */ \
    xp = _mm_set_epi32(p[round*4+3], p[round*4+2], p[round*4+1], p[round*4+0]); \
    xp = _mm_add_epi32(xp, xstate); \
    xstate = _mm_slli_epi32(xstate, 5); \
    xstate = _mm_add_epi32(xstate, xp);
#endif
    
    H4X_SSE2_DJB2ROUND(0)
    H4X_SSE2_DJB2ROUND(1)
    H4X_SSE2_DJB2ROUND(2)
    H4X_SSE2_DJB2ROUND(3)
    
    p+=16;
  }
  //store back state from register into memory
  _mm_store_si128((__m128i*)state, xstate);
  
  //rotate back the states so that the result
  for(i=0; i<state_i; i++) {
    state_tmp = state[3];
    state[3] = state[2];
    state[2] = state[1];
    state[1] = state[0];
    state[0] = state_tmp;
  }

  //process any input that is left
  while(p<buffer_end) {
    state[state_i] = state[state_i] * 33  + *p;
    p++;
    state_i = (state_i+1) % 4;
  }

  memcpy(out_hash, state, sizeof(state));

  return HX4_ERR_SUCCESS;
}

int hashx4_djbx33ax4_128_ssse3(const void *buffer, size_t buffer_size, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = num_bytes_to_alignment(buffer);

#ifdef __GNUC__
  uint32_t state[] __attribute__((aligned(16))) = { 5381, 5381, 5381, 5381 };
#elif _MSC_VER
  _declspec(align(16)) uint32_t state[] = { 5381, 5381, 5381, 5381 };
#endif
  uint32_t state_tmp;
  __m128i xstate;
  __m128i xp;
  __m128i xpin;
  __m128i xbmask;
  __m128i xshuffle;
  int state_i = 0;
  int rc;
  int i;

  rc = check_params(sizeof(state), buffer, buffer_size, out_hash, out_hash_size);
  if (rc != HX4_ERR_SUCCESS) {
    return rc;
  }

  p = buffer;

  //hash input until p is aligned to alignment_target
  for (i = 0; p<buffer_end && i<num_bytes_to_seek; i++) {
    state[state_i] = state[state_i] * 33 + *p;
    p++;
    state_i = (state_i + 1) % 4;
  }

#ifdef __GNUC__
  p = __builtin_assume_aligned(p, 16);
#elif _MSC_VER
  __assume((size_t)p % 16 == 0);
#endif

  //rotate states to match position on the input stream
  //so that the main loop can be simple
  for (i = 0; i<state_i; i++) {
    state_tmp = state[0];
    state[0] = state[1];
    state[1] = state[2];
    state[2] = state[3];
    state[3] = state_tmp;
  }

  //transfer state into register
  xstate = _mm_load_si128((__m128i*)state);
  //load AND mask
  xbmask = _mm_set1_epi32(0x000000ff);
  //load shuffle mask
  xshuffle = _mm_set_epi8(
    15, 11, 7, 3,
    14, 10, 6, 2,
    13, 9 , 5, 1,
    12, 8 , 4, 0
  );

  //main processing loop
  while (p + 15<buffer_end) {
#ifdef __GNUC__
    p = __builtin_assume_aligned(p, 16);
#elif _MSC_VER
    __assume((size_t)p % 16 == 0);
#endif

    //load 16 bytes aligned
    xpin = _mm_load_si128((__m128i*)p);
    xpin = _mm_shuffle_epi8(xpin, xshuffle);

#define H4X_SSSE3_DJB2ROUND(round) \
    xp = _mm_srli_epi32(xpin, 8*round); \
    xp = _mm_and_si128(xp, xbmask); \
    xp = _mm_add_epi32(xp, xstate); \
    xstate = _mm_slli_epi32(xstate, 5); \
    xstate = _mm_add_epi32(xstate, xp); \

    H4X_SSSE3_DJB2ROUND(0)
    H4X_SSSE3_DJB2ROUND(1)
    H4X_SSSE3_DJB2ROUND(2)
    H4X_SSSE3_DJB2ROUND(3)

    p += 16;
  }
  //store back state from register into memory
  _mm_store_si128((__m128i*)state, xstate);

  //rotate back the states so that the result
  for (i = 0; i<state_i; i++) {
    state_tmp = state[3];
    state[3] = state[2];
    state[2] = state[1];
    state[1] = state[0];
    state[0] = state_tmp;
  }

  //process any input that is left
  while (p<buffer_end) {
    state[state_i] = state[state_i] * 33 + *p;
    p++;
    state_i = (state_i + 1) % 4;
  }

  memcpy(out_hash, state, sizeof(state));

  return HX4_ERR_SUCCESS;
}

