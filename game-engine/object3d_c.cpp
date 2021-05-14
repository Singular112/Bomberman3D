#include "object3d_c.h"

#include <engine3d/engine.h>

using namespace sr;


object3d_c::object3d_c(bool use_own_render_buffer)
{
	m_use_own_render_buffer = use_own_render_buffer;

	m_x_angle_rad = m_y_angle_rad = m_z_angle_rad = 0.0f;
	m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	m_scale = glm::vec3(1.0f, 1.0f, 1.0f);

	compute_final_transform();
}


object3d_c::~object3d_c()
{
	if (m_use_own_render_buffer)
	{
		renderbuffer_free(&m_own_render_buffer);
	}
}


void object3d_c::assign_mesh(mesh_base_c* mesh)
{
	if (mesh == nullptr)
		return;

	m_assigned_mesh = mesh;

	// make entity-list copy. all transforms will affect transformed entity-list
	m_entities_transformed = m_assigned_mesh->m_entities;

	m_current_material_set = &m_assigned_mesh->m_materials;
	m_current_textures_set = &m_assigned_mesh->m_textures;

	// current entity list by default points to local transformed entity-list
	m_current_entity_list = &m_entities_transformed;

	m_polygons_additional_attributes = 0;

	m_transformation_changed = true;
	m_final_transform = m_prev_final_transform = glm::identity<glm::mat4x4>();

	m_animation_rate = 1.0f;

	compute_constants();
	update_vertex_normals();

	m_animation_state = e_as_stop;
	m_disabled_animation = m_assigned_mesh->m_animations.size() == 0;

	if (m_use_own_render_buffer)
	{
		renderbuffer_initialize(&m_own_render_buffer,
			m_assigned_mesh->calculate_polygon_count_total() * 3);	// allocate a larger buffer size to store clipping data
	}
}


render_buffer_t* object3d_c::get_render_buffer()
{
	return &m_own_render_buffer;
}


void object3d_c::set_polygons_attributes(uint32_t attr)
{
	m_polygons_additional_attributes = attr;
}


void object3d_c::set_forcibly_shade_model(e_shade_model shade_model)
{
	auto& obj_materials = *m_current_material_set;
	for (auto& material : obj_materials)
	{
		material.attr &= ~(material.shade_model);
		material.attr |= shade_model;
		material.shade_model = shade_model;
	}
}


void object3d_c::set_forcibly_texture_mode(e_texture_mode texture_mode)
{
	auto& obj_materials = *m_current_material_set;
	for (auto& material : obj_materials)
	{
		material.attr &= ~(material.texture_mode);
		material.attr |= texture_mode;
		material.texture_mode = texture_mode;
	}
}


size_t object3d_c::calculate_polygon_count_total() const
{
	return m_assigned_mesh->calculate_polygon_count_total();
}


mesh_base_c::material_library_t& object3d_c::get_base_material_library()
{
	return m_assigned_mesh->get_material_library();
}


mesh_base_c::texture_library_t& object3d_c::get_base_texture_library()
{
	return m_assigned_mesh->get_texture_library();
}


void object3d_c::set_material_library(mesh_base_c::material_library_t* new_library)
{
	m_current_material_set = new_library;
}


void object3d_c::set_texture_library(mesh_base_c::texture_library_t* new_library)
{
	m_current_textures_set = new_library;
}


