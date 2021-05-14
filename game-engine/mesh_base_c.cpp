#include "stdafx.h"
#include "mesh_base_c.h"

#include <engine3d/sr_common.h>

void convert_aimatrix_to_glmmat4x4(aiMatrix4x4 mat_src, glm::mat4x4& mat_dest)
{
	mat_src.Transpose();

	mat_dest[0][0] = mat_src.a1;
	mat_dest[0][1] = mat_src.a2;
	mat_dest[0][2] = mat_src.a3;
	mat_dest[0][3] = mat_src.a4;

	mat_dest[1][0] = mat_src.b1;
	mat_dest[1][1] = mat_src.b2;
	mat_dest[1][2] = mat_src.b3;
	mat_dest[1][3] = mat_src.b4;

	mat_dest[2][0] = mat_src.c1;
	mat_dest[2][1] = mat_src.c2;
	mat_dest[2][2] = mat_src.c3;
	mat_dest[2][3] = mat_src.c4;

	mat_dest[3][0] = mat_src.d1;
	mat_dest[3][1] = mat_src.d2;
	mat_dest[3][2] = mat_src.d3;
	mat_dest[3][3] = mat_src.d4;
}


void convert_aivector3d_to_glmvec3(const aiVector3D& vec_src, glm::vec3& vec_dest)
{
	vec_dest.x = vec_src.x;
	vec_dest.y = vec_src.y;
	vec_dest.z = vec_src.z;
}


void convert_aiquaternion_to_glmquat(const aiQuaternion& quat_src, glm::quat& quat_dest)
{
	quat_dest.w = quat_src.w;
	quat_dest.x = quat_src.x;
	quat_dest.y = quat_src.y;
	quat_dest.z = quat_src.z;
}

//

mesh_base_c::mesh_base_c()
{
}


mesh_base_c::~mesh_base_c()
{
}


bool mesh_base_c::load(const std::string& path, uint32_t assimp_load_flags)
{
	if (m_is_valid)
	{
		unload();
	}

	m_scene = aiImportFile(path.c_str(), assimp_load_flags);

	if (m_scene == nullptr)
	{
		return false;
	}

	m_path = path;

	m_bones_num = 0;
	m_mesh_load_flags = assimp_load_flags;

	convert_aimatrix_to_glmmat4x4(m_scene->mRootNode->mTransformation,
		m_scene_inverse_transform);
	glm::inverse(m_scene_inverse_transform);

	m_entities.resize(m_scene->mNumMeshes);
	for (size_t i = 0; i < m_scene->mNumMeshes; ++i)
	{
		auto& cur_mesh = m_scene->mMeshes[i];
		auto& cur_entity = m_entities[i];

		cur_entity.is_rigged = cur_mesh->HasBones();

		cur_entity.material_index = cur_mesh->mMaterialIndex;

		cur_entity.vertices.resize(cur_mesh->mNumVertices);

		for (size_t v = 0; v < cur_mesh->mNumVertices; ++v)
		{
			auto& cur_vert = cur_entity.vertices[v];

			cur_vert.pos = glm::vec4
			(
				cur_mesh->mVertices[v].x,
				cur_mesh->mVertices[v].y,
				cur_mesh->mVertices[v].z,
				1.0f
			);

			cur_vert.normal = glm::vec4
			(
				cur_mesh->mNormals[v].x,
				cur_mesh->mNormals[v].y,
				cur_mesh->mNormals[v].z,
				1.0f
			);

			if (cur_mesh->mTextureCoords[0])
			{
				const aiVector3D* tex_coord = cur_mesh->mTextureCoords[0];

				float u_coord = tex_coord[v].x;
				float v_coord = tex_coord[v].y;

				if (u_coord > 1.0f)
					u_coord = 1.0f;
				if (v_coord > 1.0f)
					v_coord = 1.0f;
				if (u_coord < 0.0f)
					u_coord = 0.0f;
				if (v_coord < 0.0f)
					v_coord = 0.0f;

				cur_vert.uv = glm::vec2(u_coord, v_coord);
			}

			if (cur_mesh->mColors[0])
			{
				const aiColor4D* vert_color = cur_mesh->mColors[0];

				cur_vert.color = XRGB
				(
					(uint8_t)(vert_color->r * 255),
					(uint8_t)(vert_color->g * 255),
					(uint8_t)(vert_color->b * 255)
				);
			}
		}

		cur_entity.polygons.resize(cur_mesh->mNumFaces);
		for (size_t f = 0; f < cur_mesh->mNumFaces; ++f)
		{
			auto& cur_face = cur_mesh->mFaces[f];
			auto& cur_polygon = cur_entity.polygons[f];

			cur_polygon.attr = POLY_ATTR_NONE;

			if (cur_face.mNumIndices != 3)
			{
				exit(-5);
			}

			cur_polygon.indices[0] = cur_face.mIndices[0];
			cur_polygon.indices[1] = cur_face.mIndices[1];
			cur_polygon.indices[2] = cur_face.mIndices[2];
		}
	}

	load_resources();

	copy_node_hierarchy();

	copy_animation_data();

	load_bones();

	aiReleaseImport(m_scene);
	m_scene = nullptr;

	m_is_valid = true;

	return true;
}


