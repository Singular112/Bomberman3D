#include "stdafx.h"
#include "main_screen_c.h"

#include <engine3d/helpers.h>

#include "config_c.h"


std::string get_control_name(const mapping_event_s& event)
{
	std::string control_name;

	if (event.event_type == SDL_KEYDOWN)
	{
		control_name = SDL_GetKeyName(SDL_GetKeyFromScancode(event.scancode));
	}
	else if (event.event_type == SDL_JOYBUTTONDOWN)
	{
		auto jbutton_which = event.joystick_id;
		auto jbutton_id = event.jbutton_id;

		control_name = std_string_printf("JB_%d_%d", 32,
			jbutton_which,
			jbutton_id);
	}
	else if (event.event_type == SDL_JOYHATMOTION)
	{
		auto jhat_which = event.joystick_id;
		auto jhat_id = event.joystick_control_id;
		auto jhat_value = event.joystick_jvalue;

		control_name = std_string_printf("JH_%d_%d_%d", 32,
			jhat_which,
			jhat_id,
			jhat_value);
	}
	else if (event.event_type == SDL_JOYAXISMOTION)
	{
		auto jaxis_which = event.joystick_id;
		auto jaxis_id = event.joystick_control_id;
		auto jaxis_val = event.joystick_jvalue;

#if 0
		auto key_name = std_string_printf("JA_%d_%d_%d", 32,
			jaxis_which,
			jaxis_id,
			jaxis_val);
#else
		control_name = std_string_printf("JA_%d_%d_%d", 32,
			jaxis_which,
			jaxis_id,
			jaxis_val > 0 ? 1 : 2);
#endif
	}

	return control_name;
}


std::string get_control_name(const SDL_Event* event)
{
	return get_control_name(mapping_event_s(event));
}


std::map<mapping_event_s, e_input_actions, std::less<>>::iterator find_mapping_by_action(input_mapping_s& mapping, e_input_actions action)
{
	for (auto it = mapping.actions_mapping.begin();
		it != mapping.actions_mapping.end();
		it++)
	{
		if (it->second == action)
		{
			return it;
		}
	}

	return mapping.actions_mapping.end();
}


main_screen_c::main_screen_c()
{
	m_current_state = e_menu_mainscreen;
}


main_screen_c::~main_screen_c()
{
	if (m_background_fib)
	{
		FreeImage_Unload(m_background_fib);
		m_background_fib = nullptr;
	}
}