void object3d_c::update(bool update_normals, bool update_bounding_box)
{
	//SCOPED_FUNCTION_MEASURE;

	if (!m_assigned_mesh)
	{
		return;
	}

	// reset cache flag
	m_transformation_changed = false;

	// compute actual transforms & check for transformation changed
	compute_final_transform();

	if (is_animated() && m_animation_state != e_as_stop)
	{
		if (m_animation_state == e_as_blend)
		{
			update_blend(update_normals, update_bounding_box);

			return;
		}

		// dynamic object

		m_transformation_changed = true;

		if (m_animation_state == e_as_play)
		{
			// compute animation values

			auto current_timepoint = std::chrono::system_clock::now();

			std::chrono::duration<double> last_frame_time_diff =
				current_timepoint - m_last_animation_timepoint;

			m_last_animation_timepoint = current_timepoint;

			float animation_frame_time = (float)last_frame_time_diff.count();

			m_current_animation_time += animation_frame_time * m_animation_rate;

			auto anim_len_in_secs =
				get_animation_length_in_secs(m_current_animation_index);

			m_current_animation_cycle_count =
				(int64_t)floor(m_current_animation_time / anim_len_in_secs);

			if (m_animation_max_cycle_count > 0)
			{
				auto last_frame_time = m_animation_max_cycle_count * anim_len_in_secs;
				if (m_current_animation_time >= last_frame_time)
				{
					m_current_animation_time = last_frame_time - 0.01f;
				}
			}

			if (m_animation_max_cycle_count > 0 &&
				m_current_animation_cycle_count >= m_animation_max_cycle_count)
			{
				set_animation_state(e_as_pause);
			}
		}

		// compute transforms matrices for each bone
		std::vector<glm::mat4x4> transforms;
		{
			//SCOPED_MEASURE("bone_transform");

			m_assigned_mesh->bone_transform(m_current_animation_index,
				m_current_animation_time,
				transforms);
		}

		for (auto& transform : transforms)
		{
			transform = m_final_transform * transform;
		}

		size_t mesh_count = m_assigned_mesh->m_entities.size();

		// update each mesh, each vertex in face

		for (size_t mesh_index = 0; mesh_index < mesh_count; ++mesh_index)
		{
			const auto& entity = m_assigned_mesh->m_entities[mesh_index];
			auto& transformed_entity = m_entities_transformed[mesh_index];

			const auto entity_vertices = entity.vertices.data();
			auto transformed_entity_vertices = transformed_entity.vertices.data();

			if (entity.is_rigged)
			{
				for (const auto& face : transformed_entity.polygons)
				{
					const auto& bones_vertices_data
						= m_assigned_mesh->m_meshes_vertex_bones_map[mesh_index];

					for (uint32_t i = 0; i < 3; ++i)
					{
						auto index = face.indices[i];

						const auto& vertex_data = bones_vertices_data[index];

						auto bone0_index = vertex_data.id[0];
						auto bone1_index = vertex_data.id[1];
						auto bone2_index = vertex_data.id[2];
						auto bone3_index = vertex_data.id[3];

						auto bone0_weight = vertex_data.weight[0];
						auto bone1_weight = vertex_data.weight[1];
						auto bone2_weight = vertex_data.weight[2];
						auto bone3_weight = vertex_data.weight[3];

						glm::mat4x4 bone0_transform = transforms[bone0_index] * bone0_weight;
						glm::mat4x4 bone1_transform = transforms[bone1_index] * bone1_weight;
						glm::mat4x4 bone2_transform = transforms[bone2_index] * bone2_weight;
						glm::mat4x4 bone3_transform = transforms[bone3_index] * bone3_weight;
						glm::mat4x4 final_transform =
							bone0_transform
							+ bone1_transform
							+ bone2_transform
							+ bone3_transform;

						// apply final bone transformation for vertex

						const auto& cur_vert = entity_vertices[index];
						auto& trans_vert = transformed_entity_vertices[index];
						trans_vert.pos = final_transform * cur_vert.pos;
					}
				}
			}
			else
			{
				const auto& entity = m_assigned_mesh->m_entities[mesh_index];
				auto& transformed_entity = m_entities_transformed[mesh_index];

				const auto entity_vertices = entity.vertices.data();
				auto transformed_entity_vertices = transformed_entity.vertices.data();

				for (const auto& face : transformed_entity.polygons)
				{
					for (uint32_t i = 0; i < 3; ++i)
					{
						auto index = face.indices[i];

						const auto& cur_vert = entity_vertices[index];
						auto& trans_vert = transformed_entity_vertices[index];

						trans_vert.pos = m_final_transform * cur_vert.pos;
					}
				}
			}
		}
	}
	else if (m_transformation_changed)
	{
		// static object

		size_t mesh_count = m_assigned_mesh->m_entities.size();

		for (size_t mesh_index = 0; mesh_index < mesh_count; ++mesh_index)
		{
			const auto& entity = m_assigned_mesh->m_entities[mesh_index];
			auto& transformed_entity = m_entities_transformed[mesh_index];

			const auto entity_vertices = entity.vertices.data();
			auto transformed_entity_vertices = transformed_entity.vertices.data();

			for (const auto& face : transformed_entity.polygons)
			{
				for (uint32_t i = 0; i < 3; ++i)
				{
					auto index = face.indices[i];

					const auto& cur_vert = entity_vertices[index];
					auto& trans_vert = transformed_entity_vertices[index];

					trans_vert.pos = m_final_transform * cur_vert.pos;
				}
			}
		}
	}

	// update normals if need
	if (update_normals && m_transformation_changed && m_is_volume_shaded)
	{
		update_vertex_normals();
	}

	if (update_bounding_box)
	{
		compute_bounding_box();
	}
}


