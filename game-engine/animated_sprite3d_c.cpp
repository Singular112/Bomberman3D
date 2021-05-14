#include "stdafx.h"
#include "animated_sprite3d_c.h"


animated_sprite3d_c::animated_sprite3d_c(float width, float height,
	bool center_mesh,
	sr::material_t* material)
	: sprite3d_c(width, height, center_mesh, material)
{
	auto tex = material->texture2d;

	auto tex_width = tex->get_width();
	auto tex_height = tex->get_height();
	auto frame_size = 64;

	int frames_per_width = tex_width / frame_size;
	int frames_per_height = tex_height / frame_size;

	m_frames_count = frames_per_width * frames_per_height;

	// fill uv for texture strip
	{
		for (int y = 0; y < frames_per_height; ++y)
		{
			for (int x = 0; x < frames_per_width; ++x)
			{
				oneframe_uv_s oneframe_uv;
				{
					oneframe_uv.v[0] = glm::vec2((float)(x + 0) / (float)frames_per_width,
						(float)(y + 0) / (float)frames_per_height);
					oneframe_uv.v[1] = glm::vec2((float)(x + 1) / (float)frames_per_width,
						(float)(y + 0) / (float)frames_per_height);
					oneframe_uv.v[2] = glm::vec2((float)(x + 0) / (float)frames_per_width,
						(float)(y + 1) / (float)frames_per_height);
					oneframe_uv.v[3] = glm::vec2((float)(x + 1) / (float)frames_per_width,
						(float)(y + 1) / (float)frames_per_height);
				}
				m_uv_texture_strip.emplace_back(oneframe_uv);
			}
		}
	}
}


animated_sprite3d_c::~animated_sprite3d_c()
{
}


void animated_sprite3d_c::update(camera_t* cam_ptr)
{
	sprite3d_c::update(cam_ptr);

	//

#if 0
	auto now_timepoint = std::chrono::high_resolution_clock::now();

	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>
	(
		now_timepoint - m_create_timepoint
	);
	auto diff_ms = diff.count();

	auto frame_interval = m_common_animation_duration_ms / m_frames_count;

	if (diff_ms >= (m_common_animation_duration_ms - frame_interval))
	{
		diff_ms = (m_common_animation_duration_ms - frame_interval);
		m_is_finished = true;

		return;
	}

	m_current_uv_frame_index = (int)floor(diff_ms / (float)frame_interval);

	auto& current_uv_frame = m_uv_texture_strip[m_current_uv_frame_index];

	auto& entity = m_entities[0];

	for (int i = 0; i < 4; ++i)
	{
		auto& v_uv = entity.vertices[i].uv;
		{
			v_uv.x = current_uv_frame.v[i].x;
			v_uv.y = current_uv_frame.v[i].y;
		}
	}
#endif
}