void mesh_base_c::unload()
{
	m_is_valid = false;

	m_entities.clear();
	m_entities.shrink_to_fit();

	m_bone_mapping.clear();

	m_bone_info.clear();
	m_bone_info.shrink_to_fit();

	m_meshes_vertex_bones_map.clear();
	m_meshes_vertex_bones_map.shrink_to_fit();

	m_animations.clear();
	m_animations.shrink_to_fit();

	m_precalculated_static_frames.clear();
	m_precalculated_static_frames.shrink_to_fit();

	m_materials.clear();
	m_materials.shrink_to_fit();

	m_textures.clear();
	m_textures.shrink_to_fit();
}


void mesh_base_c::disable_alpha_channel_in_textures()
{
	m_disable_alpha_channel_in_textures = true;
}


bool mesh_base_c::load_resources()
{
	if (m_scene->HasTextures())
	{
		m_textures.resize(m_scene->mNumTextures);
		for (size_t i = 0; i < m_scene->mNumTextures; ++i)
		{
			auto& texture = m_scene->mTextures[i];
			auto& this_texture = m_textures[i];
			this_texture.reset(new sr::texture::texture2d_c);

			std::string full_tex_path
				= get_directory_path(m_path)
				+ "/"
				+ texture->mFilename.C_Str();

			if (!this_texture->load(full_tex_path.c_str()))
			{
				// todo ?
			}
		}
	}

	if (m_scene->HasMaterials())
	{
		m_materials.resize(m_scene->mNumMaterials);

		for (size_t i = 0; i < m_scene->mNumMaterials; ++i)
		{
			auto& material_src = m_scene->mMaterials[i];
			auto& material_dst = m_materials[i];

			material_dst.attr = 0;
			material_dst.texture2d = nullptr;
			material_dst.texture_mode = sr::e_texture_mode::e_tm_notextured;
			material_dst.shade_model = sr::e_shade_model::e_sm_noshade;
			material_dst.color.color = 0;
			material_dst.k_ambient = material_dst.k_diffuse = material_dst.k_specular = 0.0f;

			aiString material_name;
			if (material_src->Get(AI_MATKEY_NAME, material_name) == aiReturn_SUCCESS)
			{
				strcpy_s(material_dst.name, material_name.C_Str());
			}

			aiColor3D ambient_color;
			if (material_src->Get(AI_MATKEY_COLOR_AMBIENT, ambient_color) == aiReturn_SUCCESS)
			{
				material_dst.k_ambient = 1.0f;

				material_dst.k_ambient_color.a = 0;
				material_dst.k_ambient_color.r = (uint8_t)(ambient_color.r * 255);
				material_dst.k_ambient_color.g = (uint8_t)(ambient_color.g * 255);
				material_dst.k_ambient_color.b = (uint8_t)(ambient_color.b * 255);
			}

			aiColor3D diffuse_color;
			if (material_src->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color) == aiReturn_SUCCESS)
			{
				material_dst.k_diffuse = 1.0f;

				material_dst.k_diffuse_color.a = 0;
				material_dst.k_diffuse_color.r = (uint8_t)(diffuse_color.r * 255);
				material_dst.k_diffuse_color.g = (uint8_t)(diffuse_color.g * 255);
				material_dst.k_diffuse_color.b = (uint8_t)(diffuse_color.b * 255);
			}

			aiColor3D specular_color;
			if (material_src->Get(AI_MATKEY_COLOR_SPECULAR, specular_color) == aiReturn_SUCCESS)
			{
				material_dst.k_diffuse = 1.0f;

				material_dst.k_specular_color.a = 0;
				material_dst.k_specular_color.r = (uint8_t)(specular_color.r * 255);
				material_dst.k_specular_color.g = (uint8_t)(specular_color.g * 255);
				material_dst.k_specular_color.b = (uint8_t)(specular_color.b * 255);
			}

			aiShadingMode shading_model;
			if (material_src->Get(AI_MATKEY_SHADING_MODEL, shading_model) == aiReturn_SUCCESS)
			{
				if (shading_model == aiShadingMode_Flat)
				{
					material_dst.attr |= POLY_ATTR_SHADE_FLAT;
					material_dst.shade_model = sr::e_shade_model::e_sm_shade_flat;
				}
				else // if (shading_model != aiShadingMode_Gouraud)
				{
					material_dst.attr |= POLY_ATTR_SHADE_GOURAUD | POLY_ATTR_HAVE_VERTEX_NORMAL;
					material_dst.shade_model = sr::e_shade_model::e_sm_shade_gouraund;
				}
			}
			else
			{
				material_dst.attr |= POLY_ATTR_SHADE_FLAT;
				material_dst.shade_model = sr::e_shade_model::e_sm_shade_flat;
			}

			auto diff_tex_cnt = material_src->GetTextureCount(aiTextureType_DIFFUSE);
			for (uint32_t i = 0; i < diff_tex_cnt; ++i)
			{
				aiString diffuse_texture_path;
				if (material_src->Get(AI_MATKEY_TEXTURE_DIFFUSE(i), diffuse_texture_path) == aiReturn_SUCCESS)
				{
					std::string full_tex_path
						= get_directory_path(m_path)
							+ "/"
							+ diffuse_texture_path.C_Str();

					sr::texture::texture2d_c* tex = new sr::texture::texture2d_c;
					m_textures.emplace_back(tex);

					if (tex->load(full_tex_path.c_str()))
					{
						if (m_disable_alpha_channel_in_textures)
							tex->set_alpha_channel_value(0);

						material_dst.texture2d = tex;
						material_dst.attr |= POLY_ATTR_TEXTURED | m_default_texture_mode;
						material_dst.texture_mode = sr::e_texture_mode::e_tm_perspective_correct;

						material_dst.attr ^= POLY_ATTR_SHADE_GOURAUD | POLY_ATTR_HAVE_VERTEX_NORMAL;
						material_dst.attr |= POLY_ATTR_SHADE_FLAT;
						material_dst.shade_model = sr::e_shade_model::e_sm_shade_flat;
					}
				}
			}

#if 1	// unsupported
			auto spec_tex_cnt = material_src->GetTextureCount(aiTextureType_SPECULAR);
			auto amb_tex_cnt = material_src->GetTextureCount(aiTextureType_AMBIENT);
			auto unkn_tex_cnt = material_src->GetTextureCount(aiTextureType_UNKNOWN);
#endif
		}
	}

	return true;
}


