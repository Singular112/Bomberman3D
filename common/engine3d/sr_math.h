#pragma once

#include "sr_lib.h"

#ifdef WIN32
#	include <xmmintrin.h>
#endif

// stl
#include <stdint.h>

// glm
#include "glm_includes.h"

namespace sr
{

namespace math
{

#if 0
#	define DEFAULT_DATA_ALIGN_SPEC __declspec(align(16))
#else
#	define DEFAULT_DATA_ALIGN_SPEC
#endif

#define SR_PI			3.141592654f
#define SR_PI_DIV2		1.570796327f

#define SR_EPSILON_E3	(float)(1E-3)
#define SR_EPSILON_E4	(float)(1E-4)
#define SR_EPSILON_E5	(float)(1E-5)
#define SR_EPSILON_E6	(float)(1E-6)

#define SR_FCMP(a,b)	((fabs(a - b) < SR_EPSILON_E3) ? 1 : 0)

#define SR_DEG_TO_RAD(deg) (deg * SR_PI / 180.0f)

extern float g_sin_table[361];
extern float g_cos_table[361];
extern float g_epsilon;

#define FIND_MIN(a,b) ((a) < (b) ? a : b)

// ========================= VECTORS DEFINITION =========================

typedef struct vector2df_s
{
	union
	{
		DEFAULT_DATA_ALIGN_SPEC float m[2];
		struct
		{
			float x, y;
		};
	};
} vector2df_t, point2df_t;

typedef struct vector2di_s
{
	union
	{
		DEFAULT_DATA_ALIGN_SPEC int32_t m[2];
		struct
		{
			int32_t x, y;
		};
	};
} vector2di_t, point2di_t;

typedef struct vector3df_s
{
	union
	{
		DEFAULT_DATA_ALIGN_SPEC float m[3];
		struct
		{
			float x, y, z;
		};
	};
} vector3df_t, point3df_t;

typedef struct vector4df_s
{
	union
	{
		DEFAULT_DATA_ALIGN_SPEC float m[4];
		__m128 sse_128data;
		struct
		{
			float x, y, z, w;
		};
	};
} vector4df_t, point4df_t;

// ========================= MATRIX DEFINITION =========================

class mat_type1 {};
class mat_type2 {};

typedef struct matrix2x2f_s
{
	union
	{
		DEFAULT_DATA_ALIGN_SPEC float m[2][2];
		struct
		{
			float m00, m01;
			float m10, m11;
		};
	};
} matrix2x2f_t;

typedef struct matrix3x3f_s
{
	union
	{
		DEFAULT_DATA_ALIGN_SPEC float m[3][3];
		struct
		{
			float m00, m01, m02;
			float m10, m11, m12;
			float m20, m21, m22;
		};
	};
} matrix3x3f_t;

typedef struct matrix4x3f_s
{
	union
	{
		DEFAULT_DATA_ALIGN_SPEC float m[4][3];
		struct
		{
			float m00, m01, m02;
			float m10, m11, m12;
			float m20, m21, m22;
			float m30, m31, m32;
		};
	};
} matrix4x3f_t;

typedef struct matrix4x4f_s
{
	union
	{
		DEFAULT_DATA_ALIGN_SPEC float m[4][4];
		struct
		{
			float m00, m01, m02, m03;
			float m10, m11, m12, m13;
			float m20, m21, m22, m23;
			float m30, m31, m32, m33;
		};
	};
} matrix4x4f_t;

const matrix4x4f_t imatf_4x4 =
{
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};

const matrix4x3f_t imatf_4x3 =
{
	1, 0, 0,
	0, 1, 0,
	0, 0, 1,
	0, 0, 0
};

const matrix3x3f_t imatf_3x3 =
{
	1, 0, 0,
	0, 1, 0,
	0, 0, 1
};

const matrix2x2f_t imatf_2x2 =
{
	1, 0,
	0, 1
};

// ========================= PLANES DEFINITION =========================

typedef struct plane3d_s
{
	point3df_t p;
	vector3df_t n;
} plane3d_t;

// ========================= MATH FUNCTIONS DEFINITION =========================

SOFTRENDERLIB_API void build_sin_cos_table();
SOFTRENDERLIB_API float fast_sin(float angle_deg);
SOFTRENDERLIB_API float fast_cos(float angle_deg);

inline void vector3df_build(const vector3df_t* vec1,
	const vector3df_t* vec2,
	vector3df_t* vec_result);
SOFTRENDERLIB_API void vector4df_build(const vector4df_t* vec1,
	const vector4df_t* vec2,
	vector4df_t* vec_result);

inline void vector3df_initialize_xyz(vector3df_t* vec_result,
	float x, float y, float z)
{
	vec_result->x = x;
	vec_result->y = y;
	vec_result->z = z;
}
inline void vector4df_initialize_xyz(vector4df_t* vec_result,
	float x, float y, float z)
{
	vec_result->x = x;
	vec_result->y = y;
	vec_result->z = z;
	vec_result->w = 1;
}

inline void glm_vec4_initialize_xyz(glm::vec4& vec_result,
	float x, float y, float z)
{
	vec_result.x = x;
	vec_result.y = y;
	vec_result.z = z;
	vec_result.w = 1;
}

SOFTRENDERLIB_API void vector4df_cross(const vector4df_t* vec1,
	const vector4df_t* vec2,
	vector4df_t* vec_result);

SOFTRENDERLIB_API float vector4df_dot(const vector4df_t* vec1,
	const vector4df_t* vec2);

inline void vector3df_add(const vector3df_t* vec1,
	const vector3df_t* vec2,
	vector3df_t* vec_result);
SOFTRENDERLIB_API void vector4df_add(const vector4df_t* vec1,
	const vector4df_t* vec2,
	vector4df_t* vec_result);

inline float vector4df_len(const vector4df_t* vec);
inline float vector3df_len(const vector3df_t* vec);

inline void vector3df_normalize(const vector3df_t* vec,
	vector3df_t* vec_result);
SOFTRENDERLIB_API void vector4df_normalize(const vector4df_t* vec,
	vector4df_t* vec_result);

inline void vector3df_zero(vector3df_t* vec);
inline void vector4df_zero(vector4df_t* vec);

SOFTRENDERLIB_API void matrix4x4_mul_vector4d(const vector4df_t* source_vector,
	const matrix4x4f_t* mat,
	vector4df_t* dest_vector);

SOFTRENDERLIB_API void matrix4x4_init(matrix4x4f_t* mat,
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33);

SOFTRENDERLIB_API void matrix4x4_mul_matrix4x4(const matrix4x4f_t* mat1,
	const matrix4x4f_t* mat2,
	matrix4x4f_t* mat_result);

#define MATRIX4X4_COPY(mat_source, mat_dest)				\
	fast_objcopy(mat_dest, mat_source);

#define MATRIX4X4_IDENTITY(mat_dest)						\
	fast_objcopy(mat_dest, &imatf_4x4);

SOFTRENDERLIB_API void plane3d_init(plane3d_t* plane,
	const point3df_t* point,
	const vector3df_t* normal,
	bool need_normalize);

}

}
