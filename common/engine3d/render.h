#pragma once

#include "sr_types.h"
#include "frame_buffer.h"

namespace sr
{

SOFTRENDERLIB_API void renderbuffer_render(render_buffer_t* render_buffer,
	framebuffer_c* frame_buffer);
SOFTRENDERLIB_API void renderbuffer_render_llist(linked_list_s<render_buffer_t>* render_buffers,
	framebuffer_c* frame_buffer);
SOFTRENDERLIB_API void renderlist_render(render_list_t* render_list,
	framebuffer_c* frame_buffer);

//

SOFTRENDERLIB_API void render_line_clipped(rgb_color_t color,
	framebuffer_c* frame_buffer,
	point2df_t* v0,
	point2df_t* v1);

//

template <typename PolyT>
void render_polygon_wireframe(PolyT* poly,
	framebuffer_c* frame_buffer);

//

template <typename PolyT>
void render_polygon_solid(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_solid_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

//

template <typename PolyT>
void render_polygon_affine_textured_noshade(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_affine_textured_noshade_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_affine_textured_shade_flat(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_affine_textured_shade_flat_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_affine_textured_noshade_alpha0(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_affine_textured_noshade_alpha0_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_affine_textured_shade_flat_alpha0(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_affine_textured_shade_flat_alpha0_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_affine_textured_noshade_alphablend(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_affine_textured_noshade_alphablend_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_affine_textured_noshade_alphablend0_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_affine_textured_shade_flat_alphablend(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_affine_textured_shade_flat_alphablend_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

//

template <typename PolyT>
void render_polygon_corr_textured_noshade_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_corr_textured_shade_flat_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_corr_textured_noshade_alpha0_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_corr_textured_shade_flat_alpha0_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_corr_textured_noshade_alphablend_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_corr_textured_shade_flat_alphablend_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

//

template <typename PolyT>
void render_polygon_gouraund(PolyT* poly,
	framebuffer_c* frame_buffer);

template <typename PolyT>
void render_polygon_gouraund_zb(PolyT* poly,
	framebuffer_c* frame_buffer);

}
