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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __GNUC__
# include <time.h>
#elif _MSC_VER
# include <windows.h>
#endif

#include "hashx4.h"

typedef struct {
#ifdef __GNUC__
  struct timespec ts;
#elif _MSC_VER
  DWORD ticks;
#endif
} hx_time;

hx_time hx_gettime() {
  hx_time out;
  memset(&out, 0, sizeof(out));

#ifdef __GNUC__
  clock_gettime(CLOCK_MONOTONIC, &out.ts);
#elif _MSC_VER
  out.ticks = GetTickCount();
#endif
  return out;
}

float hx_time_to_s(const hx_time *t) {
#ifdef __GNUC__
  return (float)((double)t->ts.tv_sec + (double)t->ts.tv_nsec / 1000000000.0l);
#elif _MSC_VER
  return (float)(t->ticks / 1000.0);
#endif
}

float hx_timedelta_s(const hx_time *start, const hx_time *end) {
  float delta = hx_time_to_s(end) - hx_time_to_s(start);
  return delta >= 0 ? delta : -delta;
}

float MiB_per_s(float bytes, const hx_time *start, const hx_time *end) {
  float duration_s = hx_timedelta_s(start, end);
  if(duration_s == 0) {
    return -1;
  } else {
    return (float)((bytes / (1024.0*1024.0)) / duration_s);
  }
}

#define HX4_PERF_TEST_IMPL(hash_function, output_length_bits) \
static int test_##hash_function##_performance(FILE *stream, const void *in, size_t in_sz, const void *cookie, size_t cookie_sz) { \
  volatile int rc = 0; \
  hx_time start; \
  hx_time stop; \
  float timedelta = 0; \
  int repeat_count = 0; \
  volatile unsigned char hash_output[(output_length_bits)/8]; \
  \
  start = hx_gettime(); \
  while (timedelta < 10.0) { \
    repeat_count++; \
    rc += hash_function(in, in_sz, cookie, cookie_sz, (void*)hash_output, sizeof(hash_output)); \
    stop = hx_gettime(); \
    timedelta = hx_timedelta_s(&start, &stop); \
  } \
  fprintf(stream, "\thashed %dx%d MiB = %dMiB / %.2fs = %.2f MiB/s\n", \
    repeat_count, (int)(in_sz/(1024*1024)), (int)((in_sz/(1024*1024)) * repeat_count), timedelta, \
    MiB_per_s((float)in_sz*(float)repeat_count, &start, &stop)); \
  return rc; \
} \


HX4_PERF_TEST_IMPL(hx4_djbx33a_32_ref, 32)
HX4_PERF_TEST_IMPL(hx4_djbx33a_32_copt, 32)
HX4_PERF_TEST_IMPL(hx4_x4djbx33a_128_ref, 128)
HX4_PERF_TEST_IMPL(hx4_x4djbx33a_128_copt, 128)
#if HX4_HAS_MMX
HX4_PERF_TEST_IMPL(hx4_x4djbx33a_128_mmx, 128)
#endif
#if HX4_HAS_SSE2
HX4_PERF_TEST_IMPL(hx4_x4djbx33a_128_sse2, 128)
#endif
#if HX4_HAS_SSSE3
HX4_PERF_TEST_IMPL(hx4_x4djbx33a_128_ssse3, 128)
#endif
HX4_PERF_TEST_IMPL(hx4_siphash24_64_ref, 64)

