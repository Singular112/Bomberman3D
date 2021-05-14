#include "stdafx.h"
#include "object_exit3d_c.h"
#include "mesh_factory_c.h"

object_exit3d_c::object_exit3d_c()
{
	m_mesh_ptr.reset(mesh_factory_c::generate_exit(64.0f, 128.0f));

	assign_mesh(m_mesh_ptr.get());

	enable_exit();

	set_game_type(e_game_obj_type::e_gt_exit);

	use_entity_list_copy(false);

	set_polygons_attributes(POLY_ATTR_ZBUFFER);

	set_scale(1.0f, 1.0f, 1.0f);

	if (is_animated())
	{
		set_animation("Armature|idle");
		set_animation_rate(1.0f);
	}
}


void object_exit3d_c::enable_exit()
{
	auto& material_library = m_mesh_ptr->get_material_library();

	material_library[1].texture2d = resource_manager_c::me()->get_texture
	(
		resource_manager_c::e_texture_exit_bottom_enabled
	);
}


void object_exit3d_c::disable_exit()
{
	auto& material_library = m_mesh_ptr->get_material_library();

	material_library[1].texture2d = resource_manager_c::me()->get_texture
	(
		resource_manager_c::e_texture_exit_bottom_disabled
	);
}
