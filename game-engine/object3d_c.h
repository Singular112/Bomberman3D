#pragma once

#include "mesh_base_c.h"

using namespace sr;


class object3d_c
{
public:
	enum e_animation_state
	{
		e_as_stop = 0,
		e_as_play,
		e_as_pause,
		e_as_blend
	};

	typedef std::vector<mesh_base_c::entity_t> entity_list_t;

	template <typename ValT = float>
	class bounding_box_c
	{
	public:	// no getters/setters
		ValT min_x, max_x;
		ValT min_y, max_y;
		ValT min_z, max_z;

	public:
		bounding_box_c()
		{
			initialize();
		}
		~bounding_box_c() {}

		void initialize()
		{
			min_x = min_y = min_z = std::numeric_limits<ValT>::max();
			max_x = max_y = max_z = std::numeric_limits<ValT>::lowest();
		}
	};

public:
	object3d_c(bool use_own_render_buffer = false);
	virtual ~object3d_c();

	virtual void assign_mesh(mesh_base_c* mesh);

	render_buffer_t* get_render_buffer();

	// set additional attributes for each polygon while fill renderlist
	void set_polygons_attributes(uint32_t attr);

	void set_forcibly_shade_model(e_shade_model shade_model);
	void set_forcibly_texture_mode(e_texture_mode texture_mode);

	virtual size_t calculate_polygon_count_total() const;

	mesh_base_c::material_library_t& get_base_material_library();
	mesh_base_c::texture_library_t& get_base_texture_library();

	virtual void set_material_library(mesh_base_c::material_library_t* new_library);
	virtual void set_texture_library(mesh_base_c::texture_library_t* new_library);

	virtual void update(bool update_normals = true, bool update_bounding_box = false);

	virtual void fill_renderbuffer();
	virtual void fill_renderbuffer(render_buffer_t& render_buffer);
	//virtual void fill_renderlist(render_list_t& render_list);

	void cull_backfaces(camera_t* cam);
	virtual void update_vertex_normals();
	virtual void compute_light(light_source_t* lights, size_t lights_count);

	virtual const bounding_box_c<>& compute_bounding_box();

	bool is_animated() const;

	int32_t get_frames_num(uint32_t animation_index) const;
	int32_t get_frames_num(const std::string& animation_name) const;

	int32_t get_animation_index_by_name(const std::string& name) const;

	uint32_t get_current_animation_index() const;

	std::vector<std::string> get_animations_list() const;

	int64_t get_animation_length_in_ms(uint32_t animation_index) const;
	float get_animation_length_in_secs(uint32_t animation_index) const;

	bool set_animation(const std::string& name);
	bool set_animation(uint32_t index);

	void set_animation_rate(float rate);
	void set_animation_repeats(int count);

	void start_animation();

	void pause_animation();

	void stop_animation();

	void restart_animation();

	void blend_animation(uint32_t new_animation_index,
		float blend_time);
	void reset_blend_animation();

	bool is_blend_finished(int check_animation_index);

	void set_animation_enable(bool state);

	void set_animation_state(e_animation_state state);

	e_animation_state get_animation_state() const;

	int64_t get_animation_cycle_count() const;

	// transformations
	void set_x_scale(float factor);
	void set_y_scale(float factor);
	void set_z_scale(float factor);

	void set_scale(float x_scale_factor,
		float y_scale_factor,
		float z_scale_factor);

	void set_x_angle(float angle_deg);
	void set_y_angle(float angle_deg);
	void set_z_angle(float angle_deg);

	void rotate_x(float angle_deg);
	void rotate_y(float angle_deg);
	void rotate_z(float angle_deg);

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

	void look_at(glm::vec3 pos);

private:
	void compute_final_transform();
	void compute_scale_matrix();
	void compute_rotation_matrix();
	void compute_translation_matrix();

	void update_blend(bool update_normals, bool update_bounding_box);

protected:
	// return base entity list from mesh_base_c
	std::vector<mesh_base_c::entity_t>& get_base_mesh_data() const;

	void set_entity_list(entity_list_t* new_entity_list);

	//void fill_renderlist(render_list_t& render_list,
	//	entity_list_t& entity_list);

	void compute_constants();

protected:
	mesh_base_c* m_assigned_mesh = nullptr;

	// resource copy for nice graphic effects
	mesh_base_c::material_library_t* m_current_material_set = nullptr;
	mesh_base_c::texture_library_t* m_current_textures_set = nullptr;

	bool m_use_own_render_buffer;
	render_buffer_t m_own_render_buffer;

	// local data of meshes after all transformations
	entity_list_t m_entities_transformed;

	// current entity list
	entity_list_t* m_current_entity_list = nullptr;

	// polygon additional attributes what be applied during filling render list
	uint32_t m_polygons_additional_attributes = 0;

	// ban animation function even if object presents it
	bool m_disabled_animation = false;

	uint32_t m_current_animation_index = 0,
		m_blend_animation_index = 0;

	float m_blend_factor = 0.0f;
	float m_blend_time = 0.0f,
		m_current_blend_time = 0.0f;
	bool m_blend_finished = true;

	mesh_base_c::timepoint_t m_animation_start_time,
		m_last_animation_timepoint,
		m_animation_blend_time;
	float m_current_animation_time;	// in seconds
	int64_t m_current_animation_cycle_count = 0;
	int64_t m_animation_max_cycle_count = 0;

	e_animation_state m_animation_state;
	float m_animation_rate;

	// current model transformations
	glm::mat4x4 m_scale_matrix;
	glm::mat4x4 m_rotation_matrix;
	glm::mat4x4 m_translation_matrix;
	glm::mat4x4 m_final_transform, m_prev_final_transform;
	bool m_transformation_changed;	// cache var. to determine if transformation changes between frames
	glm::vec3 m_scale, m_position;
	float m_x_angle_rad, m_y_angle_rad, m_z_angle_rad;

	//
	bool m_is_volume_shaded = false;

	//
	bounding_box_c<float> m_bounding_box;
};
