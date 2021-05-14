#include "stdafx.h"
#include "modifier_object3d_c.h"

#include "mesh_factory_c.h"

modifier_object3d_c::modifier_object3d_c(e_game_modifier modifier_type)
{
	m_modifier_type = modifier_type;

	sr::texture::texture2d_c* modifier_texture = nullptr;

	resource_manager_c::e_texture_kind texture_kind = resource_manager_c::e_texture_no_texture;

	if (m_modifier_type == e_modifier_explosion_power)
		texture_kind = resource_manager_c::e_texture_modifier_explosion_power;
	else if (m_modifier_type == e_modifier_bomb_amount)
		texture_kind = resource_manager_c::e_texture_modifier_bomb_amount;
	//else if (m_modifier_type == e_modifier_character_speed)
	//	texture_kind = resource_manager_c::e_texture_modifier_character_speed;

	modifier_texture = resource_manager_c::me()->get_texture(texture_kind);

	// material
	{
		m_material.shade_model = sr::e_sm_noshade;
		m_material.is_wireframe = false;
		m_material.texture2d = modifier_texture;
		m_material.texture_mode = sr::e_tm_affine;
		m_material.k_ambient =
			m_material.k_diffuse =
			m_material.k_specular = 1.0f;
		m_material.k_ambient_color = { 255, 255, 255, 255 };
		m_material.k_diffuse_color = { 255, 255, 255, 255 };
		m_material.k_specular_color.color = XRGB(0, 0, 0);
		m_material.alpha_color = RGBA(0, 0, 0, 0);

		m_material.attr = m_material.shade_model
			| m_material.texture_mode
			| POLY_ATTR_ZBUFFER
			| POLY_ATTR_2SIDED
			| POLY_ATTR_SIMPLE_ALPHA_CHANNEL;
	}

	m_mesh = mesh_factory_c::generate_plane(1.0f, 1.0f, true, &m_material);

	assign_mesh(m_mesh);

	set_game_type(e_game_obj_type::e_gt_modifyer);

	use_entity_list_copy(false);

	set_scale(m_square_dimensions, m_square_dimensions, m_square_dimensions);

	set_x_angle(-90.0f);

	//
	auto ticks = SDL_GetTicks();
	m_ticks_shift_to_0 = 360.0f - (int)(ticks * m_fluctuations_speed_factor) % 360;
}


modifier_object3d_c::~modifier_object3d_c()
{
}


e_game_modifier modifier_object3d_c::get_modifier_type() const
{
	return m_modifier_type;
}


void modifier_object3d_c::update(bool update_normals, bool update_bounding_box)
{
	game_object_c::update(update_normals, update_bounding_box);

	float half_square_dimensions = m_square_dimensions * 0.5f;

	//
	auto ticks = SDL_GetTicks();

	int period = ((int)(ticks * m_fluctuations_speed_factor + m_ticks_shift_to_0)) % 360;
	float y_pos = abs(sin(SR_DEG_TO_RAD(period))) * (m_fluctuations_altitude - half_square_dimensions);

	set_y_pos(y_pos + half_square_dimensions);

	set_z_angle(ticks * m_rotation_speed_factor);
}