bool main_screen_c::initialize(const std::string& resource_dir,
	const viewport_settings_s& viewport_settings,
	SDL_Window* sdl_window,
	framebuffer_c* buffer)
{
	// IMPORTANT. save action-list order according to order of control settings menu items
	m_input_actions_list =
	{
		action_turn_left,
		action_turn_right,
		action_turn_up,
		action_turn_down,
		action_plane_bomb,
		action_change_view
	};

	// menu control mepping
	initialize_input_mapper();

	m_sdl_window = sdl_window;
	m_framebuffer = buffer;
	m_viewport_settings = viewport_settings;

	// menu background
	{
		std::string background_img_path = resource_dir + "/main_logo.png";
		m_background_fib = FreeImage_Load(FIF_PNG, background_img_path.c_str());

		if (!m_background_fib)
		{
			return false;
		}

		auto bg_width = FreeImage_GetWidth(m_background_fib);
		auto bg_height = FreeImage_GetHeight(m_background_fib);

		if (bg_width != viewport_settings.screen_width ||
			bg_height != viewport_settings.screen_height)
		{
			auto rescaled_fib = FreeImage_Rescale
			(
				m_background_fib,
				m_viewport_settings.screen_width,
				m_viewport_settings.screen_height,
				FILTER_LANCZOS3
			);

			FreeImage_Unload(m_background_fib);
			m_background_fib = rescaled_fib;

			if (m_background_fib == nullptr)
			{
				return false;
			}

			if (FreeImage_GetBPP(m_background_fib) != 32)
			{
				auto dib_argb32bits = FreeImage_ConvertTo32Bits(m_background_fib);
				FreeImage_Unload(m_background_fib);
				m_background_fib = dib_argb32bits;
			}
		}

		FreeImage_FlipVertical(m_background_fib);

		m_data_argb32bits = (decltype(m_data_argb32bits))FreeImage_GetBits(m_background_fib);
	}

	m_mainmenu_font = resource_manager_c::me()->get_font(resource_manager_c::e_font_bauhaus_93_28);

	const SDL_Color white_color = { 255, 255, 255, 0 };
	const SDL_Color gray_color = { 200, 200, 200, 0 };

	int center_x_coord = (int)(m_framebuffer->get_buffer_width() * 0.5f);

	float center_y_of_items_area = viewport_settings.screen_height * 0.64f;
	int items_padding = 50;
	auto compute_y_item_pos = [&](int items_count, int item_index) -> int
	{
		auto items_area_height = items_count * items_padding;

		auto first_y_item_pos = center_y_of_items_area - (items_area_height * 0.5f);

		return (int)(first_y_item_pos + item_index * items_padding);
	};

	// build main screen menu
	{
		menu_item_button_c* item_start_game = new menu_item_button_c(0, center_x_coord, compute_y_item_pos(3, 0), m_mainmenu_font,
			white_color, gray_color,
			"START GAME");
		menu_item_button_c* item_settings = new menu_item_button_c(1, center_x_coord, compute_y_item_pos(3, 1), m_mainmenu_font,
			white_color, gray_color,
			"SETTINGS");
		menu_item_button_c* item_quit = new menu_item_button_c(2, center_x_coord, compute_y_item_pos(3, 2), m_mainmenu_font,
			white_color, gray_color,
			"QUIT");

#if 0
		item_start_game += on_mouse_over_handler(
			std::bind(&main_screen_c::on_mouse_over_item, this, std::placeholders::_1));
		item_settings += on_mouse_over_handler(
			std::bind(&main_screen_c::on_mouse_over_item, this, std::placeholders::_1));
		item_quit += on_mouse_over_handler(
			std::bind(&main_screen_c::on_mouse_over_item, this, std::placeholders::_1));

		item_start_game += on_mouse_out_handler(
			std::bind(&main_screen_c::on_mouse_out_item, this, std::placeholders::_1));
		item_settings += on_mouse_out_handler(
			std::bind(&main_screen_c::on_mouse_out_item, this, std::placeholders::_1));
		item_quit += on_mouse_out_handler(
			std::bind(&main_screen_c::on_mouse_out_item, this, std::placeholders::_1));
#endif

		*item_start_game += on_mouse_click_handler(
			std::bind(&main_screen_c::on_mouse_click_item, this, std::placeholders::_1));
		*item_settings += on_mouse_click_handler(
			std::bind(&main_screen_c::on_mouse_click_item, this, std::placeholders::_1));
		*item_quit += on_mouse_click_handler(
			std::bind(&main_screen_c::on_mouse_click_item, this, std::placeholders::_1));

		m_mainmenu_items.emplace_back(item_start_game);
		m_mainmenu_items.emplace_back(item_settings);
		m_mainmenu_items.emplace_back(item_quit);

		m_input_handlers[e_menu_mainscreen] = &m_mainmenu_items;
	}

	// build settings menu
	{
		menu_item_button_c* item_graphic_settings = new menu_item_button_c(3, center_x_coord, compute_y_item_pos(3, 0), m_mainmenu_font,
			white_color, gray_color,
			"GRAPHIC SETTINGS");
		menu_item_button_c* item_sound_settings = new menu_item_button_c(4, center_x_coord, compute_y_item_pos(3, 1), m_mainmenu_font,
			white_color, gray_color,
			"SOUND SETTINGS");
		menu_item_button_c* item_control_settings = new menu_item_button_c(5, center_x_coord, compute_y_item_pos(3, 2), m_mainmenu_font,
			white_color, gray_color,
			"CONTROL SETTINGS");

		*item_graphic_settings += on_mouse_click_handler(
			std::bind(&main_screen_c::on_mouse_click_item, this, std::placeholders::_1));
		*item_sound_settings += on_mouse_click_handler(
			std::bind(&main_screen_c::on_mouse_click_item, this, std::placeholders::_1));
		*item_control_settings += on_mouse_click_handler(
			std::bind(&main_screen_c::on_mouse_click_item, this, std::placeholders::_1));

		m_settings_menu_items.emplace_back(item_graphic_settings);
		m_settings_menu_items.emplace_back(item_sound_settings);
		m_settings_menu_items.emplace_back(item_control_settings);

		m_input_handlers[e_menu_settings] = &m_settings_menu_items;
	}

	// graphics settings
	{

		m_screen_resoulutions =
		{
			{ 800, 600 },
			{ 1024, 768 },
			{ 1280, 720 },
			{ 1280, 1024 },
			{ 1360, 768 },
			{ 1600, 900 },
			{ 1920, 1080 },
			{ 2048, 1152 }
		};

		const screen_resoulution_s config_wnd_resolution =
		{
			config_c::me()->get_integer("screen_width", 1280),
			config_c::me()->get_integer("screen_height", 720)
		};

		std::string config_wnd_resolution_str = get_resolution_str(config_wnd_resolution);

		int selected_resolution = 0;
		std::vector<std::string> resolution_strs;
		for (size_t i = 0; i < m_screen_resoulutions.size(); i++)
		{
			const auto& res = m_screen_resoulutions[i];

			auto resolution_str = get_resolution_str(res);

			if (resolution_str == config_wnd_resolution_str)
			{
				selected_resolution = i;
			}

			resolution_strs.emplace_back(resolution_str);
		}

		auto item1 = new menu_item_param_c<std::string>(100, center_x_coord - 150, center_x_coord + 150,
			compute_y_item_pos(3, 0),
			m_mainmenu_font, white_color, gray_color,
			"screen resolution",
			resolution_strs,
			resolution_strs,
			selected_resolution);

		auto item2 = new menu_item_param_c<bool>(101, center_x_coord - 150, center_x_coord + 150,
			compute_y_item_pos(3, 1),
			m_mainmenu_font, white_color, gray_color,
			"fullscreen mode",
			{ "yes", "no" },
			{ true, false },
			(config_c::me()->get_bool("window_fullscreen_mode") ? 0 : 1)
		);

		menu_item_button_c* apply_item = new menu_item_button_c(102, center_x_coord,
			compute_y_item_pos(3, 2), m_mainmenu_font,
			white_color, gray_color,
			"apply");

		*apply_item += on_mouse_click_handler(
			std::bind(&main_screen_c::on_mouse_click_item, this, std::placeholders::_1));

		m_graphics_settings_items.emplace_back(item1);
		m_graphics_settings_items.emplace_back(item2);
		m_graphics_settings_items.emplace_back(apply_item);

		m_input_handlers[e_menu_graphics_settings] = &m_graphics_settings_items;
	}

	// sound settings
	{
		bool music_enabled = config_c::me()->get_bool("music_enabled", 1);
		bool sounds_enabled = config_c::me()->get_bool("sounds_enabled", 1);

		auto item1 = new menu_item_param_c<bool>(200, center_x_coord - 100, center_x_coord + 100,
			compute_y_item_pos(3, 0),
			m_mainmenu_font, white_color, gray_color,
			"music",
			{ "on", "off" },
			{ true, false },
			music_enabled ? 0 : 1);

		auto item2 = new menu_item_param_c<bool>(201, center_x_coord - 100, center_x_coord + 100,
			compute_y_item_pos(3, 1),
			m_mainmenu_font, white_color, gray_color,
			"sounds",
			{ "on", "off" },
			{ true, false },
			sounds_enabled ? 0 : 1);

		menu_item_button_c* apply_item = new menu_item_button_c(202, center_x_coord,
			compute_y_item_pos(3, 2), m_mainmenu_font,
			white_color, gray_color,
			"apply");

		*apply_item += on_mouse_click_handler(
			std::bind(&main_screen_c::on_mouse_click_item, this, std::placeholders::_1));

		m_sound_settings_items.emplace_back(item1);
		m_sound_settings_items.emplace_back(item2);
		m_sound_settings_items.emplace_back(apply_item);

		m_input_handlers[e_menu_sound_settings] = &m_sound_settings_items;
	}

	// control settings
	{
		int items_inner_padding = 150;

		page_s control_settings_page;
		control_settings_page.x = center_x_coord - items_inner_padding;
		control_settings_page.x = 320;

		auto move_left_item = new menu_item_param_c<mapping_event_s>(300, center_x_coord - items_inner_padding, center_x_coord + items_inner_padding,
			compute_y_item_pos(6, 0),
			m_mainmenu_font, white_color, gray_color,
			"move left",
			{ "A" },
			{ mapping_event_s() },
			0);

		auto move_right_item = new menu_item_param_c<mapping_event_s>(301, center_x_coord - items_inner_padding, center_x_coord + items_inner_padding,
			compute_y_item_pos(6, 1),
			m_mainmenu_font, white_color, gray_color,
			"move right",
			{ "D" },
			{ mapping_event_s() },
			0);

		auto move_forward_item = new menu_item_param_c<mapping_event_s>(302, center_x_coord - items_inner_padding, center_x_coord + items_inner_padding,
			compute_y_item_pos(6, 2),
			m_mainmenu_font, white_color, gray_color,
			"move forward",
			{ "W" },
			{ mapping_event_s() },
			0);

		auto move_backward_item = new menu_item_param_c<mapping_event_s>(303, center_x_coord - items_inner_padding, center_x_coord + items_inner_padding,
			compute_y_item_pos(6, 3),
			m_mainmenu_font, white_color, gray_color,
			"move backward",
			{ "S" },
			{ mapping_event_s() },
			0);

		auto place_bomb_item = new menu_item_param_c<mapping_event_s>(304, center_x_coord - items_inner_padding, center_x_coord + items_inner_padding,
			compute_y_item_pos(6, 4),
			m_mainmenu_font, white_color, gray_color,
			"place bomb",
			{ "SPACE" },
			{ mapping_event_s() },
			0);

		auto change_camview_item = new menu_item_param_c<mapping_event_s>(305, center_x_coord - items_inner_padding, center_x_coord + items_inner_padding,
			compute_y_item_pos(6, 5),
			m_mainmenu_font, white_color, gray_color,
			"switch cam view",
			{ "SPACE" },
			{ mapping_event_s() },
			0);

		menu_item_button_c* apply_item = new menu_item_button_c(306, center_x_coord,
			compute_y_item_pos(6, 6),
			m_mainmenu_font,
			white_color, gray_color,
			"apply");

		*apply_item += on_mouse_click_handler(
			std::bind(&main_screen_c::on_mouse_click_item, this, std::placeholders::_1));

		m_control_settings_items.emplace_back(move_left_item);
		m_control_settings_items.emplace_back(move_right_item);
		m_control_settings_items.emplace_back(move_forward_item);
		m_control_settings_items.emplace_back(move_backward_item);
		m_control_settings_items.emplace_back(place_bomb_item);
		m_control_settings_items.emplace_back(change_camview_item);
		m_control_settings_items.emplace_back(apply_item);

		m_input_handlers[e_menu_control_settings] = &m_control_settings_items;
	}

	// setup states update function
	{
		m_menu_states[e_menu_mainscreen]
			= std::bind(&main_screen_c::update_menu<decltype(m_mainmenu_items)>,
				this,
				std::ref(m_mainmenu_items));
		m_menu_states[e_menu_settings]
			= std::bind(&main_screen_c::update_menu<decltype(m_settings_menu_items)>,
				this,
				std::ref(m_settings_menu_items));
		m_menu_states[e_menu_graphics_settings]
			= std::bind(&main_screen_c::update_menu<decltype(m_graphics_settings_items)>,
				this,
				std::ref(m_graphics_settings_items));
		m_menu_states[e_menu_sound_settings]
			= std::bind(&main_screen_c::update_menu<decltype(m_sound_settings_items)>,
				this,
				std::ref(m_sound_settings_items));
		m_menu_states[e_menu_control_settings]
			= std::bind(&main_screen_c::update_menu<decltype(m_control_settings_items)>,
				this,
				std::ref(m_control_settings_items));
	}

	return true;
}