static int test_hx4_x4djbx33a_128_all_correctness(FILE *stream, const void *in, size_t in_sz, const void *cookie, size_t cookie_sz) {
  int rc = 0;
  int i;
  uint8_t hash_output_ref[128/8];
  uint8_t hash_output_copt[128/8];
#if HX4_HAS_MMX
  uint8_t hash_output_mmx[128/8];
#endif
#if HX4_HAS_SSE2
  uint8_t hash_output_sse2[128/8];
#endif
#if HX4_HAS_SSSE3
  uint8_t hash_output_ssse3[128 / 8];
#endif

  if(in_sz < 1024) {
    fprintf(stream, "\tinput buffer too small\n");
    return 1;
  }

  in_sz /= 1024;
  if(in_sz < 1024) {
    in_sz = 1024;
  }

  for(i=0; i<32 && i<in_sz; i++) { 
    rc = hx4_x4djbx33a_128_ref((uint8_t*)in+i, in_sz-i, cookie, cookie_sz, hash_output_ref, sizeof(hash_output_ref));
    if(rc != HX4_ERR_SUCCESS) {
      return rc;
    }
    rc = hx4_x4djbx33a_128_copt((uint8_t*)in+i, in_sz-i, cookie, cookie_sz, hash_output_copt, sizeof(hash_output_copt));
    if(rc != HX4_ERR_SUCCESS) {
      return rc;
    }
#if HX4_HAS_MMX
    rc = hx4_x4djbx33a_128_mmx((uint8_t*)in+i, in_sz-i, cookie, cookie_sz, hash_output_mmx, sizeof(hash_output_mmx));
    if(rc != HX4_ERR_SUCCESS) {
      return rc;
    }
#endif
#if HX4_HAS_SSE2
    rc = hx4_x4djbx33a_128_sse2((uint8_t*)in+i, in_sz-i, cookie, cookie_sz, hash_output_sse2, sizeof(hash_output_sse2));
    if(rc != HX4_ERR_SUCCESS) {
      return rc;
    }
#endif
#if HX4_HAS_SSSE3
    rc = hx4_x4djbx33a_128_ssse3((uint8_t*)in+i, in_sz-i, cookie, cookie_sz, hash_output_ssse3, sizeof(hash_output_ssse3));
    if (rc != HX4_ERR_SUCCESS) {
      return rc;
    }
#endif

    if(memcmp(hash_output_ref, hash_output_copt, sizeof(hash_output_ref)) != 0) {
      fprintf(stream, "\tcopt output doesn't match ref output at offset %d\n", i);
      return 1;
    }
#if HX4_HAS_MMX
    if(memcmp(hash_output_ref, hash_output_mmx, sizeof(hash_output_ref)) != 0) {
      fprintf(stream, "\tmmx output doesn't match ref output at offset %d\n", i);
      return 1;
    }
#endif
#if HX4_HAS_SSE2
    if(memcmp(hash_output_ref, hash_output_sse2, sizeof(hash_output_ref)) != 0) {
      fprintf(stream, "\tsse2 output doesn't match ref output at offset %d\n", i);
      return 1;
    }
#endif
#if HX4_HAS_SSSE3
    if (memcmp(hash_output_ref, hash_output_ssse3, sizeof(hash_output_ref)) != 0) {
      fprintf(stream, "\tssse3 output doesn't match ref output at offset %d\n", i);
      return 1;
    }
#endif

  }

  return 0;
}

static int test_hx4_djbx33a_32_all_correctness(FILE *stream, const void *in, size_t in_sz, const void *cookie, size_t cookie_sz) {
  int rc = 0;
  int i;
  uint8_t hash_output_ref[32/8];
  uint8_t hash_output_copt[32/8];

  if(in_sz < 1024) {
    fprintf(stream, "\tinput buffer too small\n");
    return 1;
  }
  in_sz /= 1024;
  if(in_sz < 1024) {
    in_sz = 1024;
  }

  for(i=0; i<32 && i<in_sz; i++) { 
    rc = hx4_djbx33a_32_ref((uint8_t*)in+i, in_sz-i, cookie, cookie_sz, hash_output_ref, sizeof(hash_output_ref));
    if(rc != HX4_ERR_SUCCESS) {
      return rc;
    }
    rc = hx4_djbx33a_32_copt((uint8_t*)in+i, in_sz-i, cookie, cookie_sz, hash_output_copt, sizeof(hash_output_copt));
    if(rc != HX4_ERR_SUCCESS) {
      return rc;
    }

    if(memcmp(hash_output_ref, hash_output_copt, sizeof(hash_output_ref)) != 0) {
      fprintf(stream, "\tcopt output doesn't match ref output at offset %d\n", i);
      return 1;
    }

  }

  return 0;
}

#define HX4_TEST_COOKIE_APPLIED_IMPL(hash_function, output_bits) \
static int test_##hash_function##_cookie_applied(FILE *stream, const void *in, size_t in_sz, const void *cookie, size_t cookie_sz) { \
  uint8_t out_i_cookie[(output_bits)/8]; \
  uint8_t out_z_cookie[(output_bits)/8]; \
  uint8_t out_1_cookie[(output_bits)/8]; \
  uint8_t zero_cookie[128/8]; \
  uint8_t ones_cookie[128/8]; \
  int rc; \
  if(in_sz < 1024) { \
    fprintf(stream, "\tinput buffer too small\n"); \
    return 1; \
  } \
  in_sz = 1024; \
  memset(zero_cookie, 0, sizeof(zero_cookie)); \
  memset(ones_cookie, 1, sizeof(ones_cookie)); \
  memset(out_i_cookie, 0, sizeof(out_i_cookie)); \
  memset(out_z_cookie, 0, sizeof(out_z_cookie)); \
  rc = hash_function(in, in_sz, cookie, cookie_sz, out_i_cookie, sizeof(out_i_cookie)); \
  if(rc != HX4_ERR_SUCCESS) { \
    return rc; \
  } \
  rc = hash_function(in, in_sz, zero_cookie, sizeof(zero_cookie), out_z_cookie, sizeof(out_z_cookie)); \
  if(rc != HX4_ERR_SUCCESS) { \
    return rc; \
  } \
  rc = hash_function(in, in_sz, ones_cookie, sizeof(ones_cookie), out_1_cookie, sizeof(out_1_cookie)); \
  if(rc != HX4_ERR_SUCCESS) { \
    return rc; \
  } \
  if(memcmp(out_i_cookie, out_z_cookie, sizeof(out_i_cookie)) == 0) { \
    fprintf(stream, "\tcookie not applied\n"); \
    return 2; \
  } \
  if(memcmp(out_i_cookie, out_1_cookie, sizeof(out_i_cookie)) == 0) { \
    fprintf(stream, "\tcookie not applied\n"); \
    return 3; \
  } \
  return 0; \
}


