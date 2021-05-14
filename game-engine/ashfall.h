#pragma once

#include "sprite3d_c.h"

struct ash_s
{
	ash_s(float width, float height, material_t* material)
		: sprite(width, height, true, material)
	{
	}

	glm::vec4 pos;

	sprite3d_c sprite;

	float acceleration;
};


class ashfall_c
{
	const uint32_t m_color = 0x00000000;

public:
	ashfall_c(const glm::vec4& v1,
		const glm::vec4& v2,
		float plane_y,
		bool leave_ash_on_floor);

	void update(camera_t* cam);

	void fill_renderbuffer(sr::render_buffer_t& render_buffer);

	bool is_reach_end() const;

protected:
	glm::vec4 m_v, m_v0, m_v1;

	bool m_leave_ash_on_floor;

	float m_plane_y;

	size_t m_ash_count_limit;
	size_t m_ash_count_reach_end;

	std::list<ash_s*> m_ashtry;

	bool m_finished;
};


void initialize_ash_collection(std::list<ash_s*>& ash_collection);
void free_ash_collection(std::list<ash_s*>& ash_collection);