void main_screen_c::on_enter_state()
{
	input_manager_c::me()->append_callback(std::bind(&main_screen_c::handle_input,
		this,
		std::placeholders::_1));

	SDL_ShowCursor(SDL_ENABLE);

	if (m_menu_entry_count++ == 0 && !sound_manager_c::me()->is_music_playing())
	{
		sound_manager_c::me()->play_music(resource_manager_c::e_music_mainmenu);
	}
}


bool main_screen_c::is_on_main_page() const
{
	return m_state_stack.size() == 0;
}


std::string main_screen_c::get_resolution_str(const screen_resoulution_s& res)
{
	return std::to_string(res.w) +
		" x " +
		std::to_string(res.h);
}


void main_screen_c::initialize_input_mapper()
{
	input_mapping_s mapping;

	mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_LEFT)]			= e_input_actions::action_left_key_pressed;
	mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_RIGHT)]		= e_input_actions::action_right_key_pressed;
	mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_UP)]			= e_input_actions::action_up_key_pressed;
	mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_DOWN)]			= e_input_actions::action_down_key_pressed;
	mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_ESCAPE)]		= e_input_actions::action_escape_key_pressed;
	mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_RETURN)]		= e_input_actions::action_enter_key_pressed;

	mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_F12)]			= e_input_actions::action_take_screenshot;
	mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_PRINTSCREEN)]	= e_input_actions::action_take_screenshot;

	m_menu_input_mapper.set_mapping(mapping);
}


