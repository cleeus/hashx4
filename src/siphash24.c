/*
   SipHash reference C implementation

   Written in 2012 by 
   Jean-Philippe Aumasson <jeanphilippe.aumasson@gmail.com>
   Daniel J. Bernstein <djb@cr.yp.to>

   To the extent possible under law, the author(s) have dedicated all copyright
   and related and neighboring rights to this software to the public domain
   worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with
   this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/
#include <stdint.h>
#include <string.h>

#include "hashx4.h"
#include "hx4_util.h"

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t u8;

#define ROTL(x,b) (u64)( ((x) << (b)) | ( (x) >> (64 - (b))) )

#define U32TO8_LE(p, v)         \
    (p)[0] = (u8)((v)      ); (p)[1] = (u8)((v) >>  8); \
    (p)[2] = (u8)((v) >> 16); (p)[3] = (u8)((v) >> 24);

#define U64TO8_LE(p, v)         \
  U32TO8_LE((p),     (u32)((v)      ));   \
  U32TO8_LE((p) + 4, (u32)((v) >> 32));

#define U8TO64_LE(p) \
  (((u64)((p)[0])      ) | \
   ((u64)((p)[1]) <<  8) | \
   ((u64)((p)[2]) << 16) | \
   ((u64)((p)[3]) << 24) | \
   ((u64)((p)[4]) << 32) | \
   ((u64)((p)[5]) << 40) | \
   ((u64)((p)[6]) << 48) | \
   ((u64)((p)[7]) << 56))

#define SIPROUND            \
  do {              \
    v0 += v1; v1=ROTL(v1,13); v1 ^= v0; v0=ROTL(v0,32); \
    v2 += v3; v3=ROTL(v3,16); v3 ^= v2;     \
    v0 += v3; v3=ROTL(v3,21); v3 ^= v0;     \
    v2 += v1; v1=ROTL(v1,17); v1 ^= v2; v2=ROTL(v2,32); \
  } while(0)

/* SipHash-2-4 */
static int crypto_auth( unsigned char *out, const unsigned char *in, unsigned long long inlen, const unsigned char *k )
{
  /* "somepseudorandomlygeneratedbytes" */
  u64 v0 = 0x736f6d6570736575ULL;
  u64 v1 = 0x646f72616e646f6dULL;
  u64 v2 = 0x6c7967656e657261ULL;
  u64 v3 = 0x7465646279746573ULL;
  u64 b;
  u64 k0 = U8TO64_LE( k );
  u64 k1 = U8TO64_LE( k + 8 );
  u64 m;
  const u8 *end = in + inlen - ( inlen % sizeof( u64 ) );
  const int left = inlen & 7;
  b = ( ( u64 )inlen ) << 56;
  v3 ^= k1;
  v2 ^= k0;
  v1 ^= k1;
  v0 ^= k0;

  for ( ; in != end; in += 8 )
  {
    m = U8TO64_LE( in );
    v3 ^= m;
    SIPROUND;
    SIPROUND;
    v0 ^= m;
  }

  switch( left )
  {
  case 7: b |= ( ( u64 )in[ 6] )  << 48;
  case 6: b |= ( ( u64 )in[ 5] )  << 40;
  case 5: b |= ( ( u64 )in[ 4] )  << 32;
  case 4: b |= ( ( u64 )in[ 3] )  << 24;
  case 3: b |= ( ( u64 )in[ 2] )  << 16;
  case 2: b |= ( ( u64 )in[ 1] )  <<  8;
  case 1: b |= ( ( u64 )in[ 0] ); break;
  case 0: break;
  }

  v3 ^= b;
  SIPROUND;
  SIPROUND;
  v0 ^= b;
  
  v2 ^= 0xff;
  SIPROUND;
  SIPROUND;
  SIPROUND;
  SIPROUND;
  b = v0 ^ v1 ^ v2  ^ v3;
  U64TO8_LE( out, b );
  
  return 0;
}

int hx4_siphash24_64_ref(const void *in, size_t in_sz, const void *cookie, size_t cookie_sz, void *out, size_t out_sz) {
  int rc;

  rc = hx4_check_params(64/8, in, in_sz, cookie, cookie_sz, out, out_sz);
  if(rc != HX4_ERR_SUCCESS) {
    return rc;
  }

  crypto_auth(out, in, in_sz, (const unsigned char*)cookie);
  
  return HX4_ERR_SUCCESS;
}



