#pragma once

#include "game_object_c.h"

class modifier_object3d_c
	: public game_object_c
{
public:
	modifier_object3d_c(e_game_modifier modifier_type);
	virtual ~modifier_object3d_c();

	e_game_modifier get_modifier_type() const;

	virtual void update(bool update_normals = true, bool update_bounding_box = false);

private:
	mesh_base_c* m_mesh = nullptr;

	sr::material_t m_material;

	e_game_modifier m_modifier_type;

	float m_square_dimensions = 50.0f;
	float m_fluctuations_altitude = 100.0f;
	float m_fluctuations_speed_factor = 0.2f;
	float m_rotation_speed_factor = 0.3f;

	float m_ticks_shift_to_0; // to start modifier movement from floor
};