void mesh_base_c::create_static_frames(uint32_t animation_index,
	uint32_t frames_count)
{
	if (animation_index >= m_animations.size())
		return;

	auto& animation = m_animations[animation_index];
	animation.duration;
	animation.ticks_per_second;

	auto& precalculated_static_frames = m_precalculated_static_frames[animation_index];
	precalculated_static_frames.resize(m_entities.size());

	for (uint32_t frame_i = 0; frame_i < animation.frames_count; frame_i++)
	{
		auto& ch = animation.node_anim_channel.begin()->second;
		ch.position_keys[frame_i].time;

		float time_in_seconds = 0.0f;

		std::vector<glm::mat4x4> transforms;
		bone_transform(animation_index,
			time_in_seconds,
			transforms);

		int mesh_index = 0;
		for (const auto& entity : m_entities)
		{
			auto& mesh_vertex_list = precalculated_static_frames[mesh_index];
			mesh_vertex_list.resize(animation.frames_count);
			mesh_vertex_list = entity.vertices;

			auto poly_count = entity.polygons.size();
			for (uint32_t face_i = 0; face_i < poly_count; ++face_i)
			{
				const auto& face = entity.polygons[face_i];

				const auto& bones = m_meshes_vertex_bones_map[mesh_index];

				for (uint32_t i = 0; i < 3; ++i)
				{
					auto index = face.indices[i];

					auto bone0_index = bones[index].id[0];
					auto bone1_index = bones[index].id[1];
					auto bone2_index = bones[index].id[2];
					auto bone3_index = bones[index].id[3];

					auto bone0_weight = bones[index].weight[0];
					auto bone1_weight = bones[index].weight[1];
					auto bone2_weight = bones[index].weight[2];
					auto bone3_weight = bones[index].weight[3];

					glm::mat4x4 bone0_transform = transforms[bone0_index] * bone0_weight;
					glm::mat4x4 bone1_transform = transforms[bone1_index] * bone1_weight;
					glm::mat4x4 bone2_transform = transforms[bone2_index] * bone2_weight;
					glm::mat4x4 bone3_transform = transforms[bone3_index] * bone3_weight;
					glm::mat4x4 final_transform =
						bone0_transform
						+ bone1_transform
						+ bone2_transform
						+ bone3_transform;

					mesh_vertex_list[index].pos = final_transform * mesh_vertex_list[index].pos;
				}
			}

			mesh_index++;
		}
	}
}