void object3d_c::update_blend(bool update_normals, bool update_bounding_box)
{
	/*OutputDebugStringA(std_string_printf("[%s] blend cycle. current animation: %d (%s). next animation: %d (%s) | m_blend_factor = %f\n", 1024,
		__FUNCTION__,
		m_current_animation_index,
		m_assigned_mesh->m_animations[m_current_animation_index].animation_name.c_str(),
		m_blend_animation_index,
		m_assigned_mesh->m_animations[m_blend_animation_index].animation_name.c_str(),
		m_blend_factor
		).c_str());*/

	// compute blend matrices between old & new animation frames

	auto current_timepoint = std::chrono::system_clock::now();

	m_transformation_changed = true;

	std::chrono::duration<double> last_frame_time_diff =
		current_timepoint - m_animation_blend_time;

	m_animation_blend_time = current_timepoint;

	float animation_frame_time = (float)last_frame_time_diff.count();

	m_current_blend_time += animation_frame_time;

	m_blend_factor = m_current_blend_time / m_blend_time;

	if (m_blend_factor >= 1.0f)
	{
		// blend finishes. set new animation
		m_blend_finished = true;

		m_blend_factor = 1.0f;

		/*OutputDebugStringA(std_string_printf("[%s] animation %d (%s) finished. set next animation %d (%s)\n", 1024,
			__FUNCTION__,
			m_current_animation_index,
			m_assigned_mesh->m_animations[m_current_animation_index].animation_name.c_str(),
			m_blend_animation_index,
			m_assigned_mesh->m_animations[m_blend_animation_index].animation_name.c_str()
			).c_str());*/

		restart_animation();
		set_animation(m_blend_animation_index);
	}

#if 0
	std::vector<glm::mat4x4> transforms_current;
	m_assigned_mesh->bone_transform(m_current_animation_index,
		m_current_animation_time,
		transforms_current);

	std::vector<glm::mat4x4> transforms_next;
	m_assigned_mesh->bone_transform(m_blend_animation_index,
		0.0f,	// seek to first animation frame of new animation
		transforms_next);

	auto tran_count = transforms_current.size();
	std::vector<glm::mat4x4> transforms_blended;
	transforms_blended.resize(tran_count);
	for (size_t i = 0; i < tran_count; ++i)
	{
		transforms_blended[i] = glm::mix(transforms_current[i],
			transforms_next[i], m_blend_factor);
	}
#else
	std::vector<glm::mat4x4> transforms_blended;
	m_assigned_mesh->bone_blend_transform(m_current_animation_index, m_blend_animation_index,
		m_current_animation_time, 0.0f,
		m_blend_factor,
		transforms_blended);
#endif

	for (auto& transform : transforms_blended)
	{
		transform = m_final_transform * transform;
	}

	size_t mesh_count = m_assigned_mesh->m_entities.size();

	for (size_t mesh_index = 0; mesh_index < mesh_count; ++mesh_index)
	{
		const auto& entity = m_assigned_mesh->m_entities[mesh_index];
		auto& transformed_entity = m_entities_transformed[mesh_index];

		if (!entity.is_rigged)
			continue;

		const auto entity_vertices = entity.vertices.data();
		auto transformed_entity_vertices = transformed_entity.vertices.data();

		for (const auto& face : entity.polygons)
		{
			const auto& bones_vertices_data
				= m_assigned_mesh->m_meshes_vertex_bones_map[mesh_index];

			for (uint32_t i = 0; i < 3; ++i)
			{
				auto index = face.indices[i];

				const auto& vertex_data = bones_vertices_data[index];

				auto bone0_index = vertex_data.id[0];
				auto bone1_index = vertex_data.id[1];
				auto bone2_index = vertex_data.id[2];
				auto bone3_index = vertex_data.id[3];

				auto bone0_weight = vertex_data.weight[0];
				auto bone1_weight = vertex_data.weight[1];
				auto bone2_weight = vertex_data.weight[2];
				auto bone3_weight = vertex_data.weight[3];

				glm::mat4x4 bone0_transform = transforms_blended[bone0_index] * bone0_weight;
				glm::mat4x4 bone1_transform = transforms_blended[bone1_index] * bone1_weight;
				glm::mat4x4 bone2_transform = transforms_blended[bone2_index] * bone2_weight;
				glm::mat4x4 bone3_transform = transforms_blended[bone3_index] * bone3_weight;
				glm::mat4x4 final_transform =
					bone0_transform
					+ bone1_transform
					+ bone2_transform
					+ bone3_transform;

				const auto& cur_vert = entity_vertices[index];
				auto& trans_vert = transformed_entity_vertices[index];
				trans_vert.pos = final_transform * cur_vert.pos;
			}
		}
	}

	// update normals if need
	if (m_is_volume_shaded)
	{
		update_vertex_normals();
	}

	if (update_bounding_box)
	{
		compute_bounding_box();
	}
}


