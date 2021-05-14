#include "stdafx.h"
#include "cam_view_c.h"

cam_view_c::cam_view_c(e_camview_type view_type)
{
	m_camera_type = e_camera_type::e_ct_euler;
	m_view_type = view_type;
}


void cam_view_c::initialize(viewport_settings_s viewport_settings,
	int fov,
	float plane_near_z, float plane_far_z)
{
	camera_initialize
	(
		&m_cam,
		m_camera_type,
		e_cam_rotation_sequence::e_crs_zyx,
		&m_cam_pos,
		&m_cam_dir,
		plane_near_z,
		plane_far_z,
		fov,
		viewport_settings.screen_width,
		viewport_settings.screen_height
	);

	camera_rebuild_matrix(&m_cam);
}


void cam_view_c::set_type(e_camera_type cam_type)
{
	m_camera_type = cam_type;
}


void cam_view_c::set_position(const glm::vec4& cam_pos)
{
	m_cam_pos = cam_pos;
}


void cam_view_c::set_direction(const glm::vec4& cam_dir)
{
	m_cam_dir = cam_dir;
}


void cam_view_c::set_speed(float cam_speed)
{
	m_cam_speed = cam_speed;
}


glm::vec4 cam_view_c::get_position() const
{
	return m_cam_pos;
}


glm::vec4 cam_view_c::get_direction() const
{
	return m_cam_dir;
}


float cam_view_c::get_speed() const
{
	return m_cam_speed;
}


camera_t* cam_view_c::get_camera_ptr()
{
	return &m_cam;
}
