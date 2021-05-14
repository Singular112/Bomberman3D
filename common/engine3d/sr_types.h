#pragma once

#include "setup.h"
#include "sr_math.h"
#include "sr_list.h"
#include "texture2d_c.h"

namespace sr
{

// common polygon attributes
#define POLY_ATTR_NONE							0
//#define POLY_ATTR_ENABLED						1
#define POLY_ATTR_CULLED						2
#define POLY_ATTR_CLIPPED						4
#define POLY_ATTR_HAVE_VERTEX_NORMAL			8
#define POLY_ATTR_TEXTURED						16

//
#define POLY_ATTR_WIREFRAME						32

// z-buffer flags
#define POLY_ATTR_ZBUFFER						64

// shade attributes
#define POLY_ATTR_SHADE_FLAT					128
#define POLY_ATTR_SHADE_GOURAUD					256

// texture attributes
#define POLY_ATTR_AFFINE_TEXTURE				512
#define POLY_ATTR_PERSPECTIVE_CORRECT_TEXTURE	1024

// alpha attributes
#define POLY_ATTR_SIMPLE_ALPHA_CHANNEL			2048
#define POLY_ATTR_ALPHA_CHANNEL					4096

// back face culling attributes
#define POLY_ATTR_2SIDED						8192

#define POLY_DEBUG_ATTR							16384

// common vertices attributes
#define VERT_ATTR_NONE							0
#define VERT_ATTR_CLIPPED						1
#define VERT_ATTR_CULLED						2

using namespace sr::math;

enum e_system_futures : uint8_t
{
	e_sf_sdl_mmx	= 0,
	e_sf_sdl_3dnow	= 1,
	e_sf_sdl_sse	= 2,
	e_sf_sdl_sse2	= 3,
	e_sf_sdl_sse3	= 4,
	e_sf_sdl_sse41	= 5,
	e_sf_sdl_sse42	= 6,
	e_sf_sdl_avx	= 7,
	e_sf_sdl_avx2	= 8
};


union u_rgba_color
{
	uint8_t m[4];

	uint32_t color;

	struct
	{
		uint8_t a, b, g, r;
	};
};


typedef union u_rgb24bit_color
{
	uint8_t m[3];
	struct
	{
		uint8_t b, g, r;
	};
} u_rgb24bit_color_t;


typedef union u_argb32bit_color
{
	uint8_t m[4];
	struct
	{
		uint8_t a, b, g, r;
	};
} u_argb32bit_color_t;


typedef int32_t rgb_color_t;
typedef int32_t rgba_color_t;


enum e_camera_type : uint8_t
{
	e_ct_uvn,
	e_ct_euler
};


enum e_transform_mode : uint8_t
{
	e_tm_transform_local_to_local_list,
	e_tm_transform_local_to_trans_list,
	e_tm_transform_trans_to_trans_list
};


enum e_cam_rotation_sequence : uint8_t
{
	e_crs_xyz,
	e_crs_yxz,
	e_crs_xzy,
	e_crs_yzx,
	e_crs_zyx,
	e_crs_zxy
};


enum e_sort_mode
{
	e_sm_avgz,
	e_sm_minz,
	e_sm_maxz
};


enum e_light_type
{
	e_lt_ambient,
	e_lt_infinite,
	e_lt_point
#if 0	// unsupported
	, e_lt_lightspot
#endif
};


enum e_shade_model : int
{
	e_sm_noshade				= 0,
	e_sm_shade_flat				= POLY_ATTR_SHADE_FLAT,
	e_sm_shade_gouraund			= POLY_ATTR_SHADE_GOURAUD
};


enum e_texture_mode : int
{
	e_tm_notextured				= 0,
	e_tm_affine					= POLY_ATTR_AFFINE_TEXTURE,
	e_tm_perspective_correct	= POLY_ATTR_PERSPECTIVE_CORRECT_TEXTURE
};


typedef struct camera_s
{
	e_camera_type type;

	glm::vec4 position;

	// uvn
	glm::vec3 up, direction;

	float roll, yaw, pitch;

	e_cam_rotation_sequence euler_seq;

	glm::vec4 target;

	plane3d_t plane_top,
		plane_bottom,
		plane_left,
		plane_right;

	int fov;

	float plane_near_z, plane_far_z;
	float view_dist, view_dist_mul_ar;

	uint32_t viewport_w, viewport_h; // screen width, height
	float aspect_ratio;

	float viewplane_width, viewplane_height;

	glm::mat4 rot_mat;

	glm::mat4 cam_mat, per_mat, scr_mat;
} camera_t;


typedef struct material_s
{
	int attr;

	char name[1024];

	float k_ambient,
		k_diffuse,
		k_specular;

	u_rgba_color k_ambient_color,
		k_diffuse_color,
		k_specular_color;

	u_rgba_color color;
	rgba_color_t alpha_color;

	e_shade_model shade_model;

	e_texture_mode texture_mode;

	bool is_wireframe;

	texture::texture2d_c* texture2d;

	// helpers for renders. don't touch it
	uint32_t* current_alpha_table_src;
	uint32_t* current_alpha_table_dst;

} material_t;


typedef struct vertex3df_s
{
#if 0
	uint8_t attr;
#endif

	glm::vec4 pos;
	glm::vec4 normal;
	glm::vec2 uv;

	rgb_color_t color;

	uint32_t bone_ids[4];
	float bone_weights[4];

} vertex3df_t;


typedef struct poly3df_s
	: public list_node_s<poly3df_s>
{
	int attr;
	int state;

	vertex3df_t vertex_list_local[3];
	vertex3df_t vertex_list_tran[3];

	rgb_color_t color;

	bool is_normal_valid = false;
	glm::vec4 normal;

	const material_t* material;

} poly3df_t;


typedef struct render_buffer_s
	: public list_node_s<render_buffer_s>
{
	size_t poly_count;

	size_t buffer_size;

	poly3df_t* poly_data;
	poly3df_t** poly_data_ptrs;
} render_buffer_t;


typedef struct render_list_s
	: public linked_list_s<poly3df_t>
{
	int poly_count;
} render_list_t;


typedef struct light_source_s
{
	e_light_type light_type;

	glm::vec4 position_local, position_tran;
	glm::vec4 direction_local, direction_tran;

	u_rgba_color i_ambient,
		i_diffuse,
		i_specular;

	float kc, kl, kq;
} light_source_t;


typedef struct
{
	uint32_t viewport_w, viewport_h; // screen width, height
	uint8_t depth_in_bits;

	size_t size_in_bytes;
	float* zbuffer_data;
} zbuffer_t;

}
