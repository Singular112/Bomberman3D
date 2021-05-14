#include "stdafx.h"
#include "sprite3d_c.h"

#include <engine3d/sr_types.h>
#include <engine3d/sr_common.h>


sprite3d_c::sprite3d_c(float width, float height,
	bool center_mesh,
	sr::material_t* material)
{
	m_is_valid = true;

	m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	m_scale = glm::vec3(1.0f, 1.0f, 1.0f);

	compute_final_transform();

	m_scene_inverse_transform
		= glm::identity<decltype(m_scene_inverse_transform)>();

	m_entities.emplace_back();
	auto& entity = m_entities.back();

	if (center_mesh)
	{
		float half_width = width / 2.0f;
		float half_height = height / 2.0f;

		entity.vertices =
		{
			vertex3df_t { glm::vec4(-half_width, half_height, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(half_width, half_height, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(-half_width, -half_height, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(half_width, -half_height, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) }
		};
	}
	else
	{
		entity.vertices =
		{
			vertex3df_t { glm::vec4(0, 0, height, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, 0, height, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(width, 0, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) }
		};
	}

	entity.polygons =
	{
		{ 0, { 0, 1, 2 } },
		{ 0, { 1, 3, 2 } }
	};

	entity.material_index = 0;
	get_material_library().emplace_back(*material);
}


sprite3d_c::~sprite3d_c()
{
}


void sprite3d_c::set_polygons_attributes(uint32_t attr)
{
	m_polygons_additional_attributes = attr;
}


void sprite3d_c::set_forcibly_texture_mode(sr::e_texture_mode texture_mode)
{
	for (auto& material : get_material_library())
	{
		material.attr &= ~(material.texture_mode);
		material.attr |= texture_mode;
		material.texture_mode = texture_mode;
	}
}


void sprite3d_c::compute_scale_matrix()
{
	m_scale_matrix = glm::scale(m_scale);
}


void sprite3d_c::compute_final_transform()
{
	compute_scale_matrix();
	compute_translation_matrix();

	m_prev_final_transform = m_final_transform;

	m_final_transform =
		m_translation_matrix * m_rotation_matrix * m_scale_matrix;

	if (m_final_transform != m_prev_final_transform)
	{
		m_transformation_changed = true;
	}
}


void sprite3d_c::compute_translation_matrix()
{
	m_translation_matrix = glm::translate(m_position);
}


void sprite3d_c::update(camera_t* cam_ptr)
{
	// reset cache flag
	m_transformation_changed = false;

	// compute actual transforms & check for transformation changed
	m_rotation_matrix = glm::inverse(cam_ptr->rot_mat);
	compute_final_transform();
}


void sprite3d_c::fill_renderbuffer(sr::render_buffer_t& render_buffer)
{
	// fill renderlist from current entity list

	auto& current_polygon_index = render_buffer.poly_count;

	for (const auto& transformed_entity : m_entities)
	{
		const auto& material =
			get_material_library()[transformed_entity.material_index];

		for (const auto& poly : transformed_entity.polygons)
		{
			if (IS_POLYGON_DISCARDED(poly.attr))
				continue;

			if (current_polygon_index >= STATIC_RENDERLIST_POLYGONS_LIMIT)
				return;

			render_buffer.poly_data_ptrs[current_polygon_index]
				= &render_buffer.poly_data[current_polygon_index];

			// polygon attributes = material attributes + additional polygon attributes

			auto& curr_poly = render_buffer.poly_data[current_polygon_index];
			curr_poly.material = &material;
			curr_poly.attr = material.attr | POLY_ATTR_ZBUFFER;
			curr_poly.is_normal_valid = false;

			// copy vertices data
			for (uint32_t i = 0; i < 3; ++i)
			{
				auto index = poly.indices[i];

				const auto& cur_vert = transformed_entity.vertices[index];
				auto& trans_vert = curr_poly.vertex_list_tran[i];
				trans_vert.pos = m_final_transform * cur_vert.pos;
				trans_vert.uv = cur_vert.uv;
			}

			++current_polygon_index;
		}
	}
}


void sprite3d_c::set_x_scale(float factor)
{
	m_scale.x = factor;
}


void sprite3d_c::set_y_scale(float factor)
{
	m_scale.y = factor;
}


void sprite3d_c::set_z_scale(float factor)
{
	m_scale.z = factor;
}


void sprite3d_c::set_scale(float x_scale_factor,
	float y_scale_factor,
	float z_scale_factor)
{
	m_scale = glm::vec3(x_scale_factor, y_scale_factor, z_scale_factor);
}


void sprite3d_c::move_forward(float speed)
{
	//auto speed_vector = m_rotation_matrix * speed;
	//m_position += speed_vector;
}


void sprite3d_c::move_backward(float speed)
{

}


void sprite3d_c::strafe_left(float speed)
{

}


void sprite3d_c::strafe_right(float speed)
{

}


void sprite3d_c::set_x_pos(float pos)
{
	m_position.x = pos;
}


void sprite3d_c::set_y_pos(float pos)
{
	m_position.y = pos;
}


void sprite3d_c::set_z_pos(float pos)
{
	m_position.z = pos;
}


void sprite3d_c::set_pos(glm::vec3 pos)
{
	m_position = pos;
}


void sprite3d_c::set_pos(float x_pos, float y_pos, float z_pos)
{
	m_position = glm::vec3(x_pos, y_pos, z_pos);
}


void sprite3d_c::move_x(float speed)
{
	m_position.x += speed;
}


void sprite3d_c::move_y(float speed)
{
	m_position.y += speed;
}


void sprite3d_c::move_z(float speed)
{
	m_position.z += speed;
}