size_t mesh_base_c::calculate_polygon_count_total() const
{
	size_t polygon_count_total = 0;

	for (auto& entity : m_entities)
	{
		polygon_count_total += entity.polygons.size();
	}

	return polygon_count_total;
}


mesh_base_c::material_library_t& mesh_base_c::get_material_library()
{
	return m_materials;
}


mesh_base_c::texture_library_t& mesh_base_c::get_texture_library()
{
	return m_textures;
}


void mesh_base_c::load_bones()
{
	for (uint32_t i = 0; i < m_scene->mNumMeshes; ++i)
	{
		auto& mesh = m_scene->mMeshes[i];

		m_meshes_vertex_bones_map.push_back({});
		auto& vertex_bone_map = m_meshes_vertex_bones_map.back();
		vertex_bone_map.resize(mesh->mNumVertices);

		for (uint32_t bi = 0; bi < mesh->mNumBones; bi++)
		{
			auto& cur_bone = mesh->mBones[bi];

			uint32_t bone_index = 0;
			std::string bone_name(cur_bone->mName.data);

			if (m_bone_mapping.find(bone_name) == m_bone_mapping.end())
			{
				bone_index = m_bones_num;
				m_bones_num++;
				m_bone_info.push_back({});
			}
			else
			{
				bone_index = m_bone_mapping[bone_name];
			}

			m_bone_mapping[bone_name] = bone_index;
			convert_aimatrix_to_glmmat4x4(cur_bone->mOffsetMatrix,
				m_bone_info[bone_index].bone_offset);

			for (uint32_t j = 0; j < cur_bone->mNumWeights; j++)
			{
				uint32_t vertex_id = cur_bone->mWeights[j].mVertexId;
				float weight = cur_bone->mWeights[j].mWeight;
				vertex_bone_map[vertex_id].add_bone_data(bone_index, weight);
			}
		}
	}
}


