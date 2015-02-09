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

#include "hashx4_config.h"

#if HX4_HAS_SSE2
#include <emmintrin.h>
#endif

#if HX4_HAS_SSSE3
#include <tmmintrin.h>
#endif

#include "hashx4.h"
#include "hx4_util.h"


int hx4_djbx33a_32_ref(const void *buffer, size_t buffer_size, const void *cookie, size_t cookie_sz, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  uint32_t state = 5381;
  int rc;

  rc = hx4_check_params(sizeof(state), buffer, buffer_size, cookie, cookie_sz, out_hash, out_hash_size);
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

int hx4_djbx33a_32_copt(const void *buffer, size_t buffer_size, const void *cookie, size_t cookie_sz, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = hx4_bytes_to_aligned(buffer);
  uint32_t state = 5381;
  int rc;
  int i;

  rc = hx4_check_params(sizeof(state), buffer, buffer_size, cookie, cookie_sz, out_hash, out_hash_size);
  if(rc != HX4_ERR_SUCCESS) {
    return rc;
  }

  p = buffer;

  //hash input until p is aligned to alignment_target
  for(i=0; p<buffer_end && i<num_bytes_to_seek; i++) {
    state  = state  * 33  + *p;
    p++;
  }

  HX4_ASSUME_ALIGNED(p, 16)

  //main processing loop
  while(p+15<buffer_end) {
    HX4_ASSUME_ALIGNED(p, 16)

#if 1
#define HX4_DJBX33A_ROUND(round) \
    /* state = state * 33 + p[round]; */ \
    state = (state << 5) + state + p[round]; \

    HX4_DJBX33A_ROUND(0)
    HX4_DJBX33A_ROUND(1)
    HX4_DJBX33A_ROUND(2)
    HX4_DJBX33A_ROUND(3)
    HX4_DJBX33A_ROUND(4)
    HX4_DJBX33A_ROUND(5)
    HX4_DJBX33A_ROUND(6)
    HX4_DJBX33A_ROUND(7)
    HX4_DJBX33A_ROUND(8)
    HX4_DJBX33A_ROUND(9)
    HX4_DJBX33A_ROUND(10)
    HX4_DJBX33A_ROUND(11)
    HX4_DJBX33A_ROUND(12)
    HX4_DJBX33A_ROUND(13)
    HX4_DJBX33A_ROUND(14)
    HX4_DJBX33A_ROUND(15)
#endif

#if 0
    for(i=0; i<16; i++) {
      //state = state*33 + p[i];
      state = (state << 5) + state + p[i];
    }
#endif

    p+=16;
  }
#undef HX4_DJBX33A_ROUND

  //hash remainder
  while(p<buffer_end) {
    //state = state * 33  + *p;
    state = (state << 5) + state + p[i];
    p++;
  }

 
  memcpy(out_hash, &state, sizeof(state));

  return HX4_ERR_SUCCESS;
}


int hx4_x4djbx33a_128_ref(const void *buffer, size_t buffer_size, const void *cookie, size_t cookie_sz, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  uint32_t state[] = { 5381, 5381, 5381, 5381 };
  int state_i=0;
  int rc;

  rc = hx4_check_params(sizeof(state), buffer, buffer_size, cookie, cookie_sz, out_hash, out_hash_size);
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

int hx4_x4djbx33a_128_copt(const void *buffer, size_t buffer_size, const void *cookie, size_t cookie_sz, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = hx4_bytes_to_aligned(buffer);
  uint32_t state[] = { 5381, 5381, 5381, 5381 };
  uint32_t state_tmp;
  int state_i=0;
  int rc;
  int i;

  rc = hx4_check_params(sizeof(state), buffer, buffer_size, cookie, cookie_sz, out_hash, out_hash_size);
  if(rc != HX4_ERR_SUCCESS) {
    return rc;
  }

  p = buffer;

  //hash input until p is aligned to alignment_target
  for(i=0; p<buffer_end && i<num_bytes_to_seek; i++) {
    //state[state_i] = state[state_i] * 33  + *p;
    state[state_i] = (state[state_i] << 5) + state[state_i]  + *p;
    p++;
    state_i = (state_i+1) % 4;
  }

  HX4_ASSUME_ALIGNED(p, 16)

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
    HX4_ASSUME_ALIGNED(p, 16)

#define HX4_DJB2X4_COPT_ROUND(state_i, round) \
    state[state_i] = (state[state_i] << 5) + state[state_i] + p[round];

    HX4_DJB2X4_COPT_ROUND(0,0)
    HX4_DJB2X4_COPT_ROUND(1,1)
    HX4_DJB2X4_COPT_ROUND(2,2)
    HX4_DJB2X4_COPT_ROUND(3,3)
    HX4_DJB2X4_COPT_ROUND(0,4)
    HX4_DJB2X4_COPT_ROUND(1,5)
    HX4_DJB2X4_COPT_ROUND(2,6)
    HX4_DJB2X4_COPT_ROUND(3,7)
    HX4_DJB2X4_COPT_ROUND(0,8)
    HX4_DJB2X4_COPT_ROUND(1,9)
    HX4_DJB2X4_COPT_ROUND(2,10)
    HX4_DJB2X4_COPT_ROUND(3,11)
    HX4_DJB2X4_COPT_ROUND(0,12)
    HX4_DJB2X4_COPT_ROUND(1,13)
    HX4_DJB2X4_COPT_ROUND(2,14)
    HX4_DJB2X4_COPT_ROUND(3,15)

    p+=16;
  }
  
#undef HX4_DJB2X4_COPT_ROUND

  //rotate back the states
  for(i=0; i<state_i; i++) {
    state_tmp = state[3];
    state[3] = state[2];
    state[2] = state[1];
    state[1] = state[0];
    state[0] = state_tmp;
  }

  //process remainder
  while(p<buffer_end) {
    //state[state_i] = state[state_i] * 33  + *p;
    state[state_i] = (state[state_i] << 5) + state[state_i]  + *p;
    p++;
    state_i = (state_i+1) % 4;
  }

  memcpy(out_hash, state, sizeof(state));

  return HX4_ERR_SUCCESS;
}

#if HX4_HAS_SSE2
int hx4_x4djbx33a_128_sse2(const void *buffer, size_t buffer_size, const void *cookie, size_t cookie_sz, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = hx4_bytes_to_aligned(buffer);

  HX4_ALIGNED(uint32_t state[], 16) = { 5381, 5381, 5381, 5381 };

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


  rc = hx4_check_params(sizeof(state), buffer, buffer_size, cookie, cookie_sz, out_hash, out_hash_size);
  if(rc != HX4_ERR_SUCCESS) {
    return rc;
  }

  p = buffer;

  //hash input until p is aligned to alignment_target
  for(i=0; p<buffer_end && i<num_bytes_to_seek; i++) {
    //state[state_i] = state[state_i] * 33  + *p;
    state[state_i] = (state[state_i] << 5) + state[state_i]  + *p;
    p++;
    state_i = (state_i+1) % 4;
  }

  HX4_ASSUME_ALIGNED(p, 16)

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
    HX4_ASSUME_ALIGNED(p, 16)

#if 0
    //load 16 bytes aligned
    xpin = _mm_load_si128((__m128i*)p);
#define HX4_SSE2_DJB2ROUND(round) \
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
#define HX4_SSE2_DJB2ROUND(round) \
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
#define HX4_SSE2_DJB2ROUND(round) \
    /* load 4 bytes, expand into xp */ \
    xp = _mm_set_epi32(p[round*4+3], p[round*4+2], p[round*4+1], p[round*4+0]); \
    xp = _mm_add_epi32(xp, xstate); \
    xstate = _mm_slli_epi32(xstate, 5); \
    xstate = _mm_add_epi32(xstate, xp);
#endif
    
    HX4_SSE2_DJB2ROUND(0)
    HX4_SSE2_DJB2ROUND(1)
    HX4_SSE2_DJB2ROUND(2)
    HX4_SSE2_DJB2ROUND(3)
    
    p+=16;
  }
#undef HX4_SSE2_DJB2ROUND

  //store back state from register into memory
  _mm_store_si128((__m128i*)state, xstate);
  
  //rotate back the states
  for(i=0; i<state_i; i++) {
    state_tmp = state[3];
    state[3] = state[2];
    state[2] = state[1];
    state[1] = state[0];
    state[0] = state_tmp;
  }

  //process any input that is left
  while(p<buffer_end) {
    //state[state_i] = state[state_i] * 33  + *p;
    state[state_i] = (state[state_i] << 5) + state[state_i]  + *p;
    p++;
    state_i = (state_i+1) % 4;
  }

  memcpy(out_hash, state, sizeof(state));

  return HX4_ERR_SUCCESS;
}
#endif //HX4_HAS_SSE2

