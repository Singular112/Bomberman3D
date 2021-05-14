#include "game_object_c.h"

#include <engine3d/engine.h>

void fast_entity_copy(mesh_base_c::entity_t& dest,
	const mesh_base_c::entity_t& src)
{
	dest.material_index = src.material_index;

	size_t vert_count = src.vertices.size();
	sr::fast_memcpy(dest.vertices.data(),
		src.vertices.data(),
		vert_count * sizeof(decltype(src.vertices)::value_type));

	size_t poly_count = src.polygons.size();
	sr::fast_memcpy(dest.polygons.data(),
		src.polygons.data(),
		poly_count * sizeof(decltype(src.polygons)::value_type));

	size_t edg_count = src.vertices_common_edges.size();
	sr::fast_memcpy(dest.vertices_common_edges.data(),
		src.vertices_common_edges.data(),
		edg_count * sizeof(decltype(src.vertices_common_edges)::value_type));
}


game_object_c::game_object_c(bool use_own_render_buffer)
	: object3d_c(use_own_render_buffer)
{
}


game_object_c::~game_object_c()
{
}


void game_object_c::set_game_type(e_game_obj_type type)
{
	m_game_type = type;
}


e_game_obj_type game_object_c::get_game_type() const
{
	return m_game_type;
}


void game_object_c::use_entity_list_copy(bool use_copy)
{
	m_use_entity_list_copy = use_copy;
}


void game_object_c::assign_mesh(mesh_base_c* mesh)
{
	object3d_c::assign_mesh(mesh);

#if 0	// in this version work correct only for static objects
	object3d_c::compute_bounding_box();
#endif

	if (m_use_entity_list_copy)
	{
		set_entity_list(&m_entities_transformed_copy);

		// prepare, for fast copies in future
		prepare_entity_copies();
	}
}


void game_object_c::update(bool update_normals, bool update_bounding_box)
{
	// update base object3d
	object3d_c::update(update_normals, update_bounding_box);

	// make entity-list copy of updated object
	if (m_use_entity_list_copy)
	{
		//SCOPED_FUNCTION_MEASURE;

		auto sz = m_entities_transformed.size();

		for (size_t i = 0; i < sz; ++i)
		{
			const auto& src = object3d_c::m_entities_transformed[i];
			auto& dest = m_entities_transformed_copy[i];

			prepare_entity_copies();
			fast_entity_copy(dest, src);
		}
	}
}


void game_object_c::update_ashfalls(camera_t* cam_ptr)
{
	// update clipping & ashfalls at last
	auto it = m_ashfalls.begin();
	while (it != m_ashfalls.end())
	{
		auto& ashfall = *it;

		if (ashfall.is_reach_end())
		{
			it = m_ashfalls.erase(it);
			continue;
		}

		ashfall.update(cam_ptr);

		it++;
	}
}


void game_object_c::ashfall_fill_renderbuffer(sr::render_buffer_t& render_buffer)
{
	for (auto& ashfall : m_ashfalls)
	{
		ashfall.fill_renderbuffer(render_buffer);
	}
}


