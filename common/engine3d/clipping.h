#pragma once

#include "sr_types.h"

namespace sr
{

// clipping api
SOFTRENDERLIB_API void renderbuffer_clip_polygons(render_buffer_t* render_buffer,
	camera_t* cam);
SOFTRENDERLIB_API void renderbuffer_clip_polygons_llist(linked_list_s<render_buffer_t>* render_buffers,
	camera_t* cam);
//SOFTRENDERLIB_API void renderlist_clip_polygons(render_list_t* render_list,
//	camera_t* cam);

SOFTRENDERLIB_API void clip_polygon(render_buffer_t* render_buffer,
	poly3df_t* poly,
	camera_t* cam);
//SOFTRENDERLIB_API void clip_polygon(render_list_t* render_list,
//	poly3df_t* poly,
//	camera_t* cam);

SOFTRENDERLIB_API bool clip_line2d(point2df_t* v0, point2df_t* v1);

}
