#pragma once

#include "input_manager/input_manager_c.h"

class config_c
	: public singleton_c<config_c>
{
public:
	void initialize_defaults()
	{
		clear();

		// common config
		{
			m_config_map["monitor_index"]			= "0";
			m_config_map["screen_width"]			= "1280";
			m_config_map["screen_height"]			= "720";
			m_config_map["window_fullscreen_mode"]	= "1";

			m_config_map["shade_model"]				= "standard";
			m_config_map["texture_mode"]			= "perspective-correct";
		}

		// input mapping
		initialize_mapping_deafults();
	}

	void initialize_mapping_deafults()
	{
		m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_A)] =
			m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_LEFT)] =
			e_input_actions::action_turn_left;
		m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_D)]
			= m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_RIGHT)]
			= e_input_actions::action_turn_right;
		m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_W)]
			= m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_UP)]
			= e_input_actions::action_turn_up;
		m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_S)]
			= m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_DOWN)]
			= e_input_actions::action_turn_down;
		m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_SPACE)]
			= e_input_actions::action_plane_bomb;

		m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_F1)]
			= e_input_actions::action_show_debug_info;
		//m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_ESCAPE)]
		//	= e_input_actions::action_quit_game;
		m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_PRINTSCREEN)]
			= e_input_actions::action_take_screenshot;
		m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_F12)]
			= e_input_actions::action_take_screenshot;

		complete_mapping();
	}

#if 0
	bool load(const std::string& config_path)
	{
		clear();

		m_config_path = config_path;

		TiXmlDocument doc(config_path.c_str());
		TiXmlHandle hDoc(&doc);

		if (!doc.LoadFile(TIXML_ENCODING_UTF8))
		{
			return false;
		}

		//
		TiXmlElement* root_elem = hDoc.FirstChildElement().Element();

		if (!load_common(root_elem))
		{
			return false;
		}

		if (!load_input_mapping(root_elem))
		{
			// ?
		}

		return true;
	}
#else
	bool load_common(const std::string& config_path)
	{
		m_config_map.clear();

		m_config_path = config_path;

		TiXmlDocument doc(config_path.c_str());
		TiXmlHandle hDoc(&doc);

		if (!doc.LoadFile(TIXML_ENCODING_UTF8))
		{
			return false;
		}

		//
		TiXmlElement* root_elem = hDoc.FirstChildElement().Element();

		return load_common(root_elem);
	}

	bool load_input_mapping(const std::string& config_path)
	{
		m_input_mapping.actions_mapping.clear();
		m_input_mapping.states_mapping.clear();
		m_input_mapping.ranges_mapping.clear();

		m_config_path = config_path;

		TiXmlDocument doc(config_path.c_str());
		TiXmlHandle hDoc(&doc);

		if (!doc.LoadFile(TIXML_ENCODING_UTF8))
		{
			return false;
		}

		//
		TiXmlElement* root_elem = hDoc.FirstChildElement().Element();

		return load_input_mapping(root_elem);
	}