void game_object_c::clip_polygon_by_y_plane(sr::render_buffer_t* render_buffer,
	sr::poly3df_t* poly,
	float y_plane)
{
	// short names
	auto& v0 = poly->vertex_list_tran[0];
	auto& v1 = poly->vertex_list_tran[1];
	auto& v2 = poly->vertex_list_tran[2];

	//bool vlist_clip_status[3] = { false, false, false };

	uint8_t vlist_outside_indexes[3];
	uint8_t vlist_inside_indexes[3];

	uint8_t num_vertexes_clipped = 0;
	uint8_t num_vertexes_inside = 0;

	if (v0.pos.y < y_plane)
	{
		//vlist_clip_status[0] = true;

		vlist_outside_indexes[num_vertexes_clipped++] = 0;
	}
	else
	{
		vlist_inside_indexes[num_vertexes_inside++] = 0;
	}

	if (v1.pos.y < y_plane)
	{
		//vlist_clip_status[1] = true;

		vlist_outside_indexes[num_vertexes_clipped++] = 1;
	}
	else
	{
		vlist_inside_indexes[num_vertexes_inside++] = 1;
	}

	if (v2.pos.y < y_plane)
	{
		//vlist_clip_status[2] = true;

		vlist_outside_indexes[num_vertexes_clipped++] = 2;
	}
	else
	{
		vlist_inside_indexes[num_vertexes_inside++] = 2;
	}

	if (num_vertexes_clipped == 3)
	{
		poly->attr = POLY_ATTR_CLIPPED;
		return;
	}

	if (num_vertexes_clipped == 0)
	{
		return;
	}

	else if (num_vertexes_clipped == 1)
	{
		// originals vertexes
		auto& v0_outside_original = poly->vertex_list_tran[vlist_outside_indexes[0]];

		const auto& v1_inside_original = poly->vertex_list_tran[vlist_inside_indexes[0]];
		const auto& v2_inside_original = poly->vertex_list_tran[vlist_inside_indexes[1]];

		// copy
		const auto v0_outside_copy = poly->vertex_list_tran[vlist_outside_indexes[0]];

		// first side clip math

		// p = p0 + (p2 - p0) * t, 0 <= t <= 1
		// t = (p - p0) / (p2 - p0)

		glm::vec4 first_side_vector;
		first_side_vector = v2_inside_original.pos - v0_outside_original.pos;

		float first_side_t = (y_plane - v0_outside_original.pos.y)
			/ first_side_vector.y;

		v0_outside_original.pos.x = v0_outside_original.pos.x + first_side_vector.x * first_side_t;
		v0_outside_original.pos.y = y_plane;
		v0_outside_original.pos.z = v0_outside_original.pos.z + first_side_vector.z * first_side_t;

		// second side clip math
		glm::vec4 second_side_vector;
		second_side_vector = v1_inside_original.pos - v0_outside_copy.pos;

		float second_side_t = (y_plane - v0_outside_copy.pos.y)
			/ second_side_vector.y;

		sr::poly3df_t new_poly;
		{
			new_poly.attr = poly->attr;
			new_poly.state = poly->state;
			new_poly.material = poly->material;
			new_poly.color = poly->color;
		}

		if (poly->attr & POLY_ATTR_AFFINE_TEXTURE || poly->attr & POLY_ATTR_PERSPECTIVE_CORRECT_TEXTURE)
		{
			v0_outside_original.uv.x = v0_outside_original.uv.x
				+ (v2_inside_original.uv.x - v0_outside_original.uv.x) * first_side_t;

			v0_outside_original.uv.y = v0_outside_original.uv.y
				+ (v2_inside_original.uv.y - v0_outside_original.uv.y) * first_side_t;

			new_poly.vertex_list_tran[0].uv.x = v0_outside_copy.uv.x
				+ (v1_inside_original.uv.x - v0_outside_copy.uv.x) * second_side_t;

			new_poly.vertex_list_tran[0].uv.y = v0_outside_copy.uv.y
				+ (v1_inside_original.uv.y - v0_outside_copy.uv.y) * second_side_t;
		}

		if (poly->attr & POLY_ATTR_SHADE_GOURAUD)
		{
			uint32_t int_color_r, int_color_g, int_color_b;

			float v_color_r = POP_R(v0_outside_original.color)
				+ (POP_R(v2_inside_original.color) - POP_R(v0_outside_original.color)) * first_side_t;

			float v_color_g = POP_G(v0_outside_original.color)
				+ (POP_G(v2_inside_original.color) - POP_G(v0_outside_original.color)) * first_side_t;

			float v_color_b = POP_B(v0_outside_original.color)
				+ (POP_B(v2_inside_original.color) - POP_B(v0_outside_original.color)) * first_side_t;

			FAST_FLOAT_TO_INT32(v_color_r, int_color_r);
			FAST_FLOAT_TO_INT32(v_color_g, int_color_g);
			FAST_FLOAT_TO_INT32(v_color_b, int_color_b);

			v0_outside_original.color = XRGB(int_color_r, int_color_g, int_color_b);

			// new vertex color compute
			auto& new_vertex_color = new_poly.vertex_list_tran[0].color;
			v_color_r = POP_R(v0_outside_copy.color)
				+ (POP_R(v1_inside_original.color) - POP_R(v0_outside_copy.color)) * second_side_t;

			v_color_g = POP_G(v0_outside_copy.color)
				+ (POP_G(v1_inside_original.color) - POP_G(v0_outside_copy.color)) * second_side_t;

			v_color_b = POP_B(v0_outside_copy.color)
				+ (POP_B(v1_inside_original.color) - POP_B(v0_outside_copy.color)) * second_side_t;

			FAST_FLOAT_TO_INT32(v_color_r, int_color_r);
			FAST_FLOAT_TO_INT32(v_color_g, int_color_g);
			FAST_FLOAT_TO_INT32(v_color_b, int_color_b);

			new_vertex_color = XRGB(int_color_r, int_color_g, int_color_b);
		}

		new_poly.vertex_list_tran[0].pos.x = v0_outside_copy.pos.x + second_side_vector.x * second_side_t;
		new_poly.vertex_list_tran[0].pos.y = y_plane;
		new_poly.vertex_list_tran[0].pos.z = v0_outside_copy.pos.z + second_side_vector.z * second_side_t;

		m_clipped_points.emplace_back(v0_outside_original.pos);
		m_clipped_points.emplace_back(new_poly.vertex_list_tran[0].pos);

		fast_memcpy(&new_poly.vertex_list_tran[1],
			&v0_outside_original,
			sizeof(v0_outside_original));

		fast_memcpy(&new_poly.vertex_list_tran[2],
			&v1_inside_original,
			sizeof(v1_inside_original));

		sr::renderbuffer_push_poly(render_buffer, &new_poly);
	}

	else if (num_vertexes_clipped == 2)
	{
		// math:
		// p = p0 + (p1 - p0) * t, 0 <= t <= 1
		// t = (p - p0) / (p1 - p0)
		// x = p0.x + (p1.x - p0.x) * t
		// y = p0.y + (p1.y - p0.y) * t
		// z = p0.z + (p1.z - p0.z) * t

		auto& v0_inside = poly->vertex_list_tran[vlist_inside_indexes[0]];

		auto& v1_outside = poly->vertex_list_tran[vlist_outside_indexes[0]];
		auto& v2_outside = poly->vertex_list_tran[vlist_outside_indexes[1]];

		// first side clip math
		// t = (y - p0.y) / (p1.y - p0.y)

		glm::vec4 first_side_vector;
		first_side_vector = v1_outside.pos - v0_inside.pos;

		float first_side_t = (y_plane - v0_inside.pos.y)
			/ first_side_vector.y;

		v1_outside.pos.x = v0_inside.pos.x + first_side_vector.x * first_side_t;
		v1_outside.pos.y = y_plane;
		v1_outside.pos.z = v0_inside.pos.z + first_side_vector.z * first_side_t;

		// second side clip math
		// t = (y - p0.y) / (p2.y - p0.y)

		glm::vec4 second_side_vector;
		second_side_vector = v2_outside.pos - v0_inside.pos;

		float second_side_t = (y_plane - v0_inside.pos.y)
			/ second_side_vector.y;

		v2_outside.pos.x = v0_inside.pos.x + second_side_vector.x * second_side_t;
		v2_outside.pos.y = y_plane;
		v2_outside.pos.z = v0_inside.pos.z + second_side_vector.z * second_side_t;

		m_clipped_points.emplace_back(v1_outside.pos);
		m_clipped_points.emplace_back(v2_outside.pos);

		if (poly->attr & POLY_ATTR_AFFINE_TEXTURE || poly->attr & POLY_ATTR_PERSPECTIVE_CORRECT_TEXTURE)
		{
			v1_outside.uv.x = v0_inside.uv.x + (v1_outside.uv.x - v0_inside.uv.x) * first_side_t;
			v1_outside.uv.y = v0_inside.uv.y + (v1_outside.uv.y - v0_inside.uv.y) * first_side_t;

			v2_outside.uv.x = v0_inside.uv.x + (v2_outside.uv.x - v0_inside.uv.x) * second_side_t;
			v2_outside.uv.y = v0_inside.uv.y + (v2_outside.uv.y - v0_inside.uv.y) * second_side_t;
		}

		if (poly->attr & POLY_ATTR_SHADE_GOURAUD)
		{
			uint32_t int_color_r, int_color_g, int_color_b;

			float v_color_r = POP_R(v0_inside.color) + (POP_R(v1_outside.color) - POP_R(v0_inside.color)) * first_side_t;
			float v_color_g = POP_G(v0_inside.color) + (POP_G(v1_outside.color) - POP_G(v0_inside.color)) * first_side_t;
			float v_color_b = POP_B(v0_inside.color) + (POP_B(v1_outside.color) - POP_B(v0_inside.color)) * first_side_t;

			FAST_FLOAT_TO_INT32(v_color_r, int_color_r);
			FAST_FLOAT_TO_INT32(v_color_g, int_color_g);
			FAST_FLOAT_TO_INT32(v_color_b, int_color_b);

			v1_outside.color = XRGB(int_color_r, int_color_g, int_color_b);

			v_color_r = POP_R(v0_inside.color) + (POP_R(v2_outside.color) - POP_R(v0_inside.color)) * second_side_t;
			v_color_g = POP_G(v0_inside.color) + (POP_G(v2_outside.color) - POP_G(v0_inside.color)) * second_side_t;
			v_color_b = POP_B(v0_inside.color) + (POP_B(v2_outside.color) - POP_B(v0_inside.color)) * second_side_t;

			FAST_FLOAT_TO_INT32(v_color_r, int_color_r);
			FAST_FLOAT_TO_INT32(v_color_g, int_color_g);
			FAST_FLOAT_TO_INT32(v_color_b, int_color_b);

			v2_outside.color = XRGB(int_color_r, int_color_g, int_color_b);
		}
	}
}


