#pragma once

#include <engine3d/sr_math.h>
#include <engine3d/sr_types.h>

// stl
#include <map>
#include <list>
#include <vector>
#include <chrono>
#include <memory>

// assimp
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#ifdef WIN32
#	pragma comment(lib, "assimp.lib")
#endif

#define USE_INTERPOLATION_BETWEEN_ANIM_FRAMES

class mesh_base_c
{
	friend class object3d_c;
	friend class mesh_factory_c;
	friend class explosion3d_c;

	// constants
	const int m_default_texture_mode = POLY_ATTR_PERSPECTIVE_CORRECT_TEXTURE;

public:
	typedef struct
	{
		int attr;
		uint32_t indices[3];
	} mesh_face_t;

	typedef std::vector<sr::vertex3df_t> mesh_vertex_list_t;
	typedef std::vector<mesh_face_t> mesh_polygon_list_t;	// todo: use linked list ?

	typedef struct
	{
		mesh_vertex_list_t vertices;
		mesh_polygon_list_t polygons;

		std::vector<uint64_t> vertices_common_edges;

		int material_index;

		bool is_rigged;
	} entity_t;

	typedef std::vector<sr::material_t> material_library_t;

	typedef std::unique_ptr<sr::texture::texture2d_c> texture_ptr_t;
	typedef std::vector<texture_ptr_t> texture_library_t;

protected:
	typedef std::string bone_name_t;

	typedef struct
	{
		glm::mat4x4 bone_offset;
		glm::mat4x4 final_transformation;
	} bone_info_t;

	typedef struct vertex_bone_data_s
	{
		uint32_t id[4];
		float weight[4];

		vertex_bone_data_s()
		{
			memset(id, 0, sizeof(id));
			memset(weight, 0, sizeof(weight));
		}

		void add_bone_data(uint32_t bone_id, float bone_weight)
		{
			for (uint32_t i = 0; i < sizeof(id) / sizeof(id[0]); i++)
			{
				if (weight[i] == 0.0)
				{
					id[i] = bone_id;
					weight[i] = bone_weight;
					return;
				}
			}
		}
	} vertex_bone_data_t;

	typedef struct mesh_node_s
	{
		std::string name;
		glm::mat4x4 transformation;

		mesh_node_s* parent_ptr = nullptr;
		std::list<mesh_node_s> childs;
	} mesh_node_t;

	typedef struct
	{
		float time;

		glm::vec3 value_vec;
		glm::mat4x4 value_mat;
	} vector_key_t;

	typedef struct
	{
		float time;

		glm::quat value_quat;
		glm::mat4x4 value_mat;
	} quat_key_t;

	typedef struct
	{
		std::string node_name;
		std::vector<vector_key_t> scaling_keys;
		std::vector<vector_key_t> position_keys;
		std::vector<quat_key_t> rotation_keys;
	} mesh_node_channel_t;

	typedef std::map<std::string, mesh_node_channel_t> node_anim_channel_t;	// <node name> - <channel>

	typedef struct
	{
		double duration, ticks_per_second;
		std::string animation_name;

		uint32_t frames_count = 0;

		node_anim_channel_t node_anim_channel;
	} animation_entity_t;

#if 0
	typedef struct
	{
		glm::vec3 scaling;
		glm::vec3 position;
		glm::quat rotation;

		glm::mat4x4 bone_offset;

		glm::mat4x4 parent_transform;
	} transform_set_t;
#endif

public:
	typedef std::vector<mesh_vertex_list_t> mesh_static_anim_frame_t; // vector of meshes

	typedef std::chrono::time_point<std::chrono::system_clock> timepoint_t;

public:
	mesh_base_c();
	virtual ~mesh_base_c();

	virtual bool load(const std::string& path,
		uint32_t assimp_load_flags);

	virtual void unload();

	virtual void disable_alpha_channel_in_textures();

	virtual void create_static_frames(uint32_t animation_index,
		uint32_t frames_count);

	virtual size_t calculate_polygon_count_total() const;

	material_library_t& get_material_library();
	texture_library_t& get_texture_library();

protected:
	// recursive copy resources data from assimp structs to own structs
	virtual bool load_resources();

	// recursive copy bones data from assimp structs to own structs
	virtual void load_bones();

	// compute transform matrices to all bones according to animation time
	virtual void bone_transform(uint32_t animation_index,
		float time_in_seconds,
		std::vector<glm::mat4x4>& transforms);

	// compute transform matrices to all bones according to animation time for blend 2 different animations
	virtual void mesh_base_c::bone_blend_transform(uint32_t animation_index1,
		uint32_t animation_index2,
		float animation_time_in_seconds1,
		float animation_time_in_seconds2,
		float blend_factor,	// [0.0f ... 1.0f]
		std::vector<glm::mat4x4>& transforms);

	// recursive copy node hierarchy from assimp structs to own structs
	virtual void copy_node_hierarchy();
	virtual void recursive_node_hierarchy(aiNode* node,
		mesh_node_t& parent_mesh_node);

	// recursive copy animation data from assimp structs to own structs
	virtual void copy_animation_data();

	virtual void compute_animation_frame(uint32_t animation_index,
		float animation_time,
		const mesh_node_t& node,
		const glm::mat4x4& parent_transform);

	virtual void compute_animation_blend(uint32_t animation_index1,
		uint32_t animation_index2,
		float animation_time1,
		float animation_time2,
		float blend_factor,
		const mesh_node_t& node,
		const glm::mat4x4& parent_transform);

	// find transformation index of frame by animation time
	uint32_t find_rotation(float animation_time,
		const mesh_node_channel_t& channel);
	uint32_t find_position(float animation_time,
		const mesh_node_channel_t& channel);
	uint32_t find_scaling(float animation_time,
		const mesh_node_channel_t& channel);

	// animation frames interpolated transformation
	void calc_interpolated_rotation(glm::quat& out,
		float animation_time,
		const mesh_node_channel_t& channel);
	void calc_interpolated_position(glm::vec3& out,
		float animation_time,
		const mesh_node_channel_t& channel);
	void calc_interpolated_scaling(glm::vec3& out,
		float animation_time,
		const mesh_node_channel_t& channel);

	void calc_interpolated_vector(glm::vec3& out,
		float animation_time,
		const vector_key_t& key1,
		const vector_key_t& key2);

protected:
	// own name
	std::string m_name;

	// full path to mesh file
	std::string m_path;

	bool m_is_valid = false;

	// flags corresponds to assimp flags
	uint32_t m_mesh_load_flags = 0;

	// temporary scene object
	const aiScene* m_scene = nullptr;

	// root scene inverse matrix
	glm::mat4x4 m_scene_inverse_transform;

	// root node
	mesh_node_s m_root_node;

	// bones data & precalculated data
	uint32_t m_bones_num = 0;
	std::map<bone_name_t, uint32_t> m_bone_mapping;	// bone name - bone index
	std::vector<bone_info_t> m_bone_info;			// bone offset + final transform data
	std::vector<std::vector<vertex_bone_data_t>> m_meshes_vertex_bones_map;

	// collection of animation data
	std::vector<animation_entity_t> m_animations;

	// precalculated static frames for animations
	std::vector<mesh_static_anim_frame_t> m_precalculated_static_frames;

	// resources
	material_library_t m_materials;
	texture_library_t m_textures;
	bool m_disable_alpha_channel_in_textures = false;

	// collection of scene meshes
	std::vector<entity_t> m_entities;
};
