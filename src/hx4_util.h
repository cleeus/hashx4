#ifndef HASHX4_UTIL_H
#define HASHX4_UTIL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __GNUC__
# define HX4_ASSUME_ALIGNED(ptr, alignment) { ptr = __builtin_assume_aligned( (ptr) , (alignment) ); }
#elif _MSC_VER
# define HX4_ASSUME_ALIGNED(ptr, alignment) __assume((size_t)p % 16 == 0);
#else
# error HX4_ASSUME_ALIGNED not yet implement on this compiler
#endif


#ifdef __GNUC__
# define HX4_ALIGNED( declaration , alignment ) declaration __attribute__((aligned( (alignment))))
#elif _MSC_VER
# define HX4_ALIGNED( declaration , alignment ) _declspec(align((alignment))) declaration
#else
# error HX4_ALIGNED not yet implemented on this compiler
#endif

#ifdef __cplusplus
extern "C" {
#endif

int hx4_check_params(size_t sizeof_state, const void *in, size_t in_sz, const void *cookie, size_t cookie_sz, void *out, size_t out_sz);
int hx4_bytes_to_aligned(const void *ptr);
 
#ifdef __cplusplus
}
#endif

#endif
