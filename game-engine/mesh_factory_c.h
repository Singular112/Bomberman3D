#pragma once

#include <memory>

#include "mesh_base_c.h"

#include "helpers.h"

using namespace sr;


class mesh_factory_c
	: public singleton_c<mesh_factory_c>
{
	typedef std::unique_ptr<mesh_base_c> mesh_ptr_t;

public:
	enum e_mesh_type
	{
		e_mesh_bomberman,
		e_mesh_bomb,
		e_mesh_flower,
		e_mesh_devil,
		e_mesh_frog,
		e_mesh_exit
	};

	class cube_obj {};

public:
	mesh_factory_c();
	virtual ~mesh_factory_c();

	void initialize(const std::string& resource_dir);

	bool create_meshes(int num_threads);

	mesh_base_c* get_mesh(e_mesh_type type);

	template <class ObjT>
	static mesh_base_c* generate_mesh(float size,
		bool center_mesh,
		material_t* material);

	static mesh_base_c* generate_plane(float width, float height,
		bool center_mesh,
		material_t* material);

	static mesh_base_c* generate_exit(float width, float height);

private:
	std::string m_resource_dir;

	std::map<e_mesh_type, std::string> m_mesh_info;
	std::map<e_mesh_type, mesh_ptr_t> m_mesh_library;
};