#endif

	bool save()
	{
		TiXmlDocument doc(m_config_path.c_str());
		TiXmlHandle hDoc(&doc);

		if (!doc.LoadFile(TIXML_ENCODING_UTF8))
		{
			return false;
		}

		//
		TiXmlElement* root_elem = hDoc.FirstChildElement().Element();

		auto save_params = [](TiXmlElement* params_elem,
			std::map<std::string, std::string> config_map_copy,
			const std::set<std::string>& check_params) -> void
		{
			// iterate through params
			if (params_elem)
			{
				auto param_elem = params_elem->FirstChildElement("param");

				while (param_elem)
				{
					auto param_name = param_elem->Attribute("name");
					auto param_value = param_elem->Attribute("value");

					if (param_name)
					{
						auto it = config_map_copy.find(param_name);

						if (it != config_map_copy.end())
						{
							param_elem->SetAttribute("value", it->second.c_str());

							config_map_copy.erase(it);
						}
					}

					//
					param_elem = param_elem->NextSiblingElement("param");
				}
			}

			// save missing params
			if (config_map_copy.size() > 0)
			{
				for (const auto& check_param : check_params)
				{
					auto it = config_map_copy.find(check_param);
					if (it != config_map_copy.end())
					{
						TiXmlElement new_param("param");
						new_param.SetAttribute("name", it->first.c_str());
						new_param.SetAttribute("value", it->second.c_str());
						params_elem->InsertEndChild(new_param);

						config_map_copy.erase(it);
					}
				}
			}
		};

		// save graphics settings
		{
			TiXmlElement* graphics_node_iterator = root_elem->FirstChildElement("graphics");

			save_params(graphics_node_iterator->FirstChildElement("params"),
				m_config_map,
				{
					"monitor_index",
					"screen_width",
					"screen_height",
					"window_fullscreen_mode"
				});

			save_params(graphics_node_iterator->FirstChildElement("renderer"),
				m_config_map,
				{
					"shade_model",
					"texture_mode"
				});
		}

		// save audio settings
		{
			TiXmlElement* audio_node_iterator = root_elem->FirstChildElement("audio");

			save_params(audio_node_iterator->FirstChildElement("params"),
				m_config_map,
				{
					"music_enabled",
					"sounds_enabled"
				});
		}

		// save_input_params
		{
			auto available_joysticks =
				input_manager_c::me()->enumerate_joysticks();

			TiXmlElement* input_node_iterator = root_elem->FirstChildElement("input");
			if (!input_node_iterator)
			{
				input_node_iterator = root_elem->InsertEndChild(TiXmlElement("input"))->ToElement();
			}

			TiXmlElement* game_controllers_node = input_node_iterator->FirstChildElement("game_controllers");
			if (!game_controllers_node)
			{
				game_controllers_node = input_node_iterator->InsertEndChild(TiXmlElement("game_controllers"))->ToElement();
			}

			// remove all childs and rebuild game_controllers node
			TiXmlElement* game_controllers_node_iterator = game_controllers_node->FirstChildElement();
			while (game_controllers_node_iterator)
			{
				auto next_node_iterator = game_controllers_node_iterator->NextSiblingElement();

				game_controllers_node->RemoveChild(game_controllers_node_iterator);

				//
				game_controllers_node_iterator = next_node_iterator;
			}

			for (const auto& game_controller : available_joysticks)
			{
				TiXmlElement new_node("game_controller");
				{
					new_node.SetAttribute("id", game_controller.id);
					new_node.SetAttribute("name", game_controller.name.c_str());
					new_node.SetAttribute("guid", game_controller.guid_str.c_str());
				}
				game_controllers_node->InsertEndChild(new_node);
			}

			TiXmlElement* input_mapping_node = input_node_iterator->FirstChildElement("input_mapping");
			if (!input_mapping_node)
			{
				input_mapping_node = input_node_iterator->InsertEndChild(TiXmlElement("input_mapping"))->ToElement();
			}

			// remove all childs and rebuild input params
			input_node_iterator = input_mapping_node->FirstChildElement();
			while (input_node_iterator)
			{
				auto next_node_iterator = input_node_iterator->NextSiblingElement();

				input_mapping_node->RemoveChild(input_node_iterator);

				//
				input_node_iterator = next_node_iterator;
			}

			auto create_xml_input_param = [&](e_input_actions action, const mapping_event_s& mapping_config)
			{
				TiXmlElement new_node("input");
				{
					new_node.SetAttribute("action", convert_action_type_to_str(action).c_str());
					new_node.SetAttribute("source", mapping_config.event_type);
					new_node.SetAttribute("scancode1", mapping_config.scancode);
					new_node.SetAttribute("scancode2", mapping_config.scancode);
					new_node.SetAttribute("joystick_id", mapping_config.joystick_id);
					new_node.SetAttribute("jasix_id", mapping_config.joystick_control_id);
					new_node.SetAttribute("jaxis_val", mapping_config.joystick_jvalue);
					new_node.SetAttribute("joy_button", mapping_config.jbutton_id);
					new_node.SetAttribute("jhat_id", mapping_config.joystick_control_id);
					new_node.SetAttribute("jhat_button", mapping_config.joystick_jvalue);
				}

				input_mapping_node->InsertEndChild(new_node);
			};

			for (const auto& action : m_input_mapping.actions_mapping)
			{
				create_xml_input_param(action.second, action.first);
			}
		}

		return doc.SaveFile(m_config_path.c_str());
	}

	void clear()
	{
		m_config_map.clear();
		m_input_mapping.actions_mapping.clear();
		m_input_mapping.states_mapping.clear();
		m_input_mapping.ranges_mapping.clear();
	}

	const std::string& get_string(const std::string& param_name,
		const std::string& default_value = std::string()) const
	{
		auto it = m_config_map.find(param_name);

		if (it != m_config_map.end())
		{
			return it->second;
		}

		return default_value;
	}

	int get_integer(const std::string& param_name,
		int default_value = 0) const
	{
		try
		{
			auto it = m_config_map.find(param_name);

			if (it != m_config_map.end())
			{
				return std::stoi(it->second);
			}
		}
		catch (const std::exception&)
		{
		}

		return default_value;
	}

	bool get_bool(const std::string& param_name,
		bool default_value = false) const
	{
		return get_integer(param_name, 0) == 1;
	}

	void get_mapping(input_mapper_c& mapper)
	{
		mapper.set_mapping(m_input_mapping);
	}

	void set_mapping(input_mapper_c& mapper)
	{
		m_input_mapping = mapper.get_mapping();

		// rebuild states by action-list
		m_input_mapping.states_mapping.clear();
		complete_mapping();
	}

	void set_param(const std::string& name, const std::string& value)
	{
		m_config_map[name] = value;
	}