void object3d_c::fill_renderbuffer()
{
	if (m_use_own_render_buffer/* && m_transformation_changed*/)
	{
		renderbuffer_reset(&m_own_render_buffer);

		fill_renderbuffer(m_own_render_buffer);
	}
}


void object3d_c::fill_renderbuffer(render_buffer_t& render_buffer)
{
	// fill renderlist from current entity list

	if (!m_current_entity_list)
	{
		return;
	}

	auto& current_polygon_index = render_buffer.poly_count;

	for (const auto& transformed_entity : *m_current_entity_list)
	{
		const auto& obj_materials = *m_current_material_set;

		const auto& material = obj_materials[transformed_entity.material_index];

		for (const auto& poly : transformed_entity.polygons)
		{
			if (IS_POLYGON_DISCARDED(poly.attr))
				continue;

			if (current_polygon_index >= render_buffer.buffer_size)
				return;

			render_buffer.poly_data_ptrs[current_polygon_index]
				= &render_buffer.poly_data[current_polygon_index];

			// polygon attributes = material attributes + additional polygon attributes

			auto& curr_poly = render_buffer.poly_data[current_polygon_index];
			curr_poly.material = &material;
			curr_poly.attr = m_polygons_additional_attributes | material.attr;
			curr_poly.is_normal_valid = false;

			// copy vertices data
			for (uint32_t i = 0; i < 3; ++i)
			{
				auto index = poly.indices[i];

				// tip: use memcpy instead of fast_memcpy. in this case it's much faster
				memcpy(&curr_poly.vertex_list_tran[i],
					&transformed_entity.vertices[index],
					sizeof(curr_poly.vertex_list_tran[i]));
			}

			++current_polygon_index;
		}
	}
}


