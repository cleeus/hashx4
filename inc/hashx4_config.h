#ifndef HASHX4_CONFIG_H
#define HASHX4_CONFIG_H

#ifdef _MSC_VER

# ifdef _M_IX86
#   define HX4_HAS_MMX 1
# else
#   define HX4_HAS_MMX 0
# endif

# define HX4_HAS_SSE2 1
# define HX4_HAS_SSSE3 1

#elif defined(__GNUC__)

# ifdef __MMX__
#   define HX4_HAS_MMX 1
# else
#   define HX4_HAS_MMX 0
# endif

# ifdef __SSE2__
#   define HX4_HAS_SSE2 1
# else
#   define HX4_HAS_SSE2 0
# endif

# ifdef __SSSE3__
#   define HX4_HAS_SSSE3 1
# else
#   define HX4_HAS_SSSE3 0
# endif

#else
# error platform auto config not implemented for this compiler
#endif

#endif