void main_screen_c::set_input_mapper()
{
	input_manager_c::me()->set_input_mapper(&m_menu_input_mapper);

	// get copy of game input mapper from config
	config_c::me()->get_mapping(m_game_input_mapper);

	// setup input menu settings state according to actual game mapping
	auto mapping = m_game_input_mapper.get_mapping();

	for (size_t i = 0; i < m_input_actions_list.size(); ++i)
	{
		auto menu_item_param = (menu_item_param_c<mapping_event_s>*)m_control_settings_items[i].get();

		auto action_it = find_mapping_by_action(mapping, m_input_actions_list[i]);

		if (action_it == mapping.actions_mapping.cend())
		{
			menu_item_param->set_text("?");
			menu_item_param->set_param_data(mapping_event_s());
		}
		else
		{
			auto control_name = get_control_name(action_it->first);
			menu_item_param->set_text(control_name);
			menu_item_param->set_param_data(action_it->first);
		}
	}
}


void main_screen_c::change_state(e_menu_state new_state)
{
	m_state_stack.emplace(m_current_state);

	m_current_state = new_state;
}


void main_screen_c::set_prev_state()
{
	if (m_state_stack.size() == 0)
	{
		return;
	}

	auto prev_state = m_state_stack.top();
	m_state_stack.pop();

	m_current_state = prev_state;
}


