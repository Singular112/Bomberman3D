#include "stdafx.h"
#include "mesh_factory_c.h"

#include "resource_manager_c.h"

//
mesh_factory_c::mesh_factory_c()
{
}


mesh_factory_c::~mesh_factory_c()
{
}


void mesh_factory_c::initialize(const std::string& resource_dir)
{
	m_resource_dir = resource_dir;

	m_mesh_info[e_mesh_bomberman]	= "/bomberman.fbx";
	m_mesh_info[e_mesh_bomb]		= "/bomb.fbx";
	m_mesh_info[e_mesh_flower]		= "/enemy_flower.fbx";
	m_mesh_info[e_mesh_devil]		= "/enemy_devil.fbx";
	//m_mesh_info[e_mesh_frog]		= "/enemy_frog.fbx";
	m_mesh_info[e_mesh_exit]		= "/exit.fbx";
}


bool mesh_factory_c::create_meshes(int num_threads)
{
	uint32_t flags =
		aiProcess_Triangulate
		| aiProcess_FlipUVs
		| aiProcess_OptimizeMeshes
		| aiProcess_ValidateDataStructure
		| aiProcess_RemoveRedundantMaterials
		| aiProcess_FixInfacingNormals
		| aiProcess_ImproveCacheLocality
		| aiProcess_LimitBoneWeights		// do not remove this line to prevent bugs in animations
		| aiProcess_JoinIdenticalVertices;

	for (const auto& mesh_info : m_mesh_info)
	{
		mesh_ptr_t mesh_ptr(new mesh_base_c);

		bool succ = mesh_ptr->load(m_resource_dir + mesh_info.second, flags);
		mesh_ptr->disable_alpha_channel_in_textures();

		if (!succ)
		{
			m_mesh_library.clear();
			return false;
		}

		mesh_ptr->m_name = mesh_info.second;

		m_mesh_library.emplace
		(
			std::make_pair(mesh_info.first, std::move(mesh_ptr))
		);
	}

	return true;
}


mesh_base_c* mesh_factory_c::get_mesh(e_mesh_type type)
{
	auto it = m_mesh_library.find(type);
	if (it != m_mesh_library.cend())
	{
		return it->second.get();
	}

	return nullptr;
}