#if HX4_HAS_SSSE3
int hx4_x4djbx33a_128_ssse3(const void *buffer, size_t buffer_size, const void *cookie, size_t cookie_sz, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = hx4_bytes_to_aligned(buffer);
  HX4_ALIGNED(uint32_t state[], 16) = { 5381, 5381, 5381, 5381 };
  uint32_t state_tmp;
  __m128i xstate;
  __m128i xp;
  __m128i xpin;
  __m128i xbmask;
  __m128i xshuffle;
  int state_i = 0;
  int rc;
  int i;

  rc = hx4_check_params(sizeof(state), buffer, buffer_size, cookie, cookie_sz, out_hash, out_hash_size);
  if (rc != HX4_ERR_SUCCESS) {
    return rc;
  }

  p = buffer;

  //hash input until p is aligned to alignment_target
  for (i = 0; p<buffer_end && i<num_bytes_to_seek; i++) {
    //state[state_i] = state[state_i] * 33 + *p;
    state[state_i] = (state[state_i] << 5) + state[state_i] + *p;
    p++;
    state_i = (state_i + 1) % 4;
  }

  HX4_ASSUME_ALIGNED(p, 16)

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
    HX4_ASSUME_ALIGNED(p, 16)

    //load 16 bytes aligned
    xpin = _mm_load_si128((__m128i*)p);
    xpin = _mm_shuffle_epi8(xpin, xshuffle);

#define HX4_SSSE3_DJB2ROUND(round) \
    xp = _mm_srli_epi32(xpin, 8*round); \
    xp = _mm_and_si128(xp, xbmask); \
    xp = _mm_add_epi32(xp, xstate); \
    xstate = _mm_slli_epi32(xstate, 5); \
    xstate = _mm_add_epi32(xstate, xp); \

    HX4_SSSE3_DJB2ROUND(0)
    HX4_SSSE3_DJB2ROUND(1)
    HX4_SSSE3_DJB2ROUND(2)
    HX4_SSSE3_DJB2ROUND(3)

    p += 16;
  }
#undef HX4_SSSE3_DJB2ROUND

  //store back state from register into memory
  _mm_store_si128((__m128i*)state, xstate);

  //rotate back the states
  for (i = 0; i<state_i; i++) {
    state_tmp = state[3];
    state[3] = state[2];
    state[2] = state[1];
    state[1] = state[0];
    state[0] = state_tmp;
  }

  //process any input that is left
  while (p<buffer_end) {
    //state[state_i] = state[state_i] * 33 + *p;
    state[state_i] = (state[state_i] << 5) + state[state_i] + *p;   
    p++;
    state_i = (state_i + 1) % 4;
  }

  memcpy(out_hash, state, sizeof(state));

  return HX4_ERR_SUCCESS;
}
#endif //HX4_HAS_SSSE3