HX4_TEST_COOKIE_APPLIED_IMPL(hx4_djbx33a_32_ref, 32)
HX4_TEST_COOKIE_APPLIED_IMPL(hx4_djbx33a_32_copt, 32)
HX4_TEST_COOKIE_APPLIED_IMPL(hx4_x4djbx33a_128_ref, 128)
HX4_TEST_COOKIE_APPLIED_IMPL(hx4_x4djbx33a_128_copt, 128)
#if HX4_HAS_MMX
HX4_TEST_COOKIE_APPLIED_IMPL(hx4_x4djbx33a_128_mmx, 128)
#endif
#if HX4_HAS_SSE2
HX4_TEST_COOKIE_APPLIED_IMPL(hx4_x4djbx33a_128_sse2, 128)
#endif
#if HX4_HAS_SSSE3
HX4_TEST_COOKIE_APPLIED_IMPL(hx4_x4djbx33a_128_ssse3, 128)
#endif


typedef int (*test_function_t)(FILE*, const void *, size_t, const void *, size_t);
typedef struct {
  test_function_t function;
  const char *name;
} test_t;

#define TEST_ITEM(function_name) { function_name , #function_name } ,

static void init_random_buffer(unsigned char *buffer, size_t buffer_size) {
  unsigned char * p = buffer;
  unsigned char * const buffer_end = buffer+buffer_size;
  srand(42);
  size_t i;

  for(i=0;p<buffer_end;p++,i++) {
    //*p = rand();
    *p = (unsigned char)(i & 0x00ff);
  }
}


int main(int argc, char **argv) {
  int test_result = 0;
  int temp = 0;
  int i = 0;
  unsigned char *random_buffer = NULL;
  const int random_buffer_size = 1024*1024*128 + 23;
  uint8_t cookie[128/8];

  printf("initializing input buffers\n");
  random_buffer = malloc(random_buffer_size);
  if(!random_buffer) {
    return -1;
  }
  init_random_buffer(random_buffer, random_buffer_size);
  init_random_buffer(cookie, sizeof(cookie));
  printf("\tallocated and initialized %d MiB for tests\n", random_buffer_size/(1024*1024));

  test_t tests[] = {
    TEST_ITEM(test_hx4_x4djbx33a_128_all_correctness)
    TEST_ITEM(test_hx4_djbx33a_32_all_correctness)
   
    TEST_ITEM(test_hx4_djbx33a_32_ref_cookie_applied)
    TEST_ITEM(test_hx4_djbx33a_32_copt_cookie_applied)
    TEST_ITEM(test_hx4_x4djbx33a_128_ref_cookie_applied)
    TEST_ITEM(test_hx4_x4djbx33a_128_copt_cookie_applied)
#if HX4_HAS_MMX
    TEST_ITEM(test_hx4_x4djbx33a_128_mmx_cookie_applied)
#endif
#if HX4_HAS_SSE2
    TEST_ITEM(test_hx4_x4djbx33a_128_sse2_cookie_applied)
#endif
#if HX4_HAS_SSSE3
    TEST_ITEM(test_hx4_x4djbx33a_128_ssse3_cookie_applied)
#endif
 
    TEST_ITEM(test_hx4_djbx33a_32_ref_performance)
    TEST_ITEM(test_hx4_djbx33a_32_copt_performance)
    TEST_ITEM(test_hx4_x4djbx33a_128_ref_performance)
    TEST_ITEM(test_hx4_x4djbx33a_128_copt_performance)
#if HX4_HAS_MMX
    TEST_ITEM(test_hx4_x4djbx33a_128_mmx_performance)
#endif
#if HX4_HAS_SSE2
    TEST_ITEM(test_hx4_x4djbx33a_128_sse2_performance)
#endif
#if HX4_HAS_SSSE3
    TEST_ITEM(test_hx4_x4djbx33a_128_ssse3_performance)
#endif
    TEST_ITEM(test_hx4_siphash24_64_ref_performance)
  };

  for(i=0; i<sizeof(tests)/sizeof(test_t); i++) {
    printf("> start executing test: %s\n", tests[i].name); 
    temp = tests[i].function(stdout, random_buffer, random_buffer_size, cookie, sizeof(cookie));
    printf("< done executing test, result: %d\n", temp);
    test_result += temp < 0 ? -temp : temp;
  }

  if(test_result != 0) {
    printf("tests failed\n");
  } else {
    printf("tests passed\n");
  }

  free(random_buffer);

#ifdef _MSC_VER
  printf("press enter\n");
  getc(stdin);
#endif
  return test_result;
}