void object3d_c::compute_light(light_source_t* lights, size_t lights_count)
{
	auto entity_count = m_assigned_mesh->m_entities.size();

	for (size_t mesh_index = 0; mesh_index < entity_count; ++mesh_index)
	{
		auto& entity = m_assigned_mesh->m_entities[mesh_index];
		auto& transformed_entity = m_current_entity_list->at(mesh_index);

		auto& obj_materials = *m_current_material_set;

		auto& material = obj_materials[entity.material_index];

		static const float color_div_factor = 1.0f / 255.0f;

		// vertices color
		float v_i_r_total, v_i_g_total, v_i_b_total;

		for (auto& vertex : transformed_entity.vertices)
		{
			v_i_r_total = v_i_g_total = v_i_b_total = 0.0f;

			// compute color for each vertex
			for (decltype(lights_count) li = 0; li < lights_count; ++li)
			{
				auto& light = lights[li];

				if (light.light_type == e_lt_ambient)
				{
					v_i_r_total += (material.k_ambient_color.r * light.i_ambient.r) * color_div_factor;
					v_i_g_total += (material.k_ambient_color.g * light.i_ambient.g) * color_div_factor;
					v_i_b_total += (material.k_ambient_color.b * light.i_ambient.b) * color_div_factor;
				}
				else if (light.light_type == e_lt_infinite)
				{
					// note: vertex.normal already normalized
					float intensity = glm::dot(vertex.normal, light.direction_local);

					if (intensity > 0.0f)
					{
						intensity *= color_div_factor;
						v_i_r_total += material.k_diffuse_color.r * light.i_diffuse.r * intensity;
						v_i_g_total += material.k_diffuse_color.g * light.i_diffuse.g * intensity;
						v_i_b_total += material.k_diffuse_color.b * light.i_diffuse.b * intensity;
					}
				}
				else if (light.light_type == e_lt_point)
				{
#if 0
					glm::vec3 normal = glm::normalize(vertex.normal);

					float intensity = glm::dot(normal, glm::vec3(light.direction_local));
#else
					float intensity = glm::fastNormalizeDot(vertex.normal, light.direction_local);
#endif

					if (intensity > 0.0f)
					{
						glm::vec3 light_vector = light.position_tran - vertex.pos;

						float dist = glm::fastLength(light_vector);
#ifdef USE_FAST_POINT_LIGHTS // fast
						float distance_factor = 1.0f / (light.kc + light.kl * dist);
#else // correct
						float distance_factor = 1.0f / (light.kq + light.kc + light.kl * dist * dist);
#endif

						intensity *= distance_factor;
						intensity *= color_div_factor;

						v_i_r_total += material.k_diffuse_color.r * light.i_diffuse.r * intensity;
						v_i_g_total += material.k_diffuse_color.g * light.i_diffuse.g * intensity;
						v_i_b_total += material.k_diffuse_color.b * light.i_diffuse.b * intensity;
					}
				}
#if 0
				else if (light.light_type == e_lt_lightspot)
				{
					// unsupported
				}
#endif
			}

			// apply final computed colors to vertex
			{
				int32_t int_i_r_total, int_i_g_total, int_i_b_total;

				FAST_FLOAT_TO_INT32(v_i_r_total, int_i_r_total);
				FAST_FLOAT_TO_INT32(v_i_g_total, int_i_g_total);
				FAST_FLOAT_TO_INT32(v_i_b_total, int_i_b_total);

				vertex.color = XRGB
				(
					FIND_MIN(int_i_r_total, 255),
					FIND_MIN(int_i_g_total, 255),
					FIND_MIN(int_i_b_total, 255)
				);
			}
		}
	}
}


const object3d_c::bounding_box_c<>& object3d_c::compute_bounding_box()
{
	//SCOPED_FUNCTION_MEASURE;

	compute_final_transform();

	for (auto& entity : *m_current_entity_list)
	{
		for (auto& polygon : entity.polygons)
		{
			auto v0 = entity.vertices[polygon.indices[0]];
			auto v1 = entity.vertices[polygon.indices[1]];
			auto v2 = entity.vertices[polygon.indices[2]];

			v0.pos = v0.pos * m_final_transform;
			v1.pos = v1.pos * m_final_transform;
			v2.pos = v2.pos * m_final_transform;

			// x
			if (v0.pos.x < m_bounding_box.min_x)
				m_bounding_box.min_x = v0.pos.x;
			if (v1.pos.x < m_bounding_box.min_x)
				m_bounding_box.min_x = v1.pos.x;
			if (v2.pos.x < m_bounding_box.min_x)
				m_bounding_box.min_x = v2.pos.x;

			if (v0.pos.x > m_bounding_box.max_x)
				m_bounding_box.max_x = v0.pos.x;
			if (v1.pos.x > m_bounding_box.max_x)
				m_bounding_box.max_x = v1.pos.x;
			if (v2.pos.x > m_bounding_box.max_x)
				m_bounding_box.max_x = v2.pos.x;

			// y
			if (v0.pos.y < m_bounding_box.min_y)
				m_bounding_box.min_y = v0.pos.y;
			if (v1.pos.y < m_bounding_box.min_y)
				m_bounding_box.min_y = v1.pos.y;
			if (v2.pos.y < m_bounding_box.min_y)
				m_bounding_box.min_y = v2.pos.y;

			if (v0.pos.y > m_bounding_box.max_y)
				m_bounding_box.max_y = v0.pos.y;
			if (v1.pos.y > m_bounding_box.max_y)
				m_bounding_box.max_y = v1.pos.y;
			if (v2.pos.y > m_bounding_box.max_y)
				m_bounding_box.max_y = v2.pos.y;

			// z
			if (v0.pos.z < m_bounding_box.min_z)
				m_bounding_box.min_z = v0.pos.z;
			if (v1.pos.z < m_bounding_box.min_z)
				m_bounding_box.min_z = v1.pos.z;
			if (v2.pos.z < m_bounding_box.min_z)
				m_bounding_box.min_z = v2.pos.z;

			if (v0.pos.z > m_bounding_box.max_z)
				m_bounding_box.max_z = v0.pos.z;
			if (v1.pos.z > m_bounding_box.max_z)
				m_bounding_box.max_z = v1.pos.z;
			if (v2.pos.z > m_bounding_box.max_z)
				m_bounding_box.max_z = v2.pos.z;
		}
	}

	return m_bounding_box;
}


