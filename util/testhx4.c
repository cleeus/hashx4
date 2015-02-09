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

float MiB_per_s(size_t bytes, const hx_time *start, const hx_time *end) {
  float duration_s = hx_timedelta_s(start, end);
  if(duration_s == 0) {
    return -1;
  } else {
    return (float)((bytes / (1024.0*1024.0)) / duration_s);
  }
}

#define HX4_PERF_TEST_IMPL(hash_function, output_length_bits) \
static int test_##hash_function##_performance(FILE *stream, const void *random_buffer, size_t random_buffer_size) { \
  int rc = 0; \
  hx_time start; \
  hx_time stop; \
  float timedelta = 0; \
  int repeat_count = 0; \
  unsigned char hash_output[(output_length_bits)/8]; \
  \
  start = hx_gettime(); \
  while (timedelta < 3.0) { \
    repeat_count++; \
    rc += hash_function(random_buffer, random_buffer_size, hash_output, sizeof(hash_output)); \
    stop = hx_gettime(); \
    timedelta = hx_timedelta_s(&start, &stop); \
  } \
  fprintf(stream, "\thashed %f MiB/s\n", MiB_per_s(random_buffer_size*repeat_count, &start, &stop)); \
  return rc; \
} \


HX4_PERF_TEST_IMPL(hx4_djbx33a_32_ref, 32)
HX4_PERF_TEST_IMPL(hx4_djbx33a_32_copt, 32)
HX4_PERF_TEST_IMPL(hx4_x4djbx33a_128_ref, 128)
HX4_PERF_TEST_IMPL(hx4_x4djbx33a_128_copt, 128)
HX4_PERF_TEST_IMPL(hx4_x4djbx33a_128_sse2, 128)
HX4_PERF_TEST_IMPL(hx4_x4djbx33a_128_ssse3, 128)
HX4_PERF_TEST_IMPL(hx4_siphash24_64_ref, 64)

static int test_hx4_x4djbx33a_128_all_correctness(FILE *stream, const void *random_buffer, size_t random_buffer_size) {
  int rc = 0;
  int i;
  uint8_t hash_output_ref[128/8];
  uint8_t hash_output_copt[128/8];
  uint8_t hash_output_sse2[128/8];
  uint8_t hash_output_ssse3[128 / 8];

  if(random_buffer_size < 1024) {
    fprintf(stream, "\trandom buffer too small\n");
    return 1;
  }

  random_buffer_size /= 1024;
  if(random_buffer_size < 1024) {
    random_buffer_size = 1024;
  }

  for(i=0; i<32 && i<random_buffer_size; i++) { 
    rc = hx4_x4djbx33a_128_ref((uint8_t*)random_buffer+i, random_buffer_size-i, hash_output_ref, sizeof(hash_output_ref));
    if(rc != HX4_ERR_SUCCESS) {
      return rc;
    }
    rc = hx4_x4djbx33a_128_copt((uint8_t*)random_buffer+i, random_buffer_size-i, hash_output_copt, sizeof(hash_output_copt));
    if(rc != HX4_ERR_SUCCESS) {
      return rc;
    }
    rc = hx4_x4djbx33a_128_sse2((uint8_t*)random_buffer+i, random_buffer_size-i, hash_output_sse2, sizeof(hash_output_sse2));
    if(rc != HX4_ERR_SUCCESS) {
      return rc;
    }
    rc = hx4_x4djbx33a_128_ssse3((uint8_t*)random_buffer + i, random_buffer_size - i, hash_output_ssse3, sizeof(hash_output_ssse3));
    if (rc != HX4_ERR_SUCCESS) {
      return rc;
    }


    if(memcmp(hash_output_ref, hash_output_copt, sizeof(hash_output_ref)) != 0) {
      fprintf(stream, "\tcopt output doesn't match ref output at offset %d\n", i);
      return 1;
    }

    if(memcmp(hash_output_ref, hash_output_sse2, sizeof(hash_output_ref)) != 0) {
      fprintf(stream, "\tsse2 output doesn't match ref output at offset %d\n", i);
      return 1;
    }

    if (memcmp(hash_output_ref, hash_output_ssse3, sizeof(hash_output_ref)) != 0) {
      fprintf(stream, "\tssse3 output doesn't match ref output at offset %d\n", i);
      return 1;
    }

  }

  return 0;
}


typedef int (*test_function_t)(FILE*, const void *, size_t);
typedef struct {
  test_function_t function;
  const char *name;
} test_t;

#define TEST_ITEM(function_name) { function_name , #function_name } ,

static void init_random_buffer(unsigned char *buffer, size_t buffer_size) {
  unsigned char * p = buffer;
  unsigned char * const buffer_end = buffer+buffer_size;
  //srand(42);
  for(;p<buffer_end;p++) {
    //*p = rand();
    *p = (unsigned char)(p-buffer);
  }
}


int main(int argc, char **argv) {
  int test_result = 0;
  int temp = 0;
  int i = 0;
  unsigned char *random_buffer = NULL;
  const int random_buffer_size = 1024*1024*512 + 23;
  
  random_buffer = malloc(random_buffer_size);
  if(!random_buffer) {
    return -1;
  }
  init_random_buffer(random_buffer, random_buffer_size);

  test_t tests[] = {
    TEST_ITEM(test_hx4_djbx33a_32_ref_performance)
    TEST_ITEM(test_hx4_djbx33a_32_copt_performance)
    TEST_ITEM(test_hx4_x4djbx33a_128_ref_performance)
    TEST_ITEM(test_hx4_x4djbx33a_128_copt_performance)
    TEST_ITEM(test_hx4_x4djbx33a_128_sse2_performance)
    TEST_ITEM(test_hx4_x4djbx33a_128_ssse3_performance)
    TEST_ITEM(test_hx4_x4djbx33a_128_all_correctness)
    TEST_ITEM(test_hx4_siphash24_64_ref_performance)
  };

  for(i=0; i<sizeof(tests)/sizeof(test_t); i++) {
    printf("> start executing test: %s\n", tests[i].name); 
    temp = tests[i].function(stdout, random_buffer, random_buffer_size);
    printf("< done executing test, result: %d\n", temp);
    test_result += temp < 0 ? -temp : temp;
  }

  if(test_result != 0) {
    printf("tests failed\n");
  } else {
    printf("tests passed\n");
  }

  free(random_buffer);

  getc(stdin);
  return test_result;
}