void mesh_base_c::bone_transform(uint32_t animation_index,
	float time_in_seconds,
	std::vector<glm::mat4x4>& transforms)
{
	glm::mat4x4 identity_mat = glm::identity<glm::mat4x4>();

	const auto& animation = m_animations[animation_index];

	float ticks_per_second = animation.ticks_per_second != 0 ?
		(float)animation.ticks_per_second :
		25.0f;

	float time_in_ticks = time_in_seconds * ticks_per_second;
	float animation_time = (float)fmod(time_in_ticks, animation.duration);

	compute_animation_frame
	(
		animation_index,
		animation_time,
		m_root_node,
		identity_mat
	);

	transforms.resize(m_bones_num);

	for (uint32_t i = 0; i < m_bones_num; i++)
	{
		transforms[i] = m_bone_info[i].final_transformation;
	}
}


void mesh_base_c::bone_blend_transform(uint32_t animation_index1,
	uint32_t animation_index2,
	float animation_time_in_seconds1,
	float animation_time_in_seconds2,
	float blend_factor,
	std::vector<glm::mat4x4>& transforms)
{
	glm::mat4x4 identity_mat = glm::identity<glm::mat4x4>();

	const auto& animation1 = m_animations[animation_index1];
	const auto& animation2 = m_animations[animation_index2];

	float ticks_per_second1 = animation1.ticks_per_second != 0 ?
		(float)animation1.ticks_per_second :
		25.0f;

	float ticks_per_second2 = animation2.ticks_per_second != 0 ?
		(float)animation2.ticks_per_second :
		25.0f;

	float time_in_ticks1 = animation_time_in_seconds1 * ticks_per_second1;
	float animation_time1 = (float)fmod(time_in_ticks1, animation1.duration);

	float time_in_ticks2 = animation_time_in_seconds2 * ticks_per_second2;
	float animation_time2 = (float)fmod(time_in_ticks2, animation2.duration);

	compute_animation_blend
	(
		animation_index1, animation_index2,
		animation_time1, animation_time2,
		blend_factor,
		m_root_node,
		identity_mat
	);

	transforms.resize(m_bones_num);

	for (uint32_t i = 0; i < m_bones_num; i++)
	{
		transforms[i] = m_bone_info[i].final_transformation;
	}
}


void mesh_base_c::recursive_node_hierarchy(aiNode* node,
	mesh_node_t& parent_mesh_node)
{
	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		const auto& ai_child = node->mChildren[i];

		parent_mesh_node.childs.push_back({});
		auto& new_node = parent_mesh_node.childs.back();
		{
			new_node.parent_ptr = &parent_mesh_node;
			new_node.name = ai_child->mName.C_Str();

			convert_aimatrix_to_glmmat4x4(ai_child->mTransformation,
				new_node.transformation);
		}

		recursive_node_hierarchy(ai_child, new_node);
	}
}


void mesh_base_c::copy_node_hierarchy()
{
	m_root_node.name = m_scene->mRootNode->mName.C_Str();
	convert_aimatrix_to_glmmat4x4(m_scene->mRootNode->mTransformation,
		m_root_node.transformation);
	recursive_node_hierarchy(m_scene->mRootNode, m_root_node);
}


