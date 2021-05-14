#pragma once

#include "setup.h"

#include "sr_lib.h"

#include "sr_list.h"
#include "sr_math.h"
#include "sr_types.h"
#include "sr_common.h"
#include "frame_buffer.h"
#include "texture2d_c.h"
#include "render.h"
#include "materials.h"
#include "clipping.h"

namespace sr
{

using namespace sr::math;

SOFTRENDERLIB_API int initialize_sr_engine(int viewport_width, int viewport_height);
SOFTRENDERLIB_API void sr_free_resources();

SOFTRENDERLIB_API bool check_is_future_presented(e_system_futures fut);

SOFTRENDERLIB_API int get_max_light_source_count();
SOFTRENDERLIB_API light_source_t* get_light_source(int index);
SOFTRENDERLIB_API light_source_t* add_light_source();
SOFTRENDERLIB_API int get_light_sources_count();

SOFTRENDERLIB_API void set_1zbuffer_ptr(zbuffer_t*);
SOFTRENDERLIB_API zbuffer_t* get_1zbuffer_ptr();

#if 0
SOFTRENDERLIB_API void set_current_alpha_table_level(int level);
#endif
SOFTRENDERLIB_API void set_material_alpha_level(material_t* mat, int level);

SOFTRENDERLIB_API int get_fps();

SOFTRENDERLIB_API void set_clipplane_area(int x_min, int x_max,
	int y_min, int y_max);

SOFTRENDERLIB_API void camera_initialize(camera_t* cam,
	e_camera_type type,
	e_cam_rotation_sequence euler_seq,
	const glm::vec4* position,
	const glm::vec4* direction,
	float plane_near_z,
	float plane_far_z,
	int fov,
	int32_t viewport_w, int32_t viewport_h);

SOFTRENDERLIB_API void camera_rebuild_matrix(camera_t* cam);

SOFTRENDERLIB_API void camera_build_euler_matrix(camera_t* cam,
	e_cam_rotation_sequence seq);

SOFTRENDERLIB_API void camera_build_uvn_matrix(camera_t* cam);

SOFTRENDERLIB_API void camera_rotate(camera_t* cam,
	float yaw, float roll, float pitch);

SOFTRENDERLIB_API void camera_look_at(camera_t* cam,
	const glm::vec4& pos);

SOFTRENDERLIB_API void camera_set_yaw(camera_t* cam, float angle_deg);
SOFTRENDERLIB_API void camera_set_roll(camera_t* cam, float angle_deg);
SOFTRENDERLIB_API void camera_set_pitch(camera_t* cam, float angle_deg);

SOFTRENDERLIB_API void camera_move_forward(camera_t* cam, float speed);
SOFTRENDERLIB_API void camera_move_backward(camera_t* cam, float speed);
SOFTRENDERLIB_API void camera_strafe_left(camera_t* cam, float speed);
SOFTRENDERLIB_API void camera_strafe_right(camera_t* cam, float speed);

SOFTRENDERLIB_API void build_rotation_matrix_xyz(matrix4x4f_t* mat,
	float x_angle, float y_angle, float z_angle);

// frame buffer api
#if !defined(USE_SDL) && defined(WIN32)
SOFTRENDERLIB_API void draw_frame_buffer(const framebuffer_c* frame_buffer, HWND wnd);
#endif

// renderlist api
SOFTRENDERLIB_API void renderbuffer_initialize(render_buffer_t* render_buffer,
	size_t buffer_size);
SOFTRENDERLIB_API void renderbuffer_free(render_buffer_t* render_buffer);

SOFTRENDERLIB_API void renderlist_initialize(render_list_t* render_list);

SOFTRENDERLIB_API void renderbuffer_reset(render_buffer_t* render_buffer);
SOFTRENDERLIB_API void renderlist_reset(render_list_t* render_list);

SOFTRENDERLIB_API int renderbuffer_push_poly(render_buffer_t* render_buffer,
	const poly3df_t* poly);
SOFTRENDERLIB_API int renderlist_push_poly(render_list_t* render_list,
	const poly3df_t* poly);

SOFTRENDERLIB_API void renderbuffer_apply_transformation(render_buffer_t* render_buffer,
	const glm::mat4& mat,
	e_transform_mode transform_mode);

SOFTRENDERLIB_API void renderbuffer_apply_world_mat(render_buffer_t* render_buffer,
	const glm::vec4& pos,
	e_transform_mode transform_mode);

SOFTRENDERLIB_API void renderbuffer_apply_world_to_cam_mat(render_buffer_t* render_buffer,
	camera_t* cam);
SOFTRENDERLIB_API void renderbuffer_apply_world_to_cam_mat_llist(linked_list_s<render_buffer_t>* render_buffers,
	camera_t* cam);
SOFTRENDERLIB_API void renderlist_apply_world_to_cam_mat(render_list_t* render_list,
	camera_t* cam);

SOFTRENDERLIB_API void renderbuffer_apply_cam_to_perspective_screen(render_buffer_t* render_buffer,
	camera_t* cam);
SOFTRENDERLIB_API void renderbuffer_apply_cam_to_perspective_screen_llist(linked_list_s<render_buffer_t>* render_buffers,
	camera_t* cam);
SOFTRENDERLIB_API void renderlist_apply_cam_to_perspective_screen(render_list_t* render_list,
	camera_t* cam);

SOFTRENDERLIB_API void renderbuffer_z_sort(render_buffer_t* render_buffer,
	e_sort_mode mode);

SOFTRENDERLIB_API void renderbuffer_cull_backfaces(render_buffer_t* render_buffer,
	camera_t* cam);
SOFTRENDERLIB_API void renderbuffer_cull_backfaces_llist(linked_list_s<render_buffer_t>* render_buffers,
	camera_t* cam);
SOFTRENDERLIB_API void renderlist_cull_backfaces(render_list_t* render_list,
	camera_t* cam);

SOFTRENDERLIB_API void renderbuffer_apply_light(render_buffer_t* render_buffer, bool use_world_coords);
SOFTRENDERLIB_API void renderbuffer_apply_light_llist(linked_list_s<render_buffer_t>* render_buffers,
	bool use_world_coords);
SOFTRENDERLIB_API void renderlist_apply_light(render_list_t* render_list);

SOFTRENDERLIB_API void transform_lights(camera_t* cam,
	e_transform_mode transform_mode);

// z-buffer api
SOFTRENDERLIB_API bool initialize_zbuffer(zbuffer_t* zbuffer_ptr, camera_t* cam,
	uint8_t depth_in_bits);
SOFTRENDERLIB_API void free_zbuffer(zbuffer_t* zbuffer_ptr);
SOFTRENDERLIB_API void clear_zbuffer(zbuffer_t* zbuffer_ptr);

}
