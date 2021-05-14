#pragma once

struct viewport_settings_s
{
	int monitor_index;
	int screen_width;
	int screen_height;
	bool window_fullscreen_mode;
};


class cam_view_c
{
public:
	enum e_camview_type
	{
		e_camview_main_view
	};

public:
	cam_view_c(e_camview_type view_type);

public:
	void initialize(viewport_settings_s viewport_settings,
		int fov,
		float plane_near_z = 50.0f, float plane_far_z = 1000.0f);

	void set_type(e_camera_type cam_type);

	void set_position(const glm::vec4& cam_pos);

	void set_direction(const glm::vec4& cam_dir);

	void set_speed(float cam_speed);

	glm::vec4 get_position() const;

	glm::vec4 get_direction() const;

	float get_speed() const;

	camera_t* get_camera_ptr();

private:
	e_camview_type m_view_type;

	e_camera_type m_camera_type;

	camera_t m_cam;
	glm::vec4 m_cam_pos;
	glm::vec4 m_cam_dir = glm::vec4(0.0f, 0, 1, 1);
	float m_cam_speed = 0.5f;
};