private:
	bool load_common(TiXmlElement* root_elem)
	{
		TiXmlElement* graphics_node_iterator = root_elem->FirstChildElement("graphics");

		if (graphics_node_iterator)
		{
			// iterate through params
			auto params_elem = graphics_node_iterator->FirstChildElement("params");
			if (params_elem)
			{
				auto param_elem = params_elem->FirstChildElement("param");

				while (param_elem)
				{
					auto param_name = param_elem->Attribute("name");
					auto param_value = param_elem->Attribute("value");

					if (param_name && param_value)
					{
						m_config_map.emplace(std::make_pair(param_name, param_value));
					}

					//
					param_elem = param_elem->NextSiblingElement("param");
				}
			}

			// iterate through renderer
			auto renderer_elem = graphics_node_iterator->FirstChildElement("renderer");
			if (renderer_elem)
			{
				auto param_elem = renderer_elem->FirstChildElement("param");

				while (param_elem)
				{
					auto param_name = param_elem->Attribute("name");
					auto param_value = param_elem->Attribute("value");

					if (param_name && param_value)
					{
						m_config_map.emplace(std::make_pair(param_name, param_value));
					}

					//
					param_elem = param_elem->NextSiblingElement("param");
				}
			}
		}

		TiXmlElement* audio_node_iterator = root_elem->FirstChildElement("audio");

		if (audio_node_iterator)
		{
			// iterate through params
			auto params_elem = audio_node_iterator->FirstChildElement("params");
			if (params_elem)
			{
				auto param_elem = params_elem->FirstChildElement("param");

				while (param_elem)
				{
					auto param_name = param_elem->Attribute("name");
					auto param_value = param_elem->Attribute("value");

					if (param_name && param_value)
					{
						m_config_map.emplace(std::make_pair(param_name, param_value));
					}

					//
					param_elem = param_elem->NextSiblingElement("param");
				}
			}
		}

		return m_config_map.size() > 0;
	}

	bool load_input_mapping(TiXmlElement* root_elem)
	{
		auto available_joysticks =
			input_manager_c::me()->enumerate_joysticks();

		TiXmlElement* node_iterator = root_elem->FirstChildElement("input");

		if (node_iterator)
		{
			// iterate through params
			auto params_elem = node_iterator->FirstChildElement("params");
			if (params_elem)
			{
				auto param_elem = params_elem->FirstChildElement("param");

				while (param_elem)
				{

					//
					param_elem = param_elem->NextSiblingElement("param");
				}
			}

			// iterate through key_mapping
			auto key_mapping_elem = node_iterator->FirstChildElement("input_mapping");
			if (key_mapping_elem)
			{
				auto key_elem = key_mapping_elem->FirstChildElement("input");

				while (key_elem)
				{
					auto attr_source = key_elem->Attribute("source");

					auto attr_state = key_elem->Attribute("state");
					auto attr_action = key_elem->Attribute("action");

					if (!attr_source || !(attr_state || attr_action))
					{
						key_elem = key_elem->NextSiblingElement("input");
						continue;
					}

					auto attr_scancode1 = key_elem->Attribute("scancode1");
					auto attr_scancode2 = key_elem->Attribute("scancode2");

					auto attr_joystick_id = key_elem->Attribute("joystick_id");

					auto attr_jasix_id = key_elem->Attribute("jasix_id");
					auto attr_jaxis_val = key_elem->Attribute("jaxis_val");

					auto attr_jhat_id = key_elem->Attribute("jhat_id");
					auto attr_jhat_button = key_elem->Attribute("jhat_button");

					auto attr_joy_button = key_elem->Attribute("joy_button");

					try
					{
						auto attr_source_int = std::stoi(attr_source);

						if (attr_source_int == SDL_KEYDOWN)
						{
							if (attr_state)
							{
								auto state_type = convert_str_to_state_type(attr_state);

								if (attr_scancode1)
								{
									m_input_mapping.states_mapping[mapping_event_s((SDL_Scancode)std::stoi(attr_scancode1))] = state_type;
								}

								if (attr_scancode2)
								{
									m_input_mapping.states_mapping[mapping_event_s((SDL_Scancode)std::stoi(attr_scancode2))] = state_type;
								}

							}
							else if (attr_action)
							{
								auto action_type = convert_str_to_action_type(attr_action);

								if (attr_scancode1)
								{
									auto sdl_code = (SDL_Scancode)std::stoi(attr_scancode1);

									m_input_mapping.actions_mapping[mapping_event_s(sdl_code)]
										= action_type;
								}

								if (attr_scancode2)
								{
									auto sdl_code = (SDL_Scancode)std::stoi(attr_scancode2);

									m_input_mapping.actions_mapping[mapping_event_s(sdl_code)] = action_type;
								}
							}
						}
						else if (attr_source_int == SDL_JOYAXISMOTION
							&& attr_joystick_id && attr_jasix_id && attr_jaxis_val)
						{
							auto attr_joystick_id_int = (SDL_JoystickID)std::stoi(attr_joystick_id);
							auto attr_jasix_id_int = std::stoi(attr_jasix_id);
							auto attr_jaxis_val_int = std::stoi(attr_jaxis_val);

							mapping_event_s mapping_event(attr_source_int,
								attr_joystick_id_int,
								attr_jasix_id_int,
								attr_jaxis_val_int > 0 ? joystick_edgezone_value + 1 : -joystick_edgezone_value - 1);

							if (attr_state)
							{
								auto state_type = convert_str_to_state_type(attr_state);
								m_input_mapping.states_mapping[mapping_event] = state_type;
							}
							else if (attr_action)
							{
								auto action_type = convert_str_to_action_type(attr_action);
								m_input_mapping.actions_mapping[mapping_event] = action_type;
							}
						}
						else if (attr_source_int == SDL_JOYHATMOTION
							&& attr_joystick_id && attr_jhat_id && attr_jhat_button)
						{
							auto attr_joystick_id_int = (SDL_JoystickID)std::stoi(attr_joystick_id);
							auto attr_jhat_id_int = std::stoi(attr_jhat_id);
							auto attr_jhat_button_int = (uint8_t)std::stoi(attr_jhat_button);

							mapping_event_s mapping_event(attr_source_int,
								attr_joystick_id_int,
								attr_jhat_id_int,
								attr_jhat_button_int);

							if (attr_state)
							{
								auto state_type = convert_str_to_state_type(attr_state);
								m_input_mapping.states_mapping[mapping_event] = state_type;
							}
							else if (attr_action)
							{
								auto action_type = convert_str_to_action_type(attr_action);
								m_input_mapping.actions_mapping[mapping_event] = action_type;
							}
						}
						else if (attr_source_int == SDL_JOYBUTTONDOWN
							&& attr_joystick_id && attr_jhat_button)
						{
							auto attr_joystick_id_int = (SDL_JoystickID)std::stoi(attr_joystick_id);
							auto attr_joy_button_int = (uint8_t)std::stoi(attr_joy_button);

							mapping_event_s mapping_event(attr_source_int,
								attr_joystick_id_int,
								attr_joy_button_int);

							if (attr_state)
							{
								auto state_type = convert_str_to_state_type(attr_state);
								m_input_mapping.states_mapping[mapping_event] = state_type;
							}
							else if (attr_action)
							{
								auto action_type = convert_str_to_action_type(attr_action);
								m_input_mapping.actions_mapping[mapping_event] = action_type;
							}
						}
					}
					catch (const std::exception&)
					{
					}

					//
					key_elem = key_elem->NextSiblingElement("input");
				}
			}
		}

		complete_mapping();

		extern std::map<mapping_event_s, e_input_actions, std::less<>>::iterator find_mapping_by_action(input_mapping_s& mapping, e_input_actions action);

		if (find_mapping_by_action(m_input_mapping, e_input_actions::action_show_debug_info)
			== m_input_mapping.actions_mapping.cend())
		{
			m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_F1)]
				= e_input_actions::action_show_debug_info;
		}

		if (find_mapping_by_action(m_input_mapping, e_input_actions::action_take_screenshot)
			== m_input_mapping.actions_mapping.cend())
		{
			m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_PRINTSCREEN)]
				= e_input_actions::action_take_screenshot;
			m_input_mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_F12)]
				= e_input_actions::action_take_screenshot;
		}

		return m_input_mapping.actions_mapping.size() > 0 &&
			m_input_mapping.states_mapping.size() > 0/* &&
			m_ranges_mapping.size() > 0*/;
	}

	e_input_actions convert_str_to_action_type(const std::string& str)
	{
		if (str == "action_plane_bomb")
			return e_input_actions::action_plane_bomb;
		else if (str == "action_show_debug_info")
			return e_input_actions::action_show_debug_info;
		else if (str == "action_take_screenshot")
			return e_input_actions::action_take_screenshot;
		else if (str == "action_turn_left")
			return e_input_actions::action_turn_left;
		else if (str == "action_turn_right")
			return e_input_actions::action_turn_right;
		else if (str == "action_turn_up")
			return e_input_actions::action_turn_up;
		else if (str == "action_turn_down")
			return e_input_actions::action_turn_down;
		else if (str == "action_quit_game")
			return e_input_actions::action_quit_game;
		else if (str == "action_change_view")
			return e_input_actions::action_change_view;

		return e_input_actions::action_none;
	}

	std::string convert_action_type_to_str(e_input_actions action)
	{
		if (action == e_input_actions::action_plane_bomb)
			return "action_plane_bomb";
		else if (action == e_input_actions::action_show_debug_info)
			return "action_show_debug_info";
		else if (action == e_input_actions::action_take_screenshot)
			return "action_take_screenshot";
		else if (action == e_input_actions::action_turn_left)
			return "action_turn_left";
		else if (action == e_input_actions::action_turn_right)
			return "action_turn_right";
		else if (action == e_input_actions::action_turn_up)
			return "action_turn_up";
		else if (action == e_input_actions::action_turn_down)
			return "action_turn_down";
		else if (action == e_input_actions::action_quit_game)
			return "action_quit_game";
		else if (action == e_input_actions::action_change_view)
			return "action_change_view";

		return "action_none";
	}

	e_input_states convert_str_to_state_type(const std::string& str)
	{
		if (str == "state_run_left")
			return e_input_states::state_run_left;
		else if (str == "state_run_right")
			return e_input_states::state_run_right;
		else if (str == "state_run_forward")
			return e_input_states::state_run_forward;
		else if (str == "state_run_backward")
			return e_input_states::state_run_backward;

		return e_input_states::state_none;
	}

	void complete_mapping()
	{
		// build states by action-list

		for (auto& it : m_input_mapping.actions_mapping)
		{
			if (it.second == action_turn_left)
				m_input_mapping.states_mapping[it.first] = state_run_left;
			if (it.second == action_turn_right)
				m_input_mapping.states_mapping[it.first] = state_run_right;
			if (it.second == action_turn_up)
				m_input_mapping.states_mapping[it.first] = state_run_forward;
			if (it.second == action_turn_down)
				m_input_mapping.states_mapping[it.first] = state_run_backward;
		}
	}

private:
	std::string m_config_path;

	std::map<std::string, std::string> m_config_map;

	input_mapping_s m_input_mapping;
};
