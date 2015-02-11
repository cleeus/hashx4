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

#if HX4_HAS_MMX
# include <mmintrin.h>
#endif

#if HX4_HAS_SSE2
# include <emmintrin.h>
#endif

#if HX4_HAS_SSSE3
# include <tmmintrin.h>
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

  hx4_xor_cookie_32(&state, cookie);
  memcpy(out_hash, &state, sizeof(state));
  return HX4_ERR_SUCCESS;
}

int hx4_djbx33a_32_copt(const void *buffer, size_t buffer_size, const void *cookie, size_t cookie_sz, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = hx4_bytes_to_aligned(buffer, 16);
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
    state  = (state << 5) + state  + *p;
    p++;
  }

  HX4_ASSUME_ALIGNED(p, 16)

  //main processing loop
  while(p+15<buffer_end) {
    HX4_ASSUME_ALIGNED(p, 16)

#if 1
#define HX4_DJBX33A_ROUND(round) \
    /* state = state * 33 + p[round]; */ \
    state = (state << 5) + state + p[round];

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
    state = (state << 5) + state + *p;
    p++;
  }

 
  hx4_xor_cookie_32(&state, cookie);
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

  hx4_xor_cookie_128(state, cookie);
  memcpy(out_hash, state, sizeof(state));

  return HX4_ERR_SUCCESS;
}

int hx4_x4djbx33a_128_copt(const void *buffer, size_t buffer_size, const void *cookie, size_t cookie_sz, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = hx4_bytes_to_aligned(buffer, 16);
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
    state_i = (state_i+1) & 0x03;
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
    state_i = (state_i+1) & 0x03;
  }

  hx4_xor_cookie_128(state, cookie);
  memcpy(out_hash, state, sizeof(state));

  return HX4_ERR_SUCCESS;
}

#if HX4_HAS_MMX
int hx4_x4djbx33a_128_mmx(const void *buffer, size_t buffer_size, const void *cookie, size_t cookie_sz, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = hx4_bytes_to_aligned(buffer, 8);
  HX4_ALIGNED(uint32_t state[], 8) = { 5381, 5381, 5381, 5381 };
  uint32_t state_tmp;
  int state_i=0;
  int rc;
  int i;
  __m64 xstate0;
  __m64 xstate1;
  __m64 xpin;
  __m64 xtmp;
  __m64 xp0;
  __m64 xp1;
  __m64 xmask0;
  __m64 xmask1;

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
    state_i = (state_i+1) & 0x03;
  }

  HX4_ASSUME_ALIGNED(p, 8)

  //rotate states to match position on the input stream
  //so that the main loop can be simple
  for(i=0; i<state_i; i++) {
    state_tmp = state[0];
    state[0] = state[1];
    state[1] = state[2];
    state[2] = state[3];
    state[3] = state_tmp;
  }

  //load state into registers
  xstate0 = ((__m64*)state)[0];
  xstate1 = ((__m64*)state)[1];
  //load AND masks
  xmask0 = _mm_set_pi32(0, 0x00ff);
  xmask1 = _mm_set_pi32(0x00ff, 0);

  //main processing loop
  while(p+8<buffer_end) {
    HX4_ASSUME_ALIGNED(p, 8)

    /*load 8 bytes into xpin */
    xpin = *((__m64*)(p));

    //dword0,byte0
    //no shift needed for this one
    xp0 = _mm_and_si64(xpin, xmask0);
    //dword0,byte1
    xtmp = _mm_slli_si64(xpin, 3*8);
    xtmp = _mm_and_si64(xtmp, xmask1);
    xp0 = _mm_or_si64(xp0, xtmp);
    //dword0,byte2
    xp1 = _mm_srli_si64(xpin, 2*8);
    xp1 = _mm_and_si64(xp1, xmask0);
    //dword0,byte3
    xtmp = _mm_slli_si64(xpin, 1*8);
    xtmp = _mm_and_si64(xtmp, xmask1);
    xp1 = _mm_or_si64(xp1, xtmp);
    //xstate = xstate * 33 + xp
    xp0 = _mm_add_pi32(xstate0, xp0);
    xp1 = _mm_add_pi32(xstate1, xp1);
    xstate0 = _mm_slli_pi32(xstate0, 5);
    xstate1 = _mm_slli_pi32(xstate1, 5);
    xstate0 = _mm_add_pi32(xstate0, xp0);
    xstate1 = _mm_add_pi32(xstate1, xp1);

    //dword1,byte0
    xp0 = _mm_srli_si64(xpin, 4*8);
    xp0 = _mm_and_si64(xp0, xmask0);
    //dword1,byte1
    xtmp = _mm_srli_si64(xpin, 1*8);
    xtmp = _mm_and_si64(xtmp, xmask1);
    xp0 = _mm_or_si64(xp0, xtmp);
    //dword1,byte2
    xp1 = _mm_srli_si64(xpin, 6*8);
    xp1 = _mm_and_si64(xp1, xmask0);
    //dword1,byte3
    xtmp = _mm_srli_si64(xpin, 3*8);
    xtmp = _mm_and_si64(xtmp, xmask1);
    xp1 = _mm_or_si64(xp1, xtmp);
    //xstate = xstate * 33 + xp
    xp0 = _mm_add_pi32(xstate0, xp0);
    xp1 = _mm_add_pi32(xstate1, xp1);
    xstate0 = _mm_slli_pi32(xstate0, 5);
    xstate1 = _mm_slli_pi32(xstate1, 5);
    xstate0 = _mm_add_pi32(xstate0, xp0);
    xstate1 = _mm_add_pi32(xstate1, xp1);

    p+=8;
  }

  //store registers back into state
  ((__m64*)state)[0] = xstate0;
  ((__m64*)state)[1] = xstate1;

  //reset MMX
  _mm_empty();

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
    state_i = (state_i+1) & 0x03;
  }

  hx4_xor_cookie_128(state, cookie);
  memcpy(out_hash, state, sizeof(state));

  return HX4_ERR_SUCCESS;
}
#endif //HX4_HAS_MMX

