#pragma once

#include "mesh_base_c.h"

using namespace sr;

class sprite3d_c
	: public mesh_base_c
{
public:
	sprite3d_c(float width, float height,
		bool center_mesh,
		sr::material_t* material);

	virtual ~sprite3d_c();

	// set additional attributes for each polygon while fill renderlist
	void set_polygons_attributes(uint32_t attr);

	void set_forcibly_texture_mode(sr::e_texture_mode texture_mode);

	virtual void update(camera_t* cam_ptr);

	virtual void fill_renderlist(sr::render_list_t& render_list);

	// transformations
	void set_x_scale(float factor);
	void set_y_scale(float factor);
	void set_z_scale(float factor);

	void set_scale(float x_scale_factor,
		float y_scale_factor,
		float z_scale_factor);

	void move_forward(float speed);
	void move_backward(float speed);
	void strafe_left(float speed);
	void strafe_right(float speed);

	void set_x_pos(float pos);
	void set_y_pos(float pos);
	void set_z_pos(float pos);

	void set_pos(glm::vec3 pos);
	void set_pos(float x_pos, float y_pos, float z_pos);

	void move_x(float speed);
	void move_y(float speed);
	void move_z(float speed);

private:
	void compute_final_transform();
	void compute_scale_matrix();
	void compute_translation_matrix();

protected:
	// polygon additional attributes what be applied during filling render list
	uint32_t m_polygons_additional_attributes = 0;

	// current model transformations
	glm::mat4x4 m_scale_matrix;
	glm::mat4x4 m_rotation_matrix;
	glm::mat4x4 m_translation_matrix;
	glm::mat4x4 m_final_transform, m_prev_final_transform;
	bool m_transformation_changed;	// cache var. to determine if transformation changes between frames
	glm::vec3 m_scale, m_position;
};