void mesh_base_c::copy_animation_data()
{
	for (uint32_t anim_index = 0; anim_index < m_scene->mNumAnimations; anim_index++)
	{
		auto& ai_anim = m_scene->mAnimations[anim_index];

		m_animations.push_back({});
		m_precalculated_static_frames.push_back({});

		animation_entity_t& animation = m_animations.back();
		node_anim_channel_t& mesh_anim_channel_map = animation.node_anim_channel;

		animation.animation_name = ai_anim->mName.C_Str();
		animation.duration = ai_anim->mDuration;
		animation.ticks_per_second = ai_anim->mTicksPerSecond;

		for (uint32_t i = 0; i < ai_anim->mNumChannels; i++)
		{
			auto& channel = ai_anim->mChannels[i];
			auto node_name = std::string(channel->mNodeName.C_Str());

			animation.frames_count = sr::find_max
			(
				animation.frames_count,
				channel->mNumPositionKeys,
				channel->mNumRotationKeys,
				channel->mNumScalingKeys
			);

			auto& dest_channel = mesh_anim_channel_map[node_name];
			dest_channel.node_name = node_name;

			for (uint32_t si = 0; si < channel->mNumScalingKeys; si++)
			{
				const auto& ai_scaling_key = channel->mScalingKeys[si];

				vector_key_t new_scaling_key;
				new_scaling_key.time = (float)ai_scaling_key.mTime;

				convert_aivector3d_to_glmvec3(ai_scaling_key.mValue,
					new_scaling_key.value_vec);

				glm::vec3 scaling_vec;
				convert_aivector3d_to_glmvec3(ai_scaling_key.mValue,
					scaling_vec);
				new_scaling_key.value_mat = glm::scale(scaling_vec);

				dest_channel.scaling_keys.emplace_back(new_scaling_key);
			}

			for (uint32_t pi = 0; pi < channel->mNumPositionKeys; pi++)
			{
				const auto& ai_position_key = channel->mPositionKeys[pi];

				vector_key_t new_position_key;
				new_position_key.time = (float)ai_position_key.mTime;

				convert_aivector3d_to_glmvec3(ai_position_key.mValue,
					new_position_key.value_vec);

				glm::vec3 position_vec;
				convert_aivector3d_to_glmvec3(ai_position_key.mValue,
					position_vec);

				new_position_key.value_mat = glm::translate(position_vec);

				dest_channel.position_keys.emplace_back(new_position_key);
			}

			for (uint32_t ri = 0; ri < channel->mNumRotationKeys; ri++)
			{
				const auto& ai_rotation_key = channel->mRotationKeys[ri];

				quat_key_t new_rotation_key;
				new_rotation_key.time = (float)ai_rotation_key.mTime;

				convert_aiquaternion_to_glmquat(ai_rotation_key.mValue,
					new_rotation_key.value_quat);

				glm::quat rotation_quat;
				convert_aiquaternion_to_glmquat(ai_rotation_key.mValue,
					rotation_quat);

				new_rotation_key.value_mat = glm::mat4x4(rotation_quat);

				dest_channel.rotation_keys.emplace_back(new_rotation_key);
			}
		}
	}
}


uint32_t mesh_base_c::find_rotation(float animation_time, const mesh_node_channel_t& channel)
{
	auto sz = channel.position_keys.size();

	for (uint32_t i = 0; i < sz - 1; i++)
	{
		if (animation_time < channel.rotation_keys[i + 1].time)
		{
			return i;
		}
	}

	return 0;
}


uint32_t mesh_base_c::find_position(float animation_time, const mesh_node_channel_t& channel)
{
	auto sz = channel.position_keys.size();

	for (uint32_t i = 0; i < sz - 1; i++)
	{
		if (animation_time < channel.position_keys[i + 1].time)
		{
			return i;
		}
	}

	return 0;
}


uint32_t mesh_base_c::find_scaling(float animation_time, const mesh_node_channel_t& channel)
{
	auto sz = channel.scaling_keys.size();

	for (uint32_t i = 0; i < sz - 1; i++)
	{
		if (animation_time < channel.scaling_keys[i + 1].time)
		{
			return i;
		}
	}

	return 0;
}


void mesh_base_c::calc_interpolated_rotation(glm::quat& out,
	float animation_time,
	const mesh_node_channel_t& channel)
{
	if (channel.rotation_keys.size() == 1)
	{
		out = channel.rotation_keys[0].value_quat;
		return;
	}

	uint32_t rotation_index = find_rotation(animation_time, channel);
	uint32_t next_rotation_index = (rotation_index + 1);

	float delta_time = channel.rotation_keys[next_rotation_index].time -
		channel.rotation_keys[rotation_index].time;
	float factor = (animation_time - (float)channel.rotation_keys[rotation_index].time)
		/ delta_time;

	const auto& start_rotation_q = channel.rotation_keys[rotation_index].value_quat;
	const auto& end_rotation_q = channel.rotation_keys[next_rotation_index].value_quat;
	out = glm::slerp(start_rotation_q, end_rotation_q, factor);
	//out = glm::normalize(out);
}


