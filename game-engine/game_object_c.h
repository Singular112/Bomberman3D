#pragma once

#include "object3d_c.h"
#include "game_types.h"
#include "ashfall.h"

// stl
#include <vector>

class game_object_c
	: public object3d_c
{
public:
	game_object_c(bool use_own_render_buffer = false);
	virtual ~game_object_c();

	void set_game_type(e_game_obj_type type);
	e_game_obj_type get_game_type() const;

	void use_entity_list_copy(bool use_copy);

	void assign_mesh(mesh_base_c* mesh);

	void update(bool update_normals = true, bool update_bounding_box = false);
	void update_ashfalls(camera_t* cam_ptr);

	void ashfall_fill_renderbuffer(sr::render_buffer_t& render_buffer);

	size_t clip_by_y_plane(sr::render_buffer_t* render_buffer,
		float plane_y);

	size_t get_ashfalls_count();

	bool is_destroyed() const;

private:
	/*void clip_polygon_by_y_plane(mesh_base_c::entity_t& entity,
		mesh_base_c::mesh_face_t& face,
		float plane_y,
		std::vector<glm::vec4>& clipped_points);*/
	void clip_polygon_by_y_plane(sr::render_buffer_t* render_buffer,
		sr::poly3df_t* poly,
		float y_plane);

	void prepare_entity_copies();

protected:
	std::vector<mesh_base_c::entity_t> m_entities_transformed_copy;

	bool m_use_entity_list_copy = false;

	//
	e_game_obj_type m_game_type;

	bool m_is_destroyed = false;

	//
	std::vector<glm::vec4> m_clipped_points;
	std::list<ashfall_c> m_ashfalls;
};