#if HX4_HAS_SSE2
int hx4_x4djbx33a_128_sse2(const void *buffer, size_t buffer_size, const void *cookie, size_t cookie_sz, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = hx4_bytes_to_aligned(buffer, 16);

  HX4_ALIGNED(uint32_t state[], 16) = { 5381, 5381, 5381, 5381 };

  uint32_t state_tmp;
  __m128i xstate;
  __m128i xp;
  __m128i xpin;
  __m128i xtmp;
  __m128i xmask0;
  __m128i xmask1;
  __m128i xmask2;
  __m128i xmask3;
  int state_i = 0;
  int rc;
  int i;
  //int tmp;

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
    state_i = (state_i+1) & 0x03;
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

  //load masks
  xmask0 = _mm_set_epi32(0, 0, 0, 0x00ff);
  xmask1 = _mm_set_epi32(0, 0, 0x00ff, 0);
  xmask2 = _mm_set_epi32(0, 0x00ff, 0, 0);
  xmask3 = _mm_set_epi32(0x00ff, 0, 0, 0);

 //main processing loop
  while(p+15<buffer_end) {
    HX4_ASSUME_ALIGNED(p, 16)

#if 1
    //load 16 bytes aligned
    xpin = _mm_load_si128((__m128i*)p);

    //dword0,byte0
    //no shift needed for this one
    xp = _mm_and_si128(xpin, xmask0);
    //dword0,byte1
    xtmp = _mm_slli_si128(xpin, 3);
    xtmp = _mm_and_si128(xtmp, xmask1);
    xp = _mm_or_si128(xp, xtmp);
    //dword0,byte2
    xtmp = _mm_slli_si128(xpin, 6);
    xtmp = _mm_and_si128(xtmp, xmask2);
    xp = _mm_or_si128(xp, xtmp);
    //dword0,byte3
    xtmp = _mm_slli_si128(xpin, 9);
    xtmp = _mm_and_si128(xtmp, xmask3);
    xp = _mm_or_si128(xp, xtmp);
    //xstate = xstate * 33 + xp
    xtmp = _mm_slli_epi32(xstate, 5);
    xstate = _mm_add_epi32(xstate, xtmp);
    xstate = _mm_add_epi32(xstate, xp);

    //dword1,byte0
    xp = _mm_srli_si128(xpin, 4);
    xp = _mm_and_si128(xp, xmask0);
    //dword1,byte1
    xtmp = _mm_srli_si128(xpin, 1);
    xtmp = _mm_and_si128(xtmp, xmask1);
    xp = _mm_or_si128(xp, xtmp);
    //dword1,byte2
    xtmp = _mm_slli_si128(xpin, 2);
    xtmp = _mm_and_si128(xtmp, xmask2);
    xp = _mm_or_si128(xp, xtmp);
    //dword1,byte3
    xtmp = _mm_slli_si128(xpin, 5);
    xtmp = _mm_and_si128(xtmp, xmask3);
    xp = _mm_or_si128(xp, xtmp);
    //xstate = xstate * 33 + xp
    xtmp = _mm_slli_epi32(xstate, 5);
    xstate = _mm_add_epi32(xstate, xtmp);
    xstate = _mm_add_epi32(xstate, xp);

    //dword2,byte0
    xp = _mm_srli_si128(xpin, 8);
    xp = _mm_and_si128(xp, xmask0);
    //dword2,byte1
    xtmp = _mm_srli_si128(xpin, 5);
    xtmp = _mm_and_si128(xtmp, xmask1);
    xp = _mm_or_si128(xp, xtmp);
    //dword2,byte2
    xtmp = _mm_srli_si128(xpin, 2);
    xtmp = _mm_and_si128(xtmp, xmask2);
    xp = _mm_or_si128(xp, xtmp);
    //dword2,byte3
    xtmp = _mm_slli_si128(xpin, 1);
    xtmp = _mm_and_si128(xtmp, xmask3);
    xp = _mm_or_si128(xp, xtmp);
    //xstate = xstate * 33 + xp
    xtmp = _mm_slli_epi32(xstate, 5);
    xstate = _mm_add_epi32(xstate, xtmp);
    xstate = _mm_add_epi32(xstate, xp);

    //dword3,byte0
    xp = _mm_srli_si128(xpin, 12);
    xp = _mm_and_si128(xp, xmask0);
    //dword3,byte1
    xtmp = _mm_srli_si128(xpin, 9);
    xtmp = _mm_and_si128(xtmp, xmask1);
    xp = _mm_or_si128(xp, xtmp);
    //dword3,byte2
    xtmp = _mm_srli_si128(xpin, 6);
    xtmp = _mm_and_si128(xtmp, xmask2);
    xp = _mm_or_si128(xp, xtmp);
    //dword3,byte3
    xtmp = _mm_srli_si128(xpin, 3);
    xtmp = _mm_and_si128(xtmp, xmask3);
    xp = _mm_or_si128(xp, xtmp);
    //xstate = xstate * 33 + xp
    xtmp = _mm_slli_epi32(xstate, 5);
    xstate = _mm_add_epi32(xstate, xtmp);
    xstate = _mm_add_epi32(xstate, xp);

#endif

#if 0
    //load 16 bytes aligned
    xpin = _mm_load_si128((__m128i*)p);

    //xstate = xstate * 33
    xtmp = _mm_slli_epi32(xstate, 5);
    xstate = _mm_add_epi32(xstate, xtmp);
    //now add input bytes, one by one
    //dword0,byte0
    //no shift needed for this one
    xtmp = _mm_and_si128(xpin, xmask0);
    xstate = _mm_add_epi32(xstate, xtmp);
    //dword0,byte1
    xtmp = _mm_slli_si128(xpin, 3);
    xtmp = _mm_and_si128(xtmp, xmask1);
    xstate = _mm_add_epi32(xstate, xtmp);
    //dword0,byte2
    xtmp = _mm_slli_si128(xpin, 6);
    xtmp = _mm_and_si128(xtmp, xmask2);
    xstate = _mm_add_epi32(xstate, xtmp);
    //dword0,byte3
    xtmp = _mm_slli_si128(xpin, 9);
    xtmp = _mm_and_si128(xtmp, xmask3);
    xstate = _mm_add_epi32(xstate, xtmp);

    //xstate = xstate * 33
    xtmp = _mm_slli_epi32(xstate, 5);
    xstate = _mm_add_epi32(xstate, xtmp);
    //now add input bytes, one by one
    //dword1,byte0
    xtmp = _mm_srli_si128(xpin, 4);
    xtmp = _mm_and_si128(xtmp, xmask0);
    xstate = _mm_add_epi32(xstate, xtmp);
    //dword1,byte1
    xtmp = _mm_srli_si128(xpin, 1);
    xtmp = _mm_and_si128(xtmp, xmask1);
    xstate = _mm_add_epi32(xstate, xtmp);
    //dword1,byte2
    xtmp = _mm_slli_si128(xpin, 2);
    xtmp = _mm_and_si128(xtmp, xmask2);
    xstate = _mm_add_epi32(xstate, xtmp);
    //dword1,byte3
    xtmp = _mm_slli_si128(xpin, 5);
    xtmp = _mm_and_si128(xtmp, xmask3);
    xstate = _mm_add_epi32(xstate, xtmp);

    //xstate = xstate * 33
    xtmp = _mm_slli_epi32(xstate, 5);
    xstate = _mm_add_epi32(xstate, xtmp);
    //now add input bytes, one by one
    //dword2,byte0
    xtmp = _mm_srli_si128(xpin, 8);
    xtmp = _mm_and_si128(xtmp, xmask0);
    xstate = _mm_add_epi32(xstate, xtmp);
    //dword2,byte1
    xtmp = _mm_srli_si128(xpin, 5);
    xtmp = _mm_and_si128(xtmp, xmask1);
    xstate = _mm_add_epi32(xstate, xtmp);
    //dword2,byte2
    xtmp = _mm_srli_si128(xpin, 2);
    xtmp = _mm_and_si128(xtmp, xmask2);
    xstate = _mm_add_epi32(xstate, xtmp);
    //dword2,byte3
    xtmp = _mm_slli_si128(xpin, 1);
    xtmp = _mm_and_si128(xtmp, xmask3);
    xstate = _mm_add_epi32(xstate, xtmp);

    //xstate = xstate * 33
    xtmp = _mm_slli_epi32(xstate, 5);
    xstate = _mm_add_epi32(xstate, xtmp);
    //now add input bytes, one by one
    //dword3,byte0
    xtmp = _mm_srli_si128(xpin, 12);
    xtmp = _mm_and_si128(xtmp, xmask0);
    xstate = _mm_add_epi32(xstate, xtmp);
    //dword3,byte1
    xtmp = _mm_srli_si128(xpin, 9);
    xtmp = _mm_and_si128(xtmp, xmask1);
    xstate = _mm_add_epi32(xstate, xtmp);
    //dword3,byte2
    xtmp = _mm_srli_si128(xpin, 6);
    xtmp = _mm_and_si128(xtmp, xmask2);
    xstate = _mm_add_epi32(xstate, xtmp);
    //dword3,byte3
    xtmp = _mm_srli_si128(xpin, 3);
    xtmp = _mm_and_si128(xtmp, xmask3);
    xstate = _mm_add_epi32(xstate, xtmp);
#endif

#if 0
#define _mm_extract_epi8(x, imm) \
    ((((imm) & 0x1) == 0) ? \
    _mm_extract_epi16((x), (imm) >> 1) & 0xff : \
    _mm_extract_epi16(_mm_srli_epi16((x), 8), (imm) >> 1))

    //load 16 bytes aligned
    xpin = _mm_load_si128((__m128i*)p);

#define H4X_SSE2_X4DJBX33A_ROUND(round) \
    xp = _mm_setzero_si128(); \
    tmp = _mm_extract_epi8(xpin, round * 4 + 0); \
    xp = _mm_insert_epi16(xp, tmp, 0); \
    tmp = _mm_extract_epi8(xpin, round * 4 + 1); \
    xp = _mm_insert_epi16(xp, tmp, 2); \
    tmp = _mm_extract_epi8(xpin, round * 4 + 2); \
    xp = _mm_insert_epi16(xp, tmp, 4); \
    tmp = _mm_extract_epi8(xpin, round * 4 + 3); \
    xp = _mm_insert_epi16(xp, tmp, 6); \
    xtmp = _mm_slli_epi32(xstate, 5); \
    xstate = _mm_add_epi32(xstate, xtmp); \
    xstate = _mm_add_epi32(xstate, xp);

    H4X_SSE2_X4DJBX33A_ROUND(0)
    H4X_SSE2_X4DJBX33A_ROUND(1)
    H4X_SSE2_X4DJBX33A_ROUND(2)
    H4X_SSE2_X4DJBX33A_ROUND(3)
#undef H4X_SSE2_X4DJBX33A_ROUND
#endif

#if 0
#define HX4_SSE2_DJB2ROUND(round) \
    /* load 4 bytes, expand into xp */ \
    xp = _mm_setzero_si128(); \
    xp = _mm_insert_epi16(xp, p[round * 4 + 0], 0); \
    xp = _mm_insert_epi16(xp, p[round * 4 + 1], 2); \
    xp = _mm_insert_epi16(xp, p[round * 4 + 2], 4); \
    xp = _mm_insert_epi16(xp, p[round * 4 + 3], 6); \
    xp = _mm_add_epi32(xp, xstate); \
    xstate = _mm_slli_epi32(xstate, 5); \
    xstate = _mm_add_epi32(xstate, xp);
    
    HX4_SSE2_DJB2ROUND(0)
    HX4_SSE2_DJB2ROUND(1)
    HX4_SSE2_DJB2ROUND(2)
    HX4_SSE2_DJB2ROUND(3)
#undef HX4_SSE2_DJB2ROUND
#endif

#if 0
#define HX4_SSE2_DJB2ROUND(round) \
    /* load 4 bytes, expand into xp */ \
    /* xp = _mm_set_epi32(p[round*4+3], p[round*4+2], p[round*4+1], p[round*4+0]); */ \
    xp = _mm_setr_epi32(p[round*4+0], p[round*4+1], p[round*4+2], p[round*4+3]); \
    xp = _mm_add_epi32(xp, xstate); \
    xstate = _mm_slli_epi32(xstate, 5); \
    xstate = _mm_add_epi32(xstate, xp);
    HX4_SSE2_DJB2ROUND(0)
    HX4_SSE2_DJB2ROUND(1)
    HX4_SSE2_DJB2ROUND(2)
    HX4_SSE2_DJB2ROUND(3)
#undef HX4_SSE2_DJB2ROUND
#endif
        
    p+=16;
  }

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
    state_i = (state_i+1) & 0x03;
  }

  hx4_xor_cookie_128(state, cookie);
  memcpy(out_hash, state, sizeof(state));

  return HX4_ERR_SUCCESS;
}
#endif //HX4_HAS_SSE2

#if HX4_HAS_SSSE3
int hx4_x4djbx33a_128_ssse3(const void *buffer, size_t buffer_size, const void *cookie, size_t cookie_sz, void *out_hash, size_t out_hash_size) {
  const uint8_t *p;
  const uint8_t * const buffer_end = (uint8_t*)buffer + buffer_size;
  const int num_bytes_to_seek = hx4_bytes_to_aligned(buffer, 16);
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
    state_i = (state_i + 1) & 0x03;
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
  while (p+15<buffer_end) {
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
    state_i = (state_i + 1) & 0x03;
  }
  
  hx4_xor_cookie_128(state, cookie);
  memcpy(out_hash, state, sizeof(state));

  return HX4_ERR_SUCCESS;
}
#endif //HX4_HAS_SSSE3