template<>
mesh_base_c* mesh_factory_c::generate_mesh<mesh_factory_c::cube_obj>(float size,
	bool center_mesh,
	material_t* material)
{
	mesh_base_c* mesh = new mesh_base_c();

	mesh->m_is_valid = true;

	mesh->m_name = "cube";

	mesh->m_scene_inverse_transform
		= glm::identity<decltype(mesh->m_scene_inverse_transform)>();

	mesh->m_entities.emplace_back();
	auto& entity = mesh->m_entities.back();

	if (center_mesh)
	{
		float half_size = size / 2.0f;

		entity.vertices =
		{
			vertex3df_t { glm::vec4(-half_size, half_size, -half_size, 1.0f), glm::vec4(), glm::vec2(0, 2.0f/3.0f) },
			vertex3df_t { glm::vec4(-half_size, -half_size, -half_size, 1.0f), glm::vec4(), glm::vec2(0.25f, 2.0f/3.0f) },
			vertex3df_t { glm::vec4(half_size, half_size, -half_size, 1.0f), glm::vec4(), glm::vec2(0, 1.0f/3.0f) },
			vertex3df_t { glm::vec4(half_size, -half_size, -half_size, 1.0f), glm::vec4(), glm::vec2(0.25f, 1.0f/3.0f) },

			vertex3df_t { glm::vec4(-half_size, -half_size, half_size, 1.0f), glm::vec4(), glm::vec2(0.5f, 2.0f/3.0f) },
			vertex3df_t { glm::vec4(half_size, -half_size, half_size, 1.0f), glm::vec4(), glm::vec2(0.5f, 1.0f/3.0f) },
			vertex3df_t { glm::vec4(-half_size, half_size, half_size, 1.0f), glm::vec4(), glm::vec2(0.75f, 2.0f/3.0f) },
			vertex3df_t { glm::vec4(half_size, half_size, half_size, 1.0f), glm::vec4(), glm::vec2(0.75f, 1.0f/3.0f) },

			vertex3df_t { glm::vec4(-half_size, half_size, -half_size, 1.0f), glm::vec4(), glm::vec2(1.0f, 2.0f/3.0f) },
			vertex3df_t { glm::vec4(half_size, half_size, -half_size, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f/3.0f) },

			vertex3df_t { glm::vec4(-half_size, half_size, -half_size, 1.0f), glm::vec4(), glm::vec2(0.25f, 1.0f) },
			vertex3df_t { glm::vec4(-half_size, half_size, half_size, 1.0f), glm::vec4(), glm::vec2(0.5f, 1.0f) },

			vertex3df_t { glm::vec4(half_size, half_size, -half_size, 1.0f), glm::vec4(), glm::vec2(0.25f, 0.0f) },
			vertex3df_t { glm::vec4(half_size, half_size, half_size, 1.0f), glm::vec4(), glm::vec2(0.5f, 0.0f) }
		};

		entity.polygons =
		{
			{ 0, { 0, 2, 1 } },
			{ 0, { 1, 2, 3 } },
			{ 0, { 4, 5, 6 } },
			{ 0, { 5, 7, 6 } },
			{ 0, { 6, 7, 8 } },
			{ 0, { 7, 9, 8 } },
			{ 0, { 1, 3, 4 } },
			{ 0, { 3, 5, 4 } },
			{ 0, { 1, 11, 10 } },
			{ 0, { 1, 4, 11 } },
			{ 0, { 3, 12, 5 } },
			{ 0, { 5, 12, 13 } }
		};
	}
	else
	{
#if 1
		entity.vertices =
		{
			// front
			vertex3df_t { glm::vec4(0, size, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(size, size, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(size, 0, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },

			// back
			vertex3df_t { glm::vec4(0, 0, size, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },
			vertex3df_t { glm::vec4(size, 0, size, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(0, size, size, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(size, size, size, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },

			// up
			vertex3df_t { glm::vec4(0, size, size, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(size, size, size, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, size, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(size, size, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },

			// bottom
			vertex3df_t { glm::vec4(0, 0, size, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(size, 0, size, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },
			vertex3df_t { glm::vec4(0, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(size, 0, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },

			// left
			vertex3df_t { glm::vec4(0, size, size, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, 0, size, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(0, size, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, 0, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },

			// right
			vertex3df_t { glm::vec4(size, size, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(size, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(size, size, size, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(size, 0, size, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },
		};

		entity.polygons =
		{
			{ 0, { 0, 2, 1 } },
			{ 0, { 1, 2, 3 } },

			{ 0, { 4, 5, 6 } },
			{ 0, { 5, 7, 6 } },

			{ 0, { 8, 9, 10 } },
			{ 0, { 9, 11, 10 } },

			{ 0, { 12, 14, 13 } },
			{ 0, { 13, 14, 15 } },

			{ 0, { 16, 18, 17 } },
			{ 0, { 17, 18, 19 } },

			{ 0, { 20, 22, 21 } },
			{ 0, { 21, 22, 23 } }
		};
#else
		entity.vertices =
		{
			vertex3df_t { glm::vec4(0, size, 0, 1.0f), glm::vec4(), glm::vec2(0, 2.0f/3.0f) },
			vertex3df_t { glm::vec4(0, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.25f, 2.0f/3.0f) },
			vertex3df_t { glm::vec4(size, size, 0, 1.0f), glm::vec4(), glm::vec2(0, 1.0f/3.0f) },
			vertex3df_t { glm::vec4(size, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.25f, 1.0f/3.0f) },

			vertex3df_t { glm::vec4(0, 0, size, 1.0f), glm::vec4(), glm::vec2(0.5f, 2.0f/3.0f) },
			vertex3df_t { glm::vec4(size, 0, size, 1.0f), glm::vec4(), glm::vec2(0.5f, 1.0f/3.0f) },
			vertex3df_t { glm::vec4(0, size, size, 1.0f), glm::vec4(), glm::vec2(0.75f, 2.0f/3.0f) },
			vertex3df_t { glm::vec4(size, size, size, 1.0f), glm::vec4(), glm::vec2(0.75f, 1.0f/3.0f) },

			vertex3df_t { glm::vec4(0, size, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 2.0f/3.0f) },
			vertex3df_t { glm::vec4(size, size, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f/3.0f) },

			vertex3df_t { glm::vec4(0, size, 0, 1.0f), glm::vec4(), glm::vec2(0.25f, 1.0f) },
			vertex3df_t { glm::vec4(0, size, size, 1.0f), glm::vec4(), glm::vec2(0.5f, 1.0f) },

			vertex3df_t { glm::vec4(size, size, 0, 1.0f), glm::vec4(), glm::vec2(0.25f, 0.0f) },
			vertex3df_t { glm::vec4(size, size, size, 1.0f), glm::vec4(), glm::vec2(0.5f, 0.0f) }
		};

		entity.polygons =
		{
			{ 0, { 0, 2, 1 } },
			{ 0, { 1, 2, 3 } },
			{ 0, { 4, 5, 6 } },
			{ 0, { 5, 7, 6 } },
			{ 0, { 6, 7, 8 } },
			{ 0, { 7, 9, 8 } },
			{ 0, { 1, 3, 4 } },
			{ 0, { 3, 5, 4 } },
			{ 0, { 1, 11, 10 } },
			{ 0, { 1, 4, 11 } },
			{ 0, { 3, 12, 5 } },
			{ 0, { 5, 12, 13 } }
		};
#endif
	}

	entity.material_index = 0;
	mesh->get_material_library().emplace_back(*material);

	return mesh;
}


mesh_base_c* mesh_factory_c::generate_plane(float width, float height,
	bool center_mesh,
	material_t* material)
{
	mesh_base_c* mesh = new mesh_base_c();

	mesh->m_is_valid = true;

	mesh->m_name = "plane";

	mesh->m_scene_inverse_transform
		= glm::identity<decltype(mesh->m_scene_inverse_transform)>();

	mesh->m_entities.emplace_back();
	auto& entity = mesh->m_entities.back();

	if (center_mesh)
	{
		float half_width = width / 2.0f;
		float half_height = height / 2.0f;

		entity.vertices =
		{
			vertex3df_t { glm::vec4(-half_width, 0, half_height, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(half_width, 0, half_height, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(-half_width, 0, -half_height, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(half_width, 0, -half_height, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) }
		};
	}
	else
	{
		entity.vertices =
		{
			vertex3df_t { glm::vec4(0, 0, height, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, 0, height, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(width, 0, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) }
		};
	}

	entity.polygons =
	{
		{ 0, { 0, 1, 2 } },
		{ 0, { 1, 3, 2 } }
	};

	entity.material_index = 0;
	mesh->get_material_library().emplace_back(*material);

	return mesh;
}


mesh_base_c* mesh_factory_c::generate_exit(float width, float height)
{
	width *= 0.95f;

	mesh_base_c* mesh = new mesh_base_c();

	mesh->m_is_valid = true;

	mesh->m_name = "exit";

	mesh->m_scene_inverse_transform
		= glm::identity<decltype(mesh->m_scene_inverse_transform)>();

	auto generate_top_entity = [](mesh_base_c* mesh, float width, float height)
	{
		mesh->m_entities.emplace_back();
		auto& entity = mesh->m_entities.back();

		float y_shift = height * 0.8f;
		height *= 0.2f;

		entity.vertices =
		{
			// front
			vertex3df_t { glm::vec4(0, height, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(width, height, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, 0, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },

			// back
			vertex3df_t { glm::vec4(0, 0, width, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },
			vertex3df_t { glm::vec4(width, 0, width, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(0, height, width, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, height, width, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },

			// up
			vertex3df_t { glm::vec4(0, height, width, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, height, width, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, height, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(width, height, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },

			// bottom
			vertex3df_t { glm::vec4(0, 0, width, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(width, 0, width, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },
			vertex3df_t { glm::vec4(0, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, 0, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },

			// left
			vertex3df_t { glm::vec4(0, height, width, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, 0, width, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(0, height, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, 0, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },

			// right
			vertex3df_t { glm::vec4(width, height, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(width, height, width, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, 0, width, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },
		};

		for (auto& vert : entity.vertices)
		{
			vert.pos.y += y_shift;
		}

		entity.polygons =
		{
			{ 0, { 0, 2, 1 } },
			{ 0, { 1, 2, 3 } },

			{ 0, { 4, 5, 6 } },
			{ 0, { 5, 7, 6 } },

			/*{ 0, { 8, 9, 10 } },
			{ 0, { 9, 11, 10 } },*/	// top

			//{ 0, { 12, 14, 13 } },
			//{ 0, { 13, 14, 15 } },

			{ 0, { 16, 18, 17 } },
			{ 0, { 17, 18, 19 } },

			{ 0, { 20, 22, 21 } },
			{ 0, { 21, 22, 23 } }
		};

		sr::material_t material;
		{
			strcpy_s(material.name, "exit_top_disabled");
			material.shade_model = sr::e_sm_noshade;
			material.is_wireframe = false;
			material.texture_mode = sr::e_tm_perspective_correct;
			material.k_ambient =
				material.k_diffuse =
				material.k_specular = 1.0f;
			material.k_ambient_color = { 255, 255, 255, 0 };
			material.k_diffuse_color = { 255, 255, 255, 0 };
			material.k_specular_color.color = XRGB(0, 0, 0);

			material.texture2d = resource_manager_c::me()->get_texture
			(
				resource_manager_c::e_texture_exit_top
			);

			material.attr = material.shade_model
				| material.texture_mode
				| POLY_ATTR_ALPHA_CHANNEL
				| POLY_ATTR_2SIDED
				| POLY_ATTR_ZBUFFER;

			sr::set_material_alpha_level(&material, 4);
		}

		entity.material_index = 0;
		mesh->get_material_library().emplace_back(material);
	};

	auto generate_bottom_entity = [](mesh_base_c* mesh, float width, float height)
	{
		mesh->m_entities.emplace_back();
		auto& entity = mesh->m_entities.back();

		height *= 0.8f;

		entity.vertices =
		{
			// front
			vertex3df_t { glm::vec4(0, height, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(width, height, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, 0, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },

			// back
			vertex3df_t { glm::vec4(0, 0, width, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },
			vertex3df_t { glm::vec4(width, 0, width, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(0, height, width, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, height, width, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },

			// up
			vertex3df_t { glm::vec4(0, height, width, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, height, width, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, height, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(width, height, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },

			// bottom
			vertex3df_t { glm::vec4(0, 0, width, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(width, 0, width, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },
			vertex3df_t { glm::vec4(0, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, 0, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },

			// left
			vertex3df_t { glm::vec4(0, height, width, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, 0, width, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(0, height, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(0, 0, 0, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },

			// right
			vertex3df_t { glm::vec4(width, height, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, 0, 0, 1.0f), glm::vec4(), glm::vec2(0.0f, 1.0f) },
			vertex3df_t { glm::vec4(width, height, width, 1.0f), glm::vec4(), glm::vec2(1.0f, 0.0f) },
			vertex3df_t { glm::vec4(width, 0, width, 1.0f), glm::vec4(), glm::vec2(1.0f, 1.0f) },
		};

		entity.polygons =
		{
			{ 0, { 0, 2, 1 } },
			{ 0, { 1, 2, 3 } },

			{ 0, { 4, 5, 6 } },
			{ 0, { 5, 7, 6 } },

			//{ 0, { 8, 9, 10 } },
			//{ 0, { 9, 11, 10 } },

			//{ 0, { 12, 14, 13 } },
			//{ 0, { 13, 14, 15 } },

			{ 0, { 16, 18, 17 } },
			{ 0, { 17, 18, 19 } },

			{ 0, { 20, 22, 21 } },
			{ 0, { 21, 22, 23 } }
		};

		sr::material_t material;
		{
			material.shade_model = sr::e_sm_noshade;
			material.is_wireframe = false;
			material.texture_mode = sr::e_tm_perspective_correct;
			material.k_ambient =
				material.k_diffuse =
				material.k_specular = 1.0f;
			material.k_ambient_color = { 255, 255, 255, 0 };
			material.k_diffuse_color = { 255, 255, 255, 0 };
			material.k_specular_color.color = XRGB(0, 0, 0);

			material.attr = material.shade_model
				| material.texture_mode
				| POLY_ATTR_ALPHA_CHANNEL
				| POLY_ATTR_2SIDED
				| POLY_ATTR_ZBUFFER;

			sr::set_material_alpha_level(&material, 2);
		}

		entity.material_index = 1;
		mesh->get_material_library().emplace_back(material);
	};

	generate_top_entity(mesh, width, height);
	generate_bottom_entity(mesh, width, height);

	return mesh;
}