void main_screen_c::on_mouse_over_item(int item_id)
{
	m_selected_item_id = item_id;
}


void main_screen_c::on_mouse_out_item(int item_id)
{
	m_selected_item_id = item_id;
}


void main_screen_c::on_mouse_click_item(int item_id)
{
	if (item_id == 1)
	{
		change_state(e_menu_settings);
	}
	else if (item_id == 0)
	{
		if (m_on_new_game_callback)
		{
			if (m_menu_entry_count == 1)
			{
				Mix_HaltMusic();
			}

			m_on_new_game_callback();
		}
	}
	else if (item_id == 2)
	{
		if (m_on_exit_game_callback)
			m_on_exit_game_callback();
	}
	else if (item_id == 3)
	{
		change_state(e_menu_graphics_settings);
	}
	else if (item_id == 4)
	{
		change_state(e_menu_sound_settings);
	}
	else if (item_id == 5)
	{
		change_state(e_menu_control_settings);
	}
	else if (item_id == 102)	// save graphic settings
	{
		// convert to specifically menu objects
		const auto item1 = (menu_item_param_c<std::string>*)m_graphics_settings_items[0].get();
		const auto item2 = (menu_item_param_c<bool>*)m_graphics_settings_items[1].get();

		int selected_resolution_index = item1->get_selected_index();
		const auto& res = m_screen_resoulutions[selected_resolution_index];

		config_c::me()->set_param("screen_width", std::to_string(res.w));
		config_c::me()->set_param("screen_height", std::to_string(res.h));

		config_c::me()->set_param("window_fullscreen_mode",
			item2->get_param_data() ? "1" : "0");

		config_c::me()->save();

#if 0
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
			"", "You will have to restart the program for the changes to take effect",
			m_sdl_window);
#elif 0
		set_prev_state();
#else
		if (m_on_need_to_restart)
		{
			m_on_need_to_restart();
		}
		else
		{
			set_prev_state();
		}
#endif
	}
	else if (item_id == 202) // sound settings
	{
		// convert to specifically menu objects
		const auto item1 = (menu_item_param_c<bool>*)m_sound_settings_items[0].get();
		const auto item2 = (menu_item_param_c<bool>*)m_sound_settings_items[1].get();

		bool music_enabled = item1->get_param_data();

		config_c::me()->set_param("music_enabled", music_enabled ? "1" : "0");
		config_c::me()->set_param("sounds_enabled", item2->get_param_data() ? "1" : "0");

		if (!music_enabled)
			sound_manager_c::me()->stop_music();

		config_c::me()->save();
		set_prev_state();
	}
	else if (item_id == 306) // control settings
	{
		auto& mapping = m_game_input_mapper.get_mapping();

		// set mapping from control settings menu items states

		for (size_t i = 0; i < m_input_actions_list.size(); ++i)
		{
			// convert to specifically menu object
			auto menu_item_param = (menu_item_param_c<mapping_event_s>*)m_control_settings_items[i].get();

			auto input_action = m_input_actions_list[i];

			auto action_it = find_mapping_by_action(mapping, input_action);

			if (action_it != mapping.actions_mapping.cend())
				mapping.actions_mapping.erase(action_it);

			mapping.actions_mapping[menu_item_param->get_param_data()] = input_action;
		}

		// update mapping in config
		config_c::me()->set_mapping(m_game_input_mapper);

		config_c::me()->save();
		set_prev_state();
	}
}


