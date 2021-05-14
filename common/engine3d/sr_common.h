#pragma once

#include "sr_types.h"

namespace sr
{

//
#define IS_POLYGON_DISCARDED(poly_attr)						\
	(poly_attr & (POLY_ATTR_CULLED | POLY_ATTR_CLIPPED))

#define IS_VERTEX_DISCARDED(vert_attr)						\
	(vert_attr & (VERT_ATTR_CLIPPED | VERT_ATTR_CULLED))

// color manipulation macro
#define POP_A(col)			((col >> 24) & 0xFF)
#define POP_R(col)			((col >> 16) & 0xFF)
#define POP_G(col)			((col >> 8) & 0xFF)
#define POP_B(col)			((col >> 0) & 0xFF)
#define XRGB(r, g, b)		(sr::rgb_color_t) ((((uint8_t)(r)) << 16) | (((uint8_t)(g)) << 8) | ((uint8_t)(b)))
#define RGBA(r, g, b, a)	(sr::rgba_color_t) ((((uint8_t)(a)) << 24) | (((uint8_t)(r)) << 16) | (((uint8_t)(g)) << 8) | ((uint8_t)(b)))

// fixed point math
#define FIXP16_CONST	65536
#define FIXP16_CONSTF	65536.5f
#define FP16_2FL(fpval) ((float)(fpval) / FIXP16_CONST)
#define FL_2FP16(flval) ((uint32_t)((flval) * FIXP16_CONSTF))
#define INT2_FP16(intval) (intval << 16)
#define FP16_2INT(fpval) (fpval >> 16)

#define FIXP28_CONST	268435456
#define FIXP28_CONSTF	268435456.5f
#define FP28_2FL(fpval) ((float)(fpval) / FIXP28_CONST)
#define FL_2FP28(flval) ((uint32_t)((flval) * FIXP28_CONSTF))
#define INT2_FP28(intval) (intval << 28)
#define FP28_2INT(fpval) (fpval >> 28)

//10 + 21 + 1
#define FIXP22_CONST	4194304
#define FIXP22_CONSTF	4194304.5f
#define FP22_2FL(fpval) ((float)(fpval) / FIXP22_CONST)
#define FL_2FP22(flval) ((uint32_t)((flval) * FIXP22_CONSTF))
#define INT2_FP22(intval) (intval << 22)
#define FP22_2INT(fpval) (fpval >> 22)

// fast conversions macro
#if 0	// very very slow!
#	define FAST_FLOAT_TO_INT32(val, int_val)	\
		__asm fld val							\
		__asm fistp int_val
#elif 1	// fastest way
#	define FAST_FLOAT_TO_INT32(val, int_val)	\
		int_val = _mm_cvtt_ss2si(_mm_load_ss(&val));
#elif 0	// too fast way as previous
#	define FAST_FLOAT_TO_INT32(val, int_val)	\
	int_val = (int32_t)val;
#else	// non tested, todo: check speed
#	define FAST_FLOAT_TO_INT32(val, int_val)	\
		__asm fld val							\
		__asm fisttp int_val
#endif


template <typename T>
__forceinline void swap(T& a, T& b) { T c = a; a = b; b = c; }


template <typename T>
T find_max(T val1, T val2) { return (val1 > val2) ? val1 : val2; }
template <typename T, typename ... Args>
T find_max(T val1, T val2, Args ... args)
{
	return find_max(find_max(val1, val2), args...);
}


template <typename IntT = int32_t>
__forceinline IntT __fastcall fast_ceil(float val)
{
#if 0
	IntT int_val = (IntT)val;
#else
	IntT int_val;
	FAST_FLOAT_TO_INT32(val, int_val);
#endif

	return int_val < val ? int_val + 1 : int_val;
}


template <typename IntT = int32_t>
__forceinline bool __fastcall is_power_of_2(IntT val)
{
	return (val != 0) && ((val & (val - 1)) == 0);
}


#ifdef SR_USE_FAST_MEMSET
template <typename DestT>
__forceinline void __fastcall fast_memset8(DestT* dest, uint8_t val, size_t count)
{
	__asm
	{
		mov			al, val
		mov			ecx, count
		mov			edi, dest
		rep stos	byte ptr es : [edi]
	}
}


template <typename DestT>
__forceinline void __fastcall fast_memset16(DestT* dest, uint16_t val, size_t count)
{
	__asm
	{
		mov			ax, val
		mov			ecx, count
		mov			edi, dest
		rep stosw
	}
}


template <typename DestT>
__forceinline void __fastcall fast_memset32(DestT* dest, uint32_t val, size_t count)
{
	__asm
	{
		mov			eax, val
		mov			ecx, count
		mov			edi, dest
		rep	stosd
	}
}


#if 1
template <typename DestT, typename SrcT>
__forceinline void __fastcall fast_memcpy(DestT* dest, SrcT* src, size_t _size)
{
	__asm
	{
		mov   esi, src
		mov   edi, dest
		mov   ecx, _size
		rep movs byte ptr es : [edi], byte ptr[esi]
	}
}
#else
#	define fast_memcpy memcpy
#endif

#else
#	define fast_memcpy memcpy
#	define fast_memset8 memset
#	define fast_memset32(dest, val, sz) memset(dest, val, sz * 4)
#endif

#ifdef WIN32
template <typename DestT, typename SrcT>
__forceinline void fast_objcopy(DestT* dest, SrcT* src)
{
	auto sz = sizeof(DestT);
	__asm
	{
		mov   esi, src
		mov   edi, dest
		mov   ecx, sz
		rep movs byte ptr es : [edi], byte ptr[esi]
	}
}
#else
template <typename DestT, typename SrcT>
__forceinline void fast_objcopy(DestT* dest, SrcT* src)
{
#if 0
	auto sz = sizeof(DestT);
	asm
	(
		"mov   esi, src\n\t"
		"mov   edi, dest\n\t"
		"mov   ecx, %1\n\t"
		"rep movs byte ptr es : [edi], byte ptr[esi]\n\t"
	);
#else
	memcpy(dest, src, sizeof(DestT));
#endif
}
#endif

__forceinline u_rgba_color get_u_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0)
{
	return u_rgba_color { a, b, g, r };
}


__forceinline u_rgb24bit_color get_u_rgb24bit(uint8_t r, uint8_t g, uint8_t b)
{
	return u_rgb24bit_color { g, b, r };
}


__forceinline __m128 sse_cross_m128(__m128 const& a, __m128 const& b)
{
	/*
	http://fastcpp.blogspot.com/2011/04/vector-cross-product-using-sse-code.html
	http://threadlocalmutex.com/?p=8

	Vector Cross Product using SSE Code
	A common operation for two 3D vectors is the cross product:
	|a.x|   |b.x|   | a.y * b.z - a.z * b.y |
	|a.y| X |b.y| = | a.z * b.x - a.x * b.z |
	|a.z|   |b.z|   | a.x * b.y - a.y * b.x |
	Executing this operation using scalar instructions requires 6 multiplications and three subtractions. When using vectorized SSE code, the same operation can be performed using 2 multiplications, one subtraction and 4 shuffle operations:
	Both registers a and b contain three floats (x, y and z) where the highest float of the 128-bit register is unused. The values can be loaded using the LoadFloat3 function or SSE set methods such as _mm_setr_ps(x, y, z, 0).
	*/
	__m128 a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 a_zxy = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 b_zxy = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 1));
	__m128 b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));

	return _mm_sub_ps(_mm_mul_ps(a_yzx, b_zxy), _mm_mul_ps(a_zxy, b_yzx));
}

}
