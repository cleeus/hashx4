#ifndef HASHX4_CONFIG_H
#define HASHX4_CONFIG_H

#ifdef _MSC_VER

# ifdef _M_IX86
#   define HX4_HAS_MMX 1
# else
#   define HX4_HAS_MMX 0
#endif

# define HX4_HAS_SSE2 1
# define HX4_HAS_SSSE3 1

#else
# define HX4_HAS_MMX 1
# define HX4_HAS_SSE2 1
# define HX4_HAS_SSSE3 1
#endif

#endif