void main_screen_c::handle_input(input_state_s* state)
{
	if (state->actions.find(e_input_actions::action_escape_key_pressed)
		!= state->actions.end())
	{
		set_prev_state();

		return;
	}

	// dispatch input events to current set of menu items
	auto& items_set = *m_input_handlers[m_current_state];
	for (auto& item : items_set)
	{
		item->handle_input(state);
	}
}


void main_screen_c::handle_sdl_input(const SDL_Event* event)
{
	if (m_current_state == e_menu_control_settings)
	{
		auto& items_set = *m_input_handlers[m_current_state];
		for (auto& item : items_set)
		{
			if (item->is_in_edit_mode())
			{
				// allowed input types
				if (event->type == SDL_KEYDOWN ||
					event->type == SDL_JOYBUTTONDOWN ||
					event->type == SDL_JOYHATMOTION ||
					event->type == SDL_JOYAXISMOTION)
				{
					if (event->type == SDL_JOYAXISMOTION)
					{
						auto jaxis_val = event->jaxis.value;

						if (!(jaxis_val < -joystick_edgezone_value || jaxis_val > joystick_edgezone_value) || jaxis_val == 0)
						{
							// joystick axis is in restricted zone
							return;
						}
					}

					auto control_name = get_control_name(event);

					auto menu_item_param = (menu_item_param_c<mapping_event_s>*)item.get();

					menu_item_param->set_text(control_name);

					mapping_event_s mapping_event(event);

					menu_item_param->set_param_data(mapping_event);

					item->finish_edit_mode();
				}
			}
		}
	}
}


void main_screen_c::update()
{
	auto buffer_ptr = m_framebuffer->get_data_ptr();

	// fill background
	sr::fast_memcpy(buffer_ptr, m_data_argb32bits, m_framebuffer->get_data_size());

	// update current state
	if (m_menu_states.find(m_current_state) != m_menu_states.cend())
	{
		m_menu_states[m_current_state]();
	}
}