void object3d_c::cull_backfaces(camera_t* cam)
{
	//SCOPED_FUNCTION_MEASURE;

	for (auto& entity : *m_current_entity_list)
	{
		for (auto& poly : entity.polygons)
		{
			if (IS_POLYGON_DISCARDED(poly.attr) ||
				poly.attr & POLY_ATTR_2SIDED)
			{
				continue;
			}

			const auto& poly_v0 = entity.vertices[poly.indices[0]];
			const auto& poly_v1 = entity.vertices[poly.indices[1]];
			const auto& poly_v2 = entity.vertices[poly.indices[2]];

			glm::vec3 v1 = poly_v1.pos - poly_v0.pos;
			glm::vec3 v2 = poly_v2.pos - poly_v0.pos;

			auto normal = glm::cross(v1, v2);

			glm::vec3 cam_vec = cam->position - poly_v0.pos;

			float dp = glm::dot(cam_vec, normal);

			if (dp <= 0.0f)
			{
				poly.attr |= POLY_ATTR_CULLED;

#if 0	// it's not working on practics :( cause common vertices in few polygons
				// to reduce light-calculations cost
				poly_v0.attr |= VERT_ATTR_CULLED;
				poly_v1.attr |= VERT_ATTR_CULLED;
				poly_v2.attr |= VERT_ATTR_CULLED;
#endif
			}
		}
	}
}


void object3d_c::compute_constants()
{
	m_is_volume_shaded = false;

	for (auto& entity : *m_current_entity_list)
	{
		auto& obj_materials = *m_current_material_set;

		const auto& shade_model = obj_materials[entity.material_index].shade_model;

		if (shade_model != e_shade_model::e_sm_noshade
			&& shade_model != e_shade_model::e_sm_shade_flat)
		{
			m_is_volume_shaded = true;
		}

		entity.vertices_common_edges.resize(entity.vertices.size());

		for (auto& face : entity.polygons)
		{
			auto index_v0 = face.indices[0];
			auto index_v1 = face.indices[1];
			auto index_v2 = face.indices[2];

			entity.vertices_common_edges[index_v0]++;
			entity.vertices_common_edges[index_v1]++;
			entity.vertices_common_edges[index_v2]++;
		}
	}
}


void object3d_c::update_vertex_normals()
{
	//SCOPED_FUNCTION_MEASURE;

	// compute smooth normals

	for (auto& transformed_entity : *m_current_entity_list)
	{
		// reset normals for all vertices
		for (auto& vertex : transformed_entity.vertices)
		{
			vertex.normal.x
				= vertex.normal.y
				= vertex.normal.z = 0.0f;
		}

		// compute new normals
		for (auto& face : transformed_entity.polygons)
		{
			if (IS_POLYGON_DISCARDED(face.attr))
				continue;

			auto index_v0 = face.indices[0];
			auto index_v1 = face.indices[1];
			auto index_v2 = face.indices[2];

			auto& v0_tran = transformed_entity.vertices[index_v0];
			auto& v1_tran = transformed_entity.vertices[index_v1];
			auto& v2_tran = transformed_entity.vertices[index_v2];

			glm::vec3 vec1 = v1_tran.pos - v0_tran.pos;
			glm::vec3 vec2 = v2_tran.pos - v0_tran.pos;

			glm::vec4 normal = glm::vec4(glm::cross(vec1, vec2), 1.0f);

			v0_tran.normal += normal;
			v1_tran.normal += normal;
			v2_tran.normal += normal;
		}

#if 1
		const auto& vertices_common_edges
			= transformed_entity.vertices_common_edges.data();

		auto sz = transformed_entity.vertices.size();
		for (size_t i = 0; i < sz; ++i)
		{
			auto& vertex = transformed_entity.vertices[i];

#if 0	// not do this!
			if (IS_VERTEX_DISCARDED(vertex.attr))
				continue;
#endif

#if 0	// not necessary
			if (vertices_common_edges[i] > 1)
				vertex.normal /= (float)vertices_common_edges[i];
#endif

			vertex.normal = glm::fastNormalize(vertex.normal);
		}
#endif
	}
}


