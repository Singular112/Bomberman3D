#pragma once

#include "sr_types.h"
#include "texture2d_c.h"

namespace sr
{

using namespace sr::texture;

SOFTRENDERLIB_API material_t* register_new_material(const char* mat_name,
	int attributes,
	u_rgba_color color,
	float k_ambient, float k_diffuse, float k_specular,
	texture2d_c* texture);

SOFTRENDERLIB_API texture2d_c* register_new_texture(uint32_t texture_id,
	const char* texture_path);

}
