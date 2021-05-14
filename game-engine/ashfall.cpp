#include "stdafx.h"
#include "ashfall.h"

std::list<ash_s*> g_ash_collection;

ashfall_c::ashfall_c(const glm::vec4& v1,
	const glm::vec4& v2,
	float plane_y,
	bool leave_ash_on_floor)
{
	m_v0 = v1;
	m_v1 = v2;

	m_plane_y = plane_y;

	m_v = m_v1 - m_v0;

#if 0
	m_ash_count_limit = (int)(m_v.x / 2.0f);
#else
	m_ash_count_limit = 1;
#endif

	m_ash_count_reach_end = 0;

	m_leave_ash_on_floor = leave_ash_on_floor;

	m_finished = false;
}


void ashfall_c::update(camera_t* cam)
{
	if (m_finished)
	{
		return;
	}

	int abs_x_limit = (int)abs(m_v.x);

	if (abs_x_limit == 0)
	{
		m_finished = true;
		return;
	}

	auto ashtry_sz = m_ashtry.size();

	if (g_ash_collection.size() > 0 && ashtry_sz < m_ash_count_limit)
	{
		auto new_ash = g_ash_collection.front();
		g_ash_collection.pop_front();
		m_ashtry.push_back(new_ash);

		float sign = signbit(m_v.x) ? -1.0f : 1.0f;
		float rand_x = m_v0.x + (float)(rand() % abs_x_limit) * sign;

		new_ash->acceleration = 1.0f;

		// t = (x - p0.x) / (p1.x - p0.x)
		float t = (rand_x - m_v0.x) / m_v.x;

		new_ash->pos.x = m_v0.x + m_v.x * t;
		new_ash->pos.y = m_v0.y + m_v.y * t;
		new_ash->pos.z = m_v0.z + m_v.z * t;

		new_ash->sprite.set_pos(new_ash->pos);
	}

	auto it = m_ashtry.begin();

	while (it != m_ashtry.end())
	{
		auto& ash = *it;

		ash->sprite.update(cam);

		if (ash->pos.y <= m_plane_y)
		{
			ash->pos.y = m_plane_y;

			++m_ash_count_reach_end;

			if (m_ash_count_limit == m_ash_count_reach_end)
			{
				m_finished = true;
			}

			if (!m_leave_ash_on_floor || m_finished)
			{
				// return ash to ash collection
				g_ash_collection.emplace_back(ash);

				it = m_ashtry.erase(it);

				if (m_finished)
				{
					return;
				}

				continue;
			}
		}
		else
		{
			ash->pos.y -= ash->acceleration/* * 0.1f*/;
			ash->sprite.set_pos(ash->pos);

			float accel = 0.008f * (float)g_last_frame_duration;

			ash->acceleration += accel;
		}

		++it;
	}
}


void ashfall_c::fill_renderbuffer(sr::render_buffer_t& render_buffer)
{
	for (auto& ash : m_ashtry)
	{
		ash->sprite.fill_renderbuffer(render_buffer);
	}
}


bool ashfall_c::is_reach_end() const
{
	return m_finished;
}

// ================================================================

void initialize_ash_collection(std::list<ash_s*>& ash_collection)
{
	static sr::material_t material;
	{
		material.shade_model = sr::e_sm_shade_flat;
		material.is_wireframe = false;
		material.texture2d = nullptr;
		material.texture_mode = sr::e_tm_notextured;
		material.attr = material.shade_model | material.texture_mode;
		material.k_ambient =
			material.k_diffuse =
			material.k_specular = 1.0f;
		material.k_ambient_color = { 0, 0, 0, 0 };
		material.k_diffuse_color = { 60, 60, 60, 0 };
		material.k_specular_color.color = XRGB(0, 0, 0);
	}

	for (int i = 0; i < 10000; ++i)
	{
		ash_s* new_ash = new ash_s(10 / 4, 10 / 4, &material);
		{
			new_ash->acceleration = 1.0f;

			new_ash->sprite.set_polygons_attributes(POLY_ATTR_ZBUFFER);
		}
		ash_collection.emplace_back(new_ash);
	}
}


void free_ash_collection(std::list<ash_s*>& ash_collection)
{
	auto it = ash_collection.begin();

	while (it != ash_collection.end())
	{
		delete *it;

		//
		it = ash_collection.erase(it);
	}
}