std::vector<mesh_base_c::entity_t>& object3d_c::get_base_mesh_data() const
{
	return m_assigned_mesh->m_entities;
}


void object3d_c::set_entity_list(entity_list_t* new_entity_list)
{
	m_current_entity_list = new_entity_list;
}


bool object3d_c::is_animated() const
{
	return m_assigned_mesh->m_animations.size() > 0 &&
		!m_disabled_animation;
}


int32_t object3d_c::get_frames_num(uint32_t animation_index) const
{
	return animation_index < m_assigned_mesh->m_animations.size() ?
		m_assigned_mesh->m_animations[animation_index].node_anim_channel.size() :
		-1;
}


int32_t object3d_c::get_frames_num(const std::string& animation_name) const
{
	return get_frames_num(get_animation_index_by_name(animation_name));
}


int32_t object3d_c::get_animation_index_by_name(const std::string& name) const
{
	auto sz = m_assigned_mesh->m_animations.size();
	for (uint32_t i = 0; i < sz; i++)
	{
		if (m_assigned_mesh->m_animations[i].animation_name == name)
			return i;
	}

	return -1;
}


uint32_t object3d_c::get_current_animation_index() const
{
	return m_current_animation_index;
}


std::vector<std::string> object3d_c::get_animations_list() const
{
	std::vector<std::string> names;

	for (const auto& anim : m_assigned_mesh->m_animations)
	{
		names.emplace_back(anim.animation_name);
	}

	return names;
}


int64_t object3d_c::get_animation_length_in_ms(uint32_t animation_index) const
{
	if (animation_index >= m_assigned_mesh->m_animations.size())
		return -1;

	auto& animation = m_assigned_mesh->m_animations[animation_index];
	return (int64_t)(1000 * animation.duration / animation.ticks_per_second);
}


float object3d_c::get_animation_length_in_secs(uint32_t animation_index) const
{
	if (animation_index >= m_assigned_mesh->m_animations.size())
		return -1;

	auto& animation = m_assigned_mesh->m_animations[animation_index];
	return (float)(animation.duration / animation.ticks_per_second);
}


bool object3d_c::set_animation(const std::string& name)
{
	auto index = get_animation_index_by_name(name);
	return set_animation(index);
}


bool object3d_c::set_animation(uint32_t index)
{
	if (index < m_assigned_mesh->m_animations.size())
	{
		m_current_animation_cycle_count = 0;
		m_current_animation_index = index;
		return true;
	}

	return false;
}


void object3d_c::set_animation_rate(float rate)
{
	m_animation_rate = rate;
}


void object3d_c::set_animation_repeats(int count)
{
	m_animation_max_cycle_count = count;
}


void object3d_c::start_animation()
{
	set_animation_state(e_as_play);
}


void object3d_c::pause_animation()
{
	set_animation_state(e_as_pause);
}


void object3d_c::stop_animation()
{
	set_animation_state(e_as_stop);
}


void object3d_c::restart_animation()
{
	stop_animation();
	start_animation();
}


void object3d_c::blend_animation(uint32_t new_animation_index,
	float blend_time)
{
	if (!m_assigned_mesh)
	{
		return;
	}

	if (new_animation_index >= m_assigned_mesh->m_animations.size())
	{
		return;
	}

	if (!m_blend_finished)
	{
		m_blend_factor = 0.0f;

		m_blend_time = blend_time;

		m_blend_animation_index = new_animation_index;

		m_animation_blend_time = std::chrono::system_clock::now();

		return;
	}

	m_animation_state = e_as_blend;

	m_blend_factor = 0.0f;

	m_blend_time = blend_time;

	m_current_blend_time = 0.0f;

	m_blend_finished = false;

	m_blend_animation_index = new_animation_index;

	m_animation_blend_time = std::chrono::system_clock::now();
}


void object3d_c::reset_blend_animation()
{
	m_animation_state = e_as_stop;

	m_current_blend_time = 0.0f;

	m_blend_finished = true;

	m_blend_factor = 0.0f;
}


