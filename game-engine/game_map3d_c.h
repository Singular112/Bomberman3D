#pragma once

#include <functional>

#include "game_map_c.h"

using namespace sr;

class game_map3d_c :
	public game_map_c<map_cell3d_c>
{
	typedef std::unique_ptr<mesh_base_c> mesh_base_ptr_t;

public:
	enum e_mesh_type : int32_t
	{
		e_mt_none,

		e_mt_floor,
		e_mt_wall,
		e_mt_box
	};
	enum e_map_position_mode
	{
		e_mpm_by_center,
		e_mpm_by_left_top,
	};

public:
	game_map3d_c();
	virtual ~game_map3d_c();

	bool initialize_resources();

	void initialize_cell3d(map_cell3d_c& cell);

	void create(int x_cell_count, int y_cell_count,
		int cell_width, int cell_height);

	void set_absolute_position(const glm::vec4 pos,
		e_map_position_mode position_mode);

	glm::vec3 get_cell_position(int cell_x_index,
		int cell_y_index,
		bool get_center_position) const;

	glm::vec3 translate_local_pos_to_map_global_pos(const glm::vec3& local_pos,
		bool center) const;

	void place_game_object(map_cell3d_c* cell,
		e_game_obj_type type,
		e_game_modifier modifier_type = e_modifier_unknown);

	void add_on_boom_callback(std::function<void(map_cell3d_c*)> callback)
	{
		m_on_boom_callbacks.emplace_back(callback);
	}

	//

	void update();

	void render() {}

	void fill_renderbuffer(render_buffer_t& render_buffer);

protected:
	std::map<e_mesh_type, sr::material_t> m_material_library;

	std::map<e_mesh_type, mesh_base_ptr_t> m_mesh_library;

	float m_width = 0.0f, m_height = 0.0f;

	object3d_c m_plane_bottom_obj;

	glm::vec4 m_absolute_position;
	e_map_position_mode m_absolute_position_mode;

	//

	std::vector<std::function<void(map_cell3d_c*)>> m_on_boom_callbacks;
};
