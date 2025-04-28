#include <iostream> 
#include <string>
#include <cstring>
#include<emmintrin.h>
#include<tmmintrin.h>
using namespace std;

// 定义了Byte，便于使用
typedef unsigned char Byte;
// 定义了32比特
typedef unsigned int bit32;

// MD5的一系列参数。参数是固定的，其实你不需要看懂这些
#define s11 7
#define s12 12
#define s13 17
#define s14 22
#define s21 5
#define s22 9
#define s23 14
#define s24 20
#define s31 4
#define s32 11
#define s33 16
#define s34 23
#define s41 6
#define s42 10
#define s43 15
#define s44 21

/**
 * @Basic MD5 functions.
 *
 * @param there bit32.
 *
 * @return one bit32.
 */
// 定义了一系列MD5中的具体函数
// 这四个计算函数是需要你进行SIMD并行化的
// 可以看到，FGHI四个函数都涉及一系列位运算，在数据上是对齐的，非常容易实现SIMD的并行化

#define F(x,y,z) (((x) & (y)) | ((~x) & (z)))
#define G(x,y,z) (((x) & (z)) | ((y) & (~z)))
#define H(x,y,z) ((x) ^ (y) ^ (z))
#define I(x,y,z) ((y) ^ ((x) | (~z)))

/**
 * @Rotate Left.
 *
 * @param{num} the raw number.
 *
 * @param{n} rotate left n.
 *
 * @return the number after rotated left.
 */
// 定义了一系列MD5中的具体函数
// 这五个计算函数（ROTATELEFT/FF/GG/HH/II）和之前的FGHI一样，都是需要你进行SIMD并行化的
// 但是你需要注意的是#define的功能及其效果，可以发现这里的FGHI是没有返回值的，为什么呢？你可以查询#define的含义和用法
#define ROTATELEFT(num,n) (((num) << (n)) | ((num) >> (32-(n))))

#define FF(a,b,c,d,x,s,ac){ \
  (a) += F ((b),(c),(d)) + (x) + ac; \
  (a)=ROTATELEFT ((a),(s)); \
  (a) += (b); \
}

#define GG(a,b,c,d,x,s,ac){ \
  (a) += G ((b),(c),(d)) + (x) + ac; \
  (a)=ROTATELEFT ((a),(s)); \
  (a) += (b); \
}
#define HH(a,b,c,d,x,s,ac){ \
  (a) += H ((b),(c),(d)) + (x) + ac; \
  (a)=ROTATELEFT ((a),(s)); \
  (a) += (b); \
}
#define II(a,b,c,d,x,s,ac){ \
  (a) += I ((b),(c),(d)) + (x) + ac; \
  (a)=ROTATELEFT ((a),(s)); \
  (a) += (b); \
}

void MD5Hash(string input,bit32 *state); 

static inline __m128i sse_F(__m128i x,__m128i y,__m128i z) {
  return _mm_or_si128(_mm_and_si128(x,y),_mm_andnot_si128(x,z));
}
static inline __m128i sse_G(__m128i x,__m128i y,__m128i z) {
  return _mm_or_si128(_mm_and_si128(x,z),_mm_andnot_si128(z,y));
}
static inline __m128i sse_H(__m128i x,__m128i y,__m128i z) {
  return _mm_xor_si128(_mm_xor_si128(x,y),z);
}
static inline __m128i sse_I(__m128i x,__m128i y,__m128i z) {
  return _mm_xor_si128(y,_mm_or_si128(x,_mm_andnot_si128(z,_mm_set1_epi32(-1))));
}
static inline __m128i sse_rotateleft(__m128i num,int n) {
  return _mm_or_si128(_mm_slli_epi32(num,n),_mm_srli_epi32(num,32 - n));
}
static inline void sse_FF(__m128i* a,__m128i b,__m128i c,__m128i d,__m128i x,int s,__m128i ac) {
  *a = _mm_add_epi32(*a,sse_F(b,c,d));
  *a = _mm_add_epi32(*a,x);
  *a = _mm_add_epi32(*a,ac);
  *a = sse_rotateleft(*a,s);
  *a = _mm_add_epi32(*a,b);
}
static inline void sse_GG(__m128i* a,__m128i b,__m128i c,__m128i d,__m128i x,int s,__m128i ac) {
  *a = _mm_add_epi32(*a,sse_G(b,c,d));
  *a = _mm_add_epi32(*a,x);
  *a = _mm_add_epi32(*a,ac);
  *a = sse_rotateleft(*a,s);
  *a = _mm_add_epi32(*a,b);
}
static inline void sse_HH(__m128i* a,__m128i b,__m128i c,__m128i d,__m128i x,int s,__m128i ac) {
  *a = _mm_add_epi32(*a,sse_H(b,c,d));
  *a = _mm_add_epi32(*a,x);
  *a = _mm_add_epi32(*a,ac);
  *a = sse_rotateleft(*a,s);
  *a = _mm_add_epi32(*a,b);
}
static inline void sse_II(__m128i* a,__m128i b,__m128i c,__m128i d,__m128i x,int s,__m128i ac) {
  *a = _mm_add_epi32(*a,sse_I(b,c,d));
  *a = _mm_add_epi32(*a,x);
  *a = _mm_add_epi32(*a,ac);
  *a = sse_rotateleft(*a,s);
  *a = _mm_add_epi32(*a,b);
}
inline uint32_t byte_swap(uint32_t value) {
  return ((value & 0xFF) << 24) |
         ((value & 0xFF00) << 8) |
         ((value & 0xFF0000) >> 8) |
         ((value & 0xFF000000) >> 24);
}
inline uint32_t get(Byte* message,int block_num,int word_num) {
  int begin = block_num * 64 + word_num * 4;
  return (message[begin]) | (message[begin + 1] << 8) |
         (message[begin + 2] << 16) | (message[begin + 3] << 24);
}
void MD5Hash_simd(const string inputs[4],bit32 states[4][4]);