void mesh_base_c::calc_interpolated_position(glm::vec3& out,
	float animation_time,
	const mesh_node_channel_t& channel)
{
	if (channel.position_keys.size() == 1)
	{
		out = channel.position_keys[0].value_vec;
		return;
	}

	uint32_t index = find_position(animation_time, channel);

	calc_interpolated_vector(out, animation_time,
		channel.position_keys[index], channel.position_keys[index + 1]);
}


void mesh_base_c::calc_interpolated_scaling(glm::vec3& out,
	float animation_time,
	const mesh_node_channel_t& channel)
{
	if (channel.scaling_keys.size() == 1)
	{
		out = channel.scaling_keys[0].value_vec;
		return;
	}

	uint32_t index = find_scaling(animation_time, channel);

	calc_interpolated_vector(out, animation_time,
		channel.scaling_keys[index], channel.scaling_keys[index + 1]);
}


void mesh_base_c::calc_interpolated_vector(glm::vec3& out,
	float animation_time,
	const vector_key_t& key1,
	const vector_key_t& key2)
{
	float delta_time = key2.time - key1.time;
	float factor = (animation_time - (float)key1.time) / delta_time;

#ifdef USE_INTERPOLATION_BETWEEN_ANIM_FRAMES
	out = glm::lerp(key1.value_vec, key2.value_vec, factor);
#else
	out = glm::mix(key1.value_vec, key2.value_vec, factor);
#endif
}


void mesh_base_c::compute_animation_frame(uint32_t animation_index,
	float animation_time,
	const mesh_node_t& node,
	const glm::mat4x4& parent_transform)
{
	const auto& animation = m_animations[animation_index];

	auto node_transformation = node.transformation;

	const auto& anim_channel_it = animation.node_anim_channel.find(node.name);
	if (anim_channel_it != animation.node_anim_channel.cend())
	{
		const mesh_node_channel_t& mesh_node_channel = anim_channel_it->second;

#ifdef USE_INTERPOLATION_BETWEEN_ANIM_FRAMES
		glm::vec3 scaling_vec;
		calc_interpolated_scaling(scaling_vec, animation_time, mesh_node_channel);
		const glm::mat4x4 scaling_matrix = glm::scale(scaling_vec);

		glm::quat rotation_quat;
		calc_interpolated_rotation(rotation_quat, animation_time, mesh_node_channel);
		const auto rotation_matrix = glm::toMat4(rotation_quat);

		glm::vec3 translation_vec;
		calc_interpolated_position(translation_vec, animation_time, mesh_node_channel);
		const glm::mat4x4 translation_matrix = glm::translate(translation_vec);
#else
		auto scaling_index = find_scaling(animation_time, mesh_node_channel);
		const auto& scaling_matrix = mesh_node_channel.scaling_keys[scaling_index].value_mat;

		auto rotation_index = find_rotation(animation_time, mesh_node_channel);
		const auto& rotation_matrix = mesh_node_channel.rotation_keys[rotation_index].value_mat;

		auto position_index = find_position(animation_time, mesh_node_channel);
		const auto& translation_matrix = mesh_node_channel.position_keys[position_index].value_mat;
#endif

		node_transformation = translation_matrix * rotation_matrix * scaling_matrix;
	}

	glm::mat4x4 global_transformation = parent_transform * node_transformation;

	if (m_bone_mapping.find(node.name) != m_bone_mapping.end())
	{
		uint32_t bone_index = m_bone_mapping[node.name];

		m_bone_info[bone_index].final_transformation =
			m_scene_inverse_transform *
			global_transformation *
			m_bone_info[bone_index].bone_offset;
	}

	for (const auto& child : node.childs)
	{
		compute_animation_frame
		(
			animation_index,
			animation_time,
			child,
			global_transformation
		);
	}
}


