#pragma once

#include "deadline_object3d_c.h"

class game_map3d_c;
class map_cell3d_c;

class explosion3d_c
{
	const int m_explosion_duration_ms = 750;

	typedef struct
	{
		mesh_base_c mesh;

		glm::mat4x4 scale_matrix,
			rotation_matrix,
			translation_matrix;

		glm::mat4x4 final_transform;
	} mesh_explosion_t;

	typedef std::unique_ptr<mesh_explosion_t> mesh_ptr_t;
	typedef std::unique_ptr<object3d_c> object3d_ptr_t;

	struct oneframe_uv_s
	{
		glm::vec2 v[4];
	};

public:
	explosion3d_c(game_map3d_c* game_map,
		map_cell3d_c* cell);

	void update(bool update_normals = true, bool update_bounding_box = false);

	void fill_renderbuffer(sr::render_buffer_t& render_buffer);

	bool is_finished() const;

	const glm::vec3& get_global_position() const;

	const glm::vec3& get_local_position() const;

	int get_width() const;

	float get_power() const;

protected:
	glm::vec3 m_global_center_pos, m_local_pos;

	int m_width;

	std::vector<mesh_ptr_t> m_planes_meshes;

	// polygon additional attributes what be applied during filling render list
	uint32_t m_polygons_additional_attributes = 0;

	std::vector<oneframe_uv_s> m_uv_texture_strip;

	int m_current_uv_frame_index = 0;

	int m_frames_count;

	bool m_is_finished = false;

	float m_power = 1.0f;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_create_timepoint;
};


class object_bomb3d_c
	: public deadline_object3d_c
{
public:
	object_bomb3d_c(int timeout_ms = 3000);

	void explode();

	void update(bool update_normals = true, bool update_bounding_box = false);
};