bool object3d_c::is_blend_finished(int check_animation_index)
{
	return m_blend_finished
		&& check_animation_index == m_blend_animation_index;
}


void object3d_c::set_animation_enable(bool state)
{
	m_disabled_animation = !state;
}


void object3d_c::set_animation_state(e_animation_state state)
{
	if (state == e_as_play)
	{
		if (m_animation_state == e_as_pause)
		{
			m_last_animation_timepoint = std::chrono::system_clock::now();
		}
		else if (m_animation_state == e_as_stop)
		{
			m_animation_start_time =
				m_last_animation_timepoint
				= std::chrono::system_clock::now();

			m_current_animation_time = 0.0f;
		}
	}
	else if (state == e_as_stop)
	{
		m_current_animation_time = 0.0f;
	}
	else if (state == e_as_pause)
	{
		if (m_animation_state == e_as_stop)
		{
			return;
		}
	}

	m_animation_state = state;
}


object3d_c::e_animation_state object3d_c::get_animation_state() const
{
	return m_animation_state;
}


int64_t object3d_c::get_animation_cycle_count() const
{
	return m_current_animation_cycle_count;
}


void object3d_c::compute_scale_matrix()
{
	m_scale_matrix = glm::scale(m_scale);
}


void object3d_c::compute_final_transform()
{
	compute_rotation_matrix();
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


void object3d_c::compute_rotation_matrix()
{
	m_rotation_matrix = glm::eulerAngleXYZ(m_x_angle_rad, m_y_angle_rad, m_z_angle_rad);
}


void object3d_c::compute_translation_matrix()
{
	m_translation_matrix = glm::translate(m_position);
}


void object3d_c::set_x_scale(float factor)
{
	m_scale.x = factor;
}


void object3d_c::set_y_scale(float factor)
{
	m_scale.y = factor;
}


void object3d_c::set_z_scale(float factor)
{
	m_scale.z = factor;
}


void object3d_c::set_scale(float x_scale_factor,
	float y_scale_factor, 
	float z_scale_factor)
{
	m_scale = glm::vec3(x_scale_factor, y_scale_factor, z_scale_factor);
}

void object3d_c::set_x_angle(float angle_deg)
{
	m_x_angle_rad = glm::radians(angle_deg);
}


void object3d_c::set_y_angle(float angle_deg)
{
	m_y_angle_rad = glm::radians(angle_deg);
}


void object3d_c::set_z_angle(float angle_deg)
{
	m_z_angle_rad = glm::radians(angle_deg);
}


void object3d_c::rotate_x(float angle_deg)
{
	m_x_angle_rad += glm::radians(angle_deg);
}


void object3d_c::rotate_y(float angle_deg)
{
	m_y_angle_rad += glm::radians(angle_deg);
}


void object3d_c::rotate_z(float angle_deg)
{
	m_z_angle_rad += glm::radians(angle_deg);
}


void object3d_c::move_forward(float speed)
{
	//auto speed_vector = m_rotation_matrix * speed;
	//m_position += speed_vector;
}


void object3d_c::move_backward(float speed)
{

}


void object3d_c::strafe_left(float speed)
{

}


void object3d_c::strafe_right(float speed)
{

}


void object3d_c::set_x_pos(float pos)
{
	m_position.x = pos;
}


void object3d_c::set_y_pos(float pos)
{
	m_position.y = pos;
}


void object3d_c::set_z_pos(float pos)
{
	m_position.z = pos;
}


void object3d_c::set_pos(glm::vec3 pos)
{
	m_position = pos;
}


void object3d_c::set_pos(float x_pos, float y_pos, float z_pos)
{
	m_position = glm::vec3(x_pos, y_pos, z_pos);
}


void object3d_c::move_x(float speed)
{
	m_position.x += speed;
}


void object3d_c::move_y(float speed)
{
	m_position.y += speed;
}


void object3d_c::move_z(float speed)
{
	m_position.z += speed;
}


void object3d_c::look_at(glm::vec3 pos)
{
	// todo: not working!
	m_rotation_matrix = glm::lookAt(m_position, m_position, pos);
	glm::extractEulerAngleXYZ(m_rotation_matrix,
		m_x_angle_rad,
		m_y_angle_rad,
		m_z_angle_rad);
}