void mesh_base_c::compute_animation_blend(uint32_t animation_index1,
	uint32_t animation_index2,
	float animation_time1,
	float animation_time2,
	float blend_factor,
	const mesh_node_t& node,
	const glm::mat4x4& parent_transform)
{
	const auto& animation_first = m_animations[animation_index1];
	const auto& animation_second = m_animations[animation_index2];

	auto node_transformation = node.transformation;

	const auto& anim_channel_it1 = animation_first.node_anim_channel.find(node.name);
	const auto& anim_channel_it2 = animation_second.node_anim_channel.find(node.name);

	if (anim_channel_it1 != animation_first.node_anim_channel.cend() &&
		anim_channel_it2 != animation_second.node_anim_channel.cend())
	{
		const mesh_node_channel_t& mesh_node_channel1 = anim_channel_it1->second;
		const mesh_node_channel_t& mesh_node_channel2 = anim_channel_it2->second;

#ifdef USE_INTERPOLATION_BETWEEN_ANIM_FRAMES
		glm::vec3 scaling_vec1;
		calc_interpolated_scaling(scaling_vec1, animation_time1, mesh_node_channel1);

		glm::quat rotation_quat1;
		calc_interpolated_rotation(rotation_quat1, animation_time1, mesh_node_channel1);

		glm::vec3 translation_vec1;
		calc_interpolated_position(translation_vec1, animation_time1, mesh_node_channel1);

		//
		glm::vec3 scaling_vec2;
		calc_interpolated_scaling(scaling_vec2, animation_time2, mesh_node_channel2);

		glm::quat rotation_quat2;
		calc_interpolated_rotation(rotation_quat2, animation_time2, mesh_node_channel2);

		glm::vec3 translation_vec2;
		calc_interpolated_position(translation_vec2, animation_time2, mesh_node_channel2);

		//
		glm::mat4x4 blended_translation_matrix =
			glm::translate(glm::lerp(translation_vec1, translation_vec2, blend_factor));

		glm::mat4x4 blended_rotation_matrix =
			glm::toMat4(glm::slerp(rotation_quat1, rotation_quat2, blend_factor));

		glm::mat4x4 blended_scale_matrix =
			glm::scale(glm::lerp(scaling_vec1, scaling_vec2, blend_factor));

#else
		auto scaling_index1 = find_scaling(animation_time1, mesh_node_channel1);
		const auto& scaling_matrix1 = mesh_node_channel1.scaling_keys[scaling_index1].value_mat;

		auto rotation_index1 = find_rotation(animation_time1, mesh_node_channel1);
		const auto& rotation_matrix1 = mesh_node_channel1.rotation_keys[rotation_index1].value_mat;

		auto position_index1 = find_position(animation_time1, mesh_node_channel1);
		const auto& translation_matrix1 = mesh_node_channel1.position_keys[position_index1].value_mat;

		//
		auto scaling_index2 = find_scaling(animation_time2, mesh_node_channel2);
		const auto& scaling_matrix2 = mesh_node_channel2.scaling_keys[scaling_index2].value_mat;

		auto rotation_index2 = find_rotation(animation_time2, mesh_node_channel2);
		const auto& rotation_matrix2 = mesh_node_channel2.rotation_keys[rotation_index2].value_mat;

		auto position_index2 = find_position(animation_time2, mesh_node_channel2);
		const auto& translation_matrix2 = mesh_node_channel2.position_keys[position_index2].value_mat;

		//
		glm::mat4x4 blended_translation_matrix =
			glm::mix(translation_matrix1, translation_matrix2, blend_factor);

		glm::mat4x4 blended_rotation_matrix =
			glm::mix(rotation_matrix1, rotation_matrix2, blend_factor);

		glm::mat4x4 blended_scale_matrix =
			glm::mix(scaling_matrix1, scaling_matrix2, blend_factor);
#endif

		node_transformation = blended_translation_matrix
			* blended_rotation_matrix
			* blended_scale_matrix;
	}

	glm::mat4x4 global_transformation = parent_transform * node_transformation;

	if (m_bone_mapping.find(node.name) != m_bone_mapping.end())
	{
		uint32_t bone_index = m_bone_mapping[node.name];

		m_bone_info[bone_index].final_transformation =
			m_scene_inverse_transform *
			global_transformation *
			m_bone_info[bone_index].bone_offset;
	}

	for (const auto& child : node.childs)
	{
		compute_animation_blend
		(
			animation_index1, animation_index2,
			animation_time1, animation_time2,
			blend_factor,
			child,
			global_transformation
		);
	}
}
