#include "stdafx.h"
#include "object_bomb3d_c.h"
#include "mesh_factory_c.h"
#include "game_map3d_c.h"

explosion3d_c::explosion3d_c(game_map3d_c* game_map,
	map_cell3d_c* cell)
{
	m_is_finished = false;

	m_local_pos = cell->get_local_position();

	m_global_center_pos = game_map->translate_local_pos_to_map_global_pos
	(
		m_local_pos,
		true
	);

	// create explosion planes from top to down
	m_global_center_pos.y += game_map->get_cell_height();

	// explosion width
	m_width = game_map->get_cell_width();

	auto exp_strip_tex = resource_manager_c::me()->get_texture
	(
		resource_manager_c::e_texture_explosion
	);

	auto tex_width = exp_strip_tex->get_width();
	auto tex_height = exp_strip_tex->get_height();
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

	for (int i = 0; i < 4; i++)
	{
		material_t material;
		{
			material.texture_mode = sr::e_tm_affine;
			material.shade_model = sr::e_sm_noshade;

			material.attr = POLY_ATTR_ZBUFFER
				| material.shade_model
				| material.texture_mode
				| POLY_ATTR_ALPHA_CHANNEL
				| POLY_ATTR_SIMPLE_ALPHA_CHANNEL;

			material.is_wireframe = false;
			material.texture2d = exp_strip_tex;
			material.k_ambient =
				material.k_diffuse =
				material.k_specular = 1.0f;
			material.k_ambient_color = { 255, 255, 255, 255 };
			material.k_diffuse_color = { 255, 255, 255, 255 };
			material.k_specular_color.color = XRGB(0, 0, 0);
			material.alpha_color = RGBA(255, 255, 255, 0);

			set_material_alpha_level(&material, (4 - i) * 2);
		}

		mesh_ptr_t mesh_explosion(new mesh_explosion_t);

		// create mesh
		auto& mesh = mesh_explosion->mesh;
		auto& entities = mesh.m_entities;
		entities.emplace_back();
		auto& entity = entities.back();

		float half_width = 1.0f / 2.0f;
		float half_height = 1.0f / 2.0f;

		entity.vertices =
		{
			vertex3df_t { glm::vec4(-half_width, 0, half_height, 1.0f), glm::vec4() },
			vertex3df_t { glm::vec4(half_width, 0, half_height, 1.0f), glm::vec4() },
			vertex3df_t { glm::vec4(-half_width, 0, -half_height, 1.0f), glm::vec4() },
			vertex3df_t { glm::vec4(half_width, 0, -half_height, 1.0f), glm::vec4() }
		};

		entity.polygons =
		{
			{ 0, { 0, 1, 2 } },
			{ 0, { 1, 3, 2 } }
		};

		entity.material_index = 0;
		mesh.get_material_library().emplace_back(material);

		auto global_pos = m_global_center_pos;
		global_pos.y = global_pos.y - (3 - i) * (m_width / 4.0f) - 5;
		global_pos.x += 8;

		mesh_explosion->translation_matrix
			= glm::translate(global_pos);

		mesh_explosion->scale_matrix
			= glm::scale(glm::vec3(100.0f, 100.0f, 100.0f));

		m_planes_meshes.emplace_back(std::move(mesh_explosion));
	}

	m_create_timepoint = std::chrono::high_resolution_clock::now();
}


void explosion3d_c::update(bool update_normals, bool update_bounding_box)
{
	auto now_timepoint = std::chrono::high_resolution_clock::now();

	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>
	(
		now_timepoint - m_create_timepoint
	);
	uint64_t diff_ms = diff.count();

	auto frame_interval = m_explosion_duration_ms / m_frames_count;

	uint64_t last_frame_ms = m_explosion_duration_ms - frame_interval;

	m_power = 1.0f - diff_ms / (float)m_explosion_duration_ms;

	if (diff_ms >= last_frame_ms)
	{
		m_power = 0.0f;
		diff_ms = last_frame_ms;
		m_is_finished = true;

		return;
	}

	m_current_uv_frame_index = (int)floor(diff_ms / (float)frame_interval);

	auto& current_uv_frame = m_uv_texture_strip[m_current_uv_frame_index];

	for (auto& mesh_explosion : m_planes_meshes)
	{
		mesh_explosion->final_transform =
			mesh_explosion->translation_matrix *
			mesh_explosion->scale_matrix;

		auto& mesh = mesh_explosion->mesh;
		auto& entity = mesh.m_entities[0];

		for (int i = 0; i < 4; ++i)
		{
			auto& v_uv = entity.vertices[i].uv;
			{
				v_uv.x = current_uv_frame.v[i].x;
				v_uv.y = current_uv_frame.v[i].y;
			}
		}
	}
}


void explosion3d_c::fill_renderbuffer(sr::render_buffer_t& render_buffer)
{
	auto& current_polygon_index = render_buffer.poly_count;

	for (const auto& mesh : m_planes_meshes)
	{
		const auto& material = mesh->mesh.get_material_library()[0];
		const auto& entity = mesh->mesh.m_entities[0];

		for (const auto& poly : entity.polygons)
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
			curr_poly.attr = m_polygons_additional_attributes | material.attr;
			curr_poly.is_normal_valid = false;

			// copy vertices data
			for (uint32_t i = 0; i < 3; ++i)
			{
				auto index = poly.indices[i];

				const auto& cur_vert = entity.vertices[index];
				auto& trans_vert = curr_poly.vertex_list_tran[i];
				trans_vert.pos = mesh->final_transform * cur_vert.pos;
				trans_vert.uv = cur_vert.uv;
			}

			++current_polygon_index;
		}
	}
}


bool explosion3d_c::is_finished() const
{
	return m_is_finished;
}


const glm::vec3& explosion3d_c::get_global_position() const
{
	return m_global_center_pos;
}


const glm::vec3& explosion3d_c::get_local_position() const
{
	return m_local_pos;
}


int explosion3d_c::get_width() const
{
	return m_width;
}


float explosion3d_c::get_power() const
{
	return m_power;
}

// ===========================================================


object_bomb3d_c::object_bomb3d_c(int timeout_ms)
	: deadline_object3d_c(timeout_ms)
{
	mesh_base_c* mesh = mesh_factory_c::me()->get_mesh(
		mesh_factory_c::e_mesh_bomb);

	assign_mesh(mesh);

	set_game_type(e_game_obj_type::e_gt_bomb);

	use_entity_list_copy(false);

	set_polygons_attributes(POLY_ATTR_ZBUFFER);

	set_scale(0.5f, 0.5f, 0.5f);

	if (is_animated())
	{
		set_animation("Armature|idle");
		set_animation_rate(1.0f);
	}
}


void object_bomb3d_c::explode()
{
	m_current_dedline_object_state = e_dos_finished;
}


void object_bomb3d_c::update(bool update_normals, bool update_bounding_box)
{
	deadline_object3d_c::update(update_normals, update_bounding_box);

	//
	if (m_current_dedline_object_state == e_deadline_object_state::e_dos_stopped)
	{
		m_current_dedline_object_state = e_deadline_object_state::e_dos_continues;
	}
	else if (m_current_dedline_object_state == e_deadline_object_state::e_dos_finished)
	{
		m_is_destroyed = true;
	}
}
