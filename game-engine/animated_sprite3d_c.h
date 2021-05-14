#pragma once

#include "sprite3d_c.h"

class animated_sprite3d_c
	: public sprite3d_c
{
	struct oneframe_uv_s
	{
		glm::vec2 v[4];
	};

public:
	animated_sprite3d_c(float width, float height,
		bool center_mesh,
		sr::material_t* material);

	virtual ~animated_sprite3d_c();

	void update(camera_t* cam_ptr);

protected:
	int32_t m_common_animation_duration_ms = 0;
	int32_t m_animation_loop_count = 0;

	std::vector<oneframe_uv_s> m_uv_texture_strip;

	int m_current_uv_frame_index = 0;

	int m_frames_count = 0;

	bool m_is_finished = false;
};