void game_object_c::prepare_entity_copies()
{
#if 1
	m_entities_transformed_copy = m_entities_transformed;
#else
	auto ent_count = m_entities_transformed.size();

	m_entities_transformed_copy.resize(ent_count);

	for (size_t i = 0; i < ent_count; ++i)
	{
		auto& src = m_entities_transformed[i];
		auto& dest = m_entities_transformed_copy[i];

		dest.polygons.resize(src.polygons.size());
		dest.vertices.resize(src.vertices.size());
		dest.vertices_common_edges.resize(src.vertices_common_edges.size());
	}
#endif
}


size_t game_object_c::clip_by_y_plane(sr::render_buffer_t* render_buffer,
	float plane_y)
{
	m_clipped_points.clear();

	auto poly_count = render_buffer->poly_count;
	for (decltype(poly_count) i = 0; i < poly_count; ++i)
	{
		auto& poly = render_buffer->poly_data[i];

#if 0
		if (IS_POLYGON_DISCARDED(poly.attr))
			continue;
#endif

		clip_polygon_by_y_plane(render_buffer,
			&poly,
			plane_y);
	}

	size_t sz = m_clipped_points.size();
	for (size_t i = 0; i < sz; i += 2)
	{
		const auto& point1 = m_clipped_points[i];
		const auto& point2 = m_clipped_points[i + 1];

		ashfall_c new_ashfall(
			point1, point2,
			0,
			false);

		m_ashfalls.emplace_back(new_ashfall);
	}

	if (m_clipped_points.size() > 0)
		object3d_c::compute_constants();

	return sz;
}


size_t game_object_c::get_ashfalls_count()
{
	return m_ashfalls.size();
}


bool game_object_c::is_destroyed() const
{
	return m_is_destroyed;
}
