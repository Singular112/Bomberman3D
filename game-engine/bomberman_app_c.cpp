#include "stdafx.h"
#include "bomberman_app_c.h"

// helpers
#include "../common/helpers.h"


// game engine
#include "../game-engine/mesh_factory_c.h"
#include "../game-engine/game_map3d_c.h"
#include "../game-engine/bomberman3d_c.h"
#include "../game-engine/enemy3d_c.h"
#include "../game-engine/config_c.h"

int64_t g_last_frame_duration = 0;

//
bomberman_app_c::bomberman_app_c()
{
}


bomberman_app_c::~bomberman_app_c()
{
	m_running = false;

	resource_manager_c::free_instance();
	input_manager_c::free_instance();
	sound_manager_c::free_instance();
	config_c::free_instance();
	mesh_factory_c::free_instance();

	Mix_CloseAudio();

	if (m_app_thread)
	{
		int wait_status = 0;
		SDL_WaitThread(m_app_thread, &wait_status);
	}

	if (m_sdl_window)
	{
		SDL_DestroyWindow(m_sdl_window);
		m_sdl_window = nullptr;
	}

	if (m_zbuffer_ptr)
	{
		free_zbuffer(m_zbuffer_ptr.get());
	}

	if (m_background_fib)
	{
		FreeImage_Unload(m_background_fib);
		m_background_fib = nullptr;
	}

	sr_free_resources();
}

#if 0
float xoffset = 0, yoffset = 0, xrel = 0, yrel = 0, c = 0, d = 0;
float yaw = -90.0f;
float pitch = 0.0f;
uint32_t keymap[1024];
#endif

int bomberman_app_c::exec()
{
	if (!initialize())
	{
		return e_code_initialize_failed;
	}

	set_game_state(e_game_state_menu);

	m_running = true;

	while (m_running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
#if 0
			debug_output("event->type: %d, %x\n", 256,
				event.type,
				event.type);
#endif

			if (m_current_app_state == m_current_game_level.get())
			{
#if 0
				if (event.type == SDL_MOUSEWHEEL)
				{
					ang += event.wheel.y;
				}
#endif

#if 0
				if (m_free_fly_mode)
				{
					if (event.type == SDL_MOUSEMOTION)
					{
						static bool sfm_flag = true;
						if (sfm_flag)
						{
							sfm_flag = false;
							set_free_fly_mode();
						}

						float xrel = (float)event.motion.xrel;
						float yrel = (float)event.motion.yrel;

						yaw -= xrel * 0.1f;
						pitch += yrel * 0.1f;
						if (pitch > 89.0f)
							pitch = 89.0f;
						if (pitch < -89.0f)
							pitch = -89.0f;

						SDL_WarpMouseInWindow(m_sdl_window,
							(int)(m_viewport_settings.screen_width / 2.0f),
							(int)(m_viewport_settings.screen_height / 2.0f));
					}

					if (event.key.keysym.scancode == SDL_SCANCODE_W)
					{
						camera_move_forward(m_current_camview->get_camera_ptr(), m_current_camview->get_speed());
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_S)
					{
						camera_move_backward(m_current_camview->get_camera_ptr(), m_current_camview->get_speed());
					}

					if (event.key.keysym.scancode == SDL_SCANCODE_A)
					{
						camera_strafe_left(m_current_camview->get_camera_ptr(), m_current_camview->get_speed());
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_D)
					{
						camera_strafe_right(m_current_camview->get_camera_ptr(), m_current_camview->get_speed());
					}

					static bool pressed = false;
					if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
					{
						if (!pressed)
							debug_output("yaw: %d, pitch: %d, pos: { %f, %f, %f }\n", 256,
								(int)yaw,
								(int)pitch,
								m_current_camview->get_camera_ptr()->position.x,
								m_current_camview->get_camera_ptr()->position.y,
								m_current_camview->get_camera_ptr()->position.z);

						pressed = true;
					}
					else
					{
						pressed = false;
					}

					camera_rotate(cam_ptr, yaw, 0, pitch);
				}
#endif
			}

			if (event.type == SDL_QUIT)
			{
				return e_code_ok;
			}
			else if (event.type == SDL_MOUSEMOTION
				|| event.type == SDL_KEYDOWN
				|| event.type == SDL_KEYUP
				|| event.type == SDL_MOUSEBUTTONDOWN
				
				|| event.type == SDL_JOYAXISMOTION
#if 0
				|| event.type == SDL_JOYBALLMOTION
#endif
#if 0
				|| event.type == SDL_JOYHATMOTION
#endif
				|| event.type == SDL_JOYBUTTONDOWN
				|| event.type == SDL_JOYBUTTONUP
				)
			{
#if 0
				if (event.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					//if (m_current_game_level->is_finished())
					{
						input_manager_c::me()->reset();

						create_game_level();

						set_game_state(e_game_state_game);

						break;
					}
				}
#endif

				input_manager_c::me()->handle_sdl_input(&event);

				if (m_current_game_state == e_game_state::e_game_state_menu)
				{
					m_main_screen.handle_sdl_input(&event);
				}
				else
				{
#if 0
					if (event.type == SDL_MOUSEBUTTONDOWN)
					{
						if (event.button.button == 1)
						{
							int mouse_x, mouse_y;
							SDL_GetMouseState(&mouse_x, &mouse_y);

							OutputDebugStringA(std_string_printf(
								"{ %d, %d },\n", 128,
								mouse_x, mouse_y).c_str());

							blinking_star_c* new_star = new blinking_star_c(mouse_x, mouse_y,
								m_backbuffer.get(), m_star_fib);

							m_starmap.emplace_back(new_star);
						}
					}
#endif
				}
			}
		}

		if (m_state_changed)
		{
			m_state_changed = false;

			// clear prev state
			input_manager_c::me()->reset_state();
			input_manager_c::me()->reset_callbacks();

			// setup input manager callbacks & other work
			m_current_app_state->on_enter_state();

			// common input callback
			input_manager_c::me()->append_callback(std::bind(&bomberman_app_c::handle_input,
				this,
				std::placeholders::_1));
		}

		input_manager_c::me()->dispatch();

		frame();

		if (m_need_to_restart)
		{
			m_running = false;
			return e_code_restart;
		}
	}

	return e_code_ok;
}


bool bomberman_app_c::load_config(const std::string& config_path)
{
	return config_c::me()->load_common(config_path);
}


bool bomberman_app_c::initialize()
{
	srand((unsigned int)time(NULL));

	m_application_dir = app_directory();
	m_resource_dir = m_application_dir + "/Media";

	if (!load_config(m_application_dir + "/config.xml"))
	{
		config_c::me()->initialize_defaults();

		config_c::me()->save();
	}

#if 1
	m_viewport_settings.monitor_index = config_c::me()->get_integer("monitor_index");
	m_viewport_settings.screen_width = config_c::me()->get_integer("screen_width");
	m_viewport_settings.screen_height = config_c::me()->get_integer("screen_height");
	m_viewport_settings.window_fullscreen_mode = config_c::me()->get_bool("window_fullscreen_mode");
#else
	m_viewport_settings.monitor_index = 0;
	m_viewport_settings.screen_width = 1920;
	m_viewport_settings.screen_height = 1080;
	m_viewport_settings.window_fullscreen_mode = true;
#endif

	if (initialize_sr_engine(m_viewport_settings.screen_width,
		m_viewport_settings.screen_height) != 0)
	{
		return false;
	}

	if (!initialize_window())
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Error",
			std_string_printf
			(
				"Failed to create window with error: %s",
				1024,
			SDL_GetError()
			).c_str(),
			m_sdl_window);

		return false;
	}

	if (!config_c::me()->load_input_mapping(m_application_dir + "/config.xml"))
	{
		config_c::me()->initialize_mapping_deafults();

		config_c::me()->save();
	}

	initialize_input_manager();

	m_backbuffer.reset(new framebuffer_c);
	m_backbuffer->set_target_window(m_sdl_window);
	m_backbuffer->create();

	if (!initialize_sound_subsystem())
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Error", "Failed to initialize sound subsystem", m_sdl_window);
	}

#if 1
	if (!load_resources())
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Error",
			std_string_printf
			(
				"Failed to load resources",
				1024
			).c_str(),
			m_sdl_window);

		return false;
	}
#endif

	// initialize menu
	{
		if (!m_main_screen.initialize(m_resource_dir, m_viewport_settings, m_sdl_window, m_backbuffer.get()))
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
				"Error",
				"Failed to initialize main screen",
				m_sdl_window);

			return false;
		}

		m_main_screen.set_callbacks
		(
			std::bind(&bomberman_app_c::on_new_game_event, this),
			std::bind(&bomberman_app_c::on_exit_game_event, this),
			std::bind(&bomberman_app_c::on_need_to_restart_event, this)
		);
	}

	linkedlist_initialize(&m_render_buffer_list);
	renderbuffer_initialize(&m_main_render_buffer, 65536);

	if (!initialize_fonts())
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Error", "Failed to load fonts", m_sdl_window);
	}

	initialize_camviews();

	m_zbuffer_ptr.reset(new zbuffer_t());
	set_1zbuffer_ptr(m_zbuffer_ptr.get());
	initialize_zbuffer(m_zbuffer_ptr.get(), m_current_camview->get_camera_ptr(), 32);

	initialize_lighting();

	extern std::list<ash_s*> g_ash_collection;
	initialize_ash_collection(g_ash_collection);

	initialize_game_background();

	return true;
}


void bomberman_app_c::initialize_game_background()
{
	struct star_s
	{
		int x, y;
	};

	// screen points for resolution 1920 x 1080
	std::vector<star_s> star_coords =
	{
		{ 760, 216 },
		{ 770, 184 },
		{ 844, 185 },
		{ 845, 94 },
		{ 1021, 155 },
		{ 187, 692 },
		{ 227, 695 },
		{ 281, 468 },
		{ 430, 300 },
		{ 494, 342 },
		{ 586, 252 },
		{ 756, 254 },
		{ 731, 95 },
		{ 320, 175 },
		{ 418, 137 },
		{ 214, 81 },
		{ 113, 122 },
		{ 86, 455 },
		{ 1095, 294 },
		{ 1260, 169 },
		{ 1421, 66 },
		{ 1537, 346 },
		{ 1479, 354 },
		{ 1480, 397 },
		{ 1802, 573 },
		{ 1778, 567 },
		{ 1676, 333 },
		{ 980, 319 },
		{ 927, 137 },
		{ 189, 341 },
		{ 1409, 430 },
		{ 1784, 706 },
		{ 1733, 702 },
		{ 1563, 643 },
	};

	for (const auto& star_coord : star_coords)
	{
		int star_x = star_coord.x, star_y = star_coord.y;

		if (m_viewport_settings.screen_width != 1920 &&
			m_viewport_settings.screen_height != 1080)
		{
			float x_factor = m_viewport_settings.screen_width / 1920.0f;
			float y_factor = m_viewport_settings.screen_height / 1080.0f;

			star_x = (int)(star_x * x_factor);
			star_y = (int)(star_y * y_factor);
		}

		blinking_star_c* new_star = new blinking_star_c(star_x, star_y,
			m_backbuffer.get(), m_star_fib);

		m_starmap.emplace_back(new_star);
	}
}


bool bomberman_app_c::initialize_window()
{
	m_sdl_window = SDL_CreateWindow
	(
		"3D Bomberman",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		m_viewport_settings.screen_width,
		m_viewport_settings.screen_height,
		SDL_WINDOW_SHOWN// | SDL_WINDOW_UTILITY | SDL_WINDOWPOS_CENTERED
		//SDL_WINDOW_OPENGL
		| (m_viewport_settings.window_fullscreen_mode ? SDL_WINDOW_FULLSCREEN : 0)
	);

	if (m_sdl_window == nullptr)
	{
		return false;
	}

#if 0
	SDL_HideWindow(m_sdl_window);
#endif

	return true;
}


void bomberman_app_c::initialize_lighting()
{
#if 1
	auto ambient_light = add_light_source();
	{
		ambient_light->light_type = e_lt_ambient;
		ambient_light->i_ambient = get_u_rgba(40, 40, 40);
		//ambient_light->i_ambient = get_u_rgba(255, 255, 255);
	}
#endif

#if 1
	auto directional_light = add_light_source();
	{
		directional_light->light_type = e_lt_infinite;
		directional_light->i_diffuse = get_u_rgba(255, 255, 255);

		directional_light->direction_local
			//= glm::normalize(glm::vec4(0.7f, 0.1f, -1.0f, 0));
			= glm::normalize(glm::vec4(0.4f, 1.0f, -0.2f, 0));
	}
#endif
}


bool bomberman_app_c::load_resources()
{
	if (!resource_manager_c::me()->initialize(m_resource_dir, m_viewport_settings))
	{
		return false;
	}

	// load background image
	{
		m_background_fib = FreeImage_Load(FIF_PNG,
			(m_resource_dir + "/serenity_5k-1920x1080.png").c_str());

		if (!m_background_fib)
		{
			return false;
		}

		auto bg_width = FreeImage_GetWidth(m_background_fib);
		auto bg_height = FreeImage_GetHeight(m_background_fib);

		if (bg_width != m_viewport_settings.screen_width ||
			bg_height != m_viewport_settings.screen_height)
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

		set_alpha_channel_value(m_background_fib, 0);

		m_background_data_argb32bits = (uint32_t*)FreeImage_GetBits(m_background_fib);
	}

	m_star_fib = FreeImage_Load(FIF_PNG,
		(m_resource_dir + "/star_v1.png").c_str());

#if 1
	mesh_factory_c::me()->initialize(m_resource_dir);
#endif

	bool num_threads = 1;
	return mesh_factory_c::me()->create_meshes(num_threads);
}


void bomberman_app_c::initialize_camviews()
{
#if 0
	/*
	yaw: -90, pitch: 23, pos: { 0.000008, 453.814209, -612.025452, 1.0f }
	yaw: -90, pitch: 34, pos: { 0.000013, 467.799561, -499.141632, 1.0f }
	yaw: -90, pitch: 43, pos: { 0.000016, 522.992920, -429.623474, 1.0f }
	yaw: -90, pitch: 52, pos: { 0.000019, 589.898499, -353.895538, 1.0f }
	yaw: -90, pitch: 65, pos: { 0.000024, 578.233459, -251.873611, 1.0f }
	yaw: -90, pitch: 75, pos: { 0.000027, 545.306396, -176.841309, 1.0f }
	yaw: -90, pitch: 83, pos: { 0.000031, 575.565735, -90.470680, 1.0f }
	yaw: -90, pitch: 89, pos: { 0.000034, 571.643188, -28.801497, 1.0f }
	yaw: -90, pitch: 82, pos: { 0.000031, 537.896118, -99.216560, 1.0f }
	*/

	struct camview_template_s
	{
		e_camera_type cam_type;
		int fov;
		float yaw, pitch, roll;
		glm::vec4 direction;
		glm::vec4 position;
	};
	std::vector<camview_template_s> camview_templates =
	{
		{ e_camera_type::e_ct_euler, 90, 0.0f, 0.0f, 0.0f, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.000000, 200.000000, -800.000000, 1.0f } },

		{ e_camera_type::e_ct_uvn, 80, -90.0f, 23.0f, 0.0f, {}, { 0.000000, 453.000000, -612.025452, 1.0f } },
		{ e_camera_type::e_ct_uvn, 80, -90.0f, 34.0f, 0.0f, {}, { 0.000000, 467.000000, -499.141632, 1.0f } },
		{ e_camera_type::e_ct_uvn, 80, -90.0f, 43.0f, 0.0f, {}, { 0.000000, 522.000000, -429.623474, 1.0f } },
		{ e_camera_type::e_ct_uvn, 80, -90.0f, 52.0f, 0.0f, {}, { 0.000000, 589.000000, -353.895538, 1.0f } },
		{ e_camera_type::e_ct_uvn, 80, -90.0f, 65.0f, 0.0f, {}, { 0.000000, 578.000000, -251.873611, 1.0f } },
		{ e_camera_type::e_ct_uvn, 80, -90.0f, 75.0f, 0.0f, {}, { 0.000000, 545.000000, -176.841309, 1.0f } },
		{ e_camera_type::e_ct_uvn, 80, -90.0f, 83.0f, 0.0f, {}, { 0.000000, 575.000000, -90.470680, 1.0f } },
		{ e_camera_type::e_ct_uvn, 80, -90.0f, 82.0f, 0.0f, {}, { 0.000000, 537.000000, -99.216560, 1.0f } },
		{ e_camera_type::e_ct_uvn, 80, -90.0f, 89.0f, 0.0f, {}, { 0.000000, 571.000000, 0.000000, 1.0f } },
	};

#elif 1
	camview_ptr_t camview(new cam_view_c(cam_view_c::e_camview_main_view));
	{
		camview->set_type(e_ct_euler);
		camview->set_position(glm::vec4(0, 200, -800, 1));
		camview->set_direction(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		camview->set_speed(0.5f);
		camview->initialize(m_viewport_settings, 90);
		camview->get_camera_ptr()->yaw = glm::radians(15.0f);
		camera_rebuild_matrix(camview->get_camera_ptr());
	}

	m_current_camview = camview.get();

	m_camviews.emplace_back(std::move(camview));
#elif 0
	camview_ptr_t main_camview(new cam_view_c(cam_view_c::e_camview_main_view));
	{
		main_camview->set_type(e_camera_type::e_ct_uvn);
		main_camview->set_position(glm::vec4(0, 200, -800, 1));
		main_camview->set_direction(glm::vec4(0.0f, 0, 1, 1));
		main_camview->set_speed(0.5f);
		main_camview->initialize(m_viewport_settings, 90);
	}

	m_current_camview = main_camview.get();

	m_camviews.emplace_back(std::move(main_camview));
#elif 0
	for (const auto& view : camview_templates)
	{
		camview_ptr_t camview(new cam_view_c(cam_view_c::e_camview_main_view));
		{
			camview->set_type(view.cam_type);
			camview->set_position(view.position);
			camview->set_direction(view.direction);
			camview->set_speed(0.5f);

			camview->initialize(m_viewport_settings, view.fov);

			if (view.cam_type == e_camera_type::e_ct_uvn)
			{
				camera_rotate
				(
					camview->get_camera_ptr(),
					view.yaw,
					view.roll,
					view.pitch
				);
			}
		}
		m_camviews.emplace_back(std::move(camview));
	}

	m_current_camview = m_camviews.front().get();
#endif
}


bool bomberman_app_c::initialize_fonts()
{
	m_font_arial18 = resource_manager_c::me()->get_font
	(
		resource_manager_c::e_font_arial_18
	);

	m_default_service_font = m_font_arial18;

	m_game_change_font_win = resource_manager_c::me()->get_font
	(
		resource_manager_c::e_font_bauhaus_93_win
	);

	m_game_change_font_loose = resource_manager_c::me()->get_font
	(
		resource_manager_c::e_font_bauhaus_93_loose
	);

	m_game_level_label_font = resource_manager_c::me()->get_font
	(
		resource_manager_c::e_font_game_label
	);

	return true;
}


bool bomberman_app_c::initialize_sound_subsystem()
{
	int flags = MIX_INIT_MP3;
	int init_result = Mix_Init(flags);

	if ((init_result & flags) != flags)
	{
		OutputDebugStringA(std_string_printf("Failed to initialize sound library with error: %s", 1024,
			Mix_GetError()).c_str());

		return false;
	}

	// initialize SDL_mixer
	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
	{
		return false;
	}

	// minimize music volume
	Mix_VolumeMusic((int)(MIX_MAX_VOLUME * 0.5f));

	return true;
}


bool bomberman_app_c::initialize_input_manager()
{
	//input_manager_c::me()->set_mode(input_manager_c::input_mode_on_every_frame);
	input_manager_c::me()->open_joysticks();

	return true;
}


void bomberman_app_c::handle_input(input_state_s* state)
{
	if (state->actions.find(e_input_actions::action_show_debug_info)
		!= state->actions.cend())
	{
		m_show_text_info = !m_show_text_info;
	}

	if (state->actions.find(e_input_actions::action_change_view)
		!= state->actions.cend())
	{
#if 0
		m_current_camview_index = (m_current_camview_index + 1) % m_camviews.size();
		m_current_camview = m_camviews[m_current_camview_index].get();
#else
		m_current_camview_index = (m_current_camview_index + 1) % 7;	// 7 levels

		if (m_current_camview_index == 0)
		{
			m_current_camview->set_type(e_camera_type::e_ct_euler);
			m_current_camview->set_position(glm::vec4(0, 200, -800, 1));
			m_current_camview->set_direction(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
			m_current_camview->initialize(m_viewport_settings, 90);
			m_current_camview->get_camera_ptr()->yaw = glm::radians(15.0f);
			camera_rebuild_matrix(m_current_camview->get_camera_ptr());
		}
		else
		{
			m_current_camview->set_type(e_camera_type::e_ct_uvn);
			m_current_camview->initialize(m_viewport_settings, 90);
			camera_rebuild_matrix(m_current_camview->get_camera_ptr());
		}
#endif
	}

#if 0
	if (state->actions.find(e_input_actions::action_quit_game)
		!= state->actions.cend())
	{
		m_running = false;
		return;
	}
#endif

	// switch between game & menu states by ESCAPE
	if (state->actions.find(e_input_actions::action_escape_key_pressed)
		!= state->actions.cend())
	{
		if (m_current_app_state == &m_main_screen)
		{
			if (m_main_screen.is_on_main_page() && m_current_game_level)	// switch to game only if gamelevel created
			{
				set_game_state(e_game_state::e_game_state_game);
			}
		}
		else
		{
			set_game_state(e_game_state::e_game_state_menu);
		}
	}

	if (state->actions.find(e_input_actions::action_take_screenshot)
		!= state->actions.cend())
	{
		static auto cur_dir = app_directory();

		auto screen_fpath = std_string_printf
		(
			"%s/Screenshots/%s_screenshot.png",
			MAX_PATH,
			cur_dir.c_str(),
			get_localtime_string().c_str()
		);

		helpers::take_screenshot(m_sdl_window,
			screen_fpath.c_str(),
			FIF_PNG);
	}
}


void bomberman_app_c::set_game_state(e_game_state new_state)
{
	m_prev_game_state = m_current_game_state;

	m_current_game_state = new_state;

	if (new_state == e_game_state_menu)
	{
		m_current_app_state = &m_main_screen;
		m_main_screen.set_input_mapper();
	}
	else if (new_state == e_game_state_game)
	{
		if (m_current_game_level)
		{
			m_current_app_state = m_current_game_level.get();
			m_current_game_level->set_input_mapper();
		}
	}

	m_state_changed = true;
}


void bomberman_app_c::set_free_fly_mode(bool state)
{
	m_free_fly_mode = state;

	SDL_SetRelativeMouseMode(m_free_fly_mode ? SDL_TRUE : SDL_FALSE);
}


void bomberman_app_c::frame()
{
	static auto next_stat_time_point =
		std::chrono::steady_clock::now() +
		std::chrono::seconds(1);

	static int64_t total_frame_time = 0;
	static int64_t total_frame_count = 0;

	auto frame_begin_timepoint = std::chrono::steady_clock::now();

	if (m_current_game_state == e_game_state_menu)
	{
		frame_menu_state();
	}
	else if (m_current_game_state == e_game_state_game)
	{
		frame_game_state();
	}
	else if (m_current_game_state == e_game_state_changelevel)
	{
		frame_changelevel_state();
	}

	auto frame_end_timepoint = std::chrono::steady_clock::now();

	auto diff = frame_end_timepoint - frame_begin_timepoint;
	auto ms_diff = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
	g_last_frame_duration = ms_diff.count();

	total_frame_time += g_last_frame_duration;
	++total_frame_count;

	if (std::chrono::steady_clock::now() >= next_stat_time_point)
	{
		next_stat_time_point =
			next_stat_time_point +
			std::chrono::seconds(1);

		m_avg_frame_duration_for_last_1_second =
			(int64_t)(total_frame_time / (float)total_frame_count);

		total_frame_time = 0;
		total_frame_count = 0;
	}
}


void camera_build_lookat_matrix(camera_t* cam)
{
	glm::mat4 translate_mat = glm::translate
	(
		glm::vec3
		(
			-cam->position.x,
			-cam->position.y,
			-cam->position.z
		)
	);

	const glm::vec3& forward_vec = glm::normalize(cam->direction - glm::vec3(cam->position));
	const glm::vec3 right_vec(glm::normalize(glm::cross(cam->up, forward_vec)));
	const glm::vec3 up_vec(glm::cross(forward_vec, right_vec));

	// tip: glm::mat -> [col][row]

	cam->rot_mat = glm::mat4(1);
	{
		cam->rot_mat[0][0] = right_vec.x;
		cam->rot_mat[1][0] = right_vec.y;
		cam->rot_mat[2][0] = right_vec.z;
		cam->rot_mat[0][1] = up_vec.x;
		cam->rot_mat[1][1] = up_vec.y;
		cam->rot_mat[2][1] = up_vec.z;
		cam->rot_mat[0][2] = forward_vec.x;
		cam->rot_mat[1][2] = forward_vec.y;
		cam->rot_mat[2][2] = forward_vec.z;
	}

	cam->cam_mat = cam->rot_mat * translate_mat;
}


void bomberman_app_c::frame_game_state()
{
	int mouse_x, mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);

	clear_zbuffer(m_zbuffer_ptr.get());
	m_backbuffer->lock();
#if 0
	m_backbuffer->clear();
#else
	sr::fast_memcpy(m_backbuffer->get_data_ptr(),
		m_background_data_argb32bits,
		m_backbuffer->get_data_size());

	// update star sky
	if (1)
	{
		auto& star = m_starmap[m_current_star_index];

		if (star->blink_finished())
		{
			m_current_star_index = rand() % m_starmap.size();

			m_starmap[m_current_star_index]->start_blink();
		}
		else
		{
			star->update();
		}
	}
	else
	{
		for (auto& star : m_starmap)
		{
			if (star->blink_finished())
			{
				star->start_blink();
			}
			else
			{
				star->update();
			}
		}
	}
#endif

	auto cam_ptr = m_current_camview->get_camera_ptr();

#if 0
	if (m_free_fly_mode)
	{
		camera_rotate(cam_ptr, yaw, 0, pitch);
	}
	else
#endif	
	if (m_current_camview_index == 0)
	{
		camera_rebuild_matrix(cam_ptr);
	}
	else
	{
		// look at the center of map with specifically angle on specifically range

		// ang <=-2 >= -50
		float cam_ang = -(m_current_camview_index - 1) * 10.0f - 2.0f;	// -2.0f - workaround to prevent camera flipping
		//debug_output("ang: %f\n", 64, cam_ang);

		float rotate_radius = 690.0f;	// depends on map height
		float cam_x_pos = 0.0f, cam_y_shift = 0.0f, cam_z_shift = 20.0f;

		cam_ptr->position =
		{
			cam_x_pos,
			rotate_radius * glm::cos(glm::radians(cam_ang)) + cam_y_shift,
			rotate_radius * glm::sin(glm::radians(cam_ang)) + cam_z_shift,
			1.0f
		};

		cam_ptr->direction = glm::vec3(0.0f, 0.0f, 0.0f);	// map center position

#if 1
		camera_build_lookat_matrix(cam_ptr);
#else
		// correct camera rotate, but incorrect lighting because camera rotation matrix is incorrect
		cam_ptr->cam_mat = glm::lookAtLH
		(
			glm::vec3(cam_ptr->position),
			cam_ptr->direction,
			cam_ptr->up
		);
#endif
	}

	linkedlist_initialize(&m_render_buffer_list);
	renderbuffer_reset(&m_main_render_buffer);
	linkedlist_insert_front(&m_render_buffer_list, &m_main_render_buffer);

	m_current_game_level->update();
	m_current_game_level->fill_renderbuffer(m_main_render_buffer);

	if (m_current_game_level->is_finished())
	{
		m_change_level_startpoint = std::chrono::system_clock::now();
		set_game_state(e_game_state_changelevel);
	}

	renderbuffer_cull_backfaces(&m_main_render_buffer, cam_ptr);
	renderbuffer_cull_backfaces_llist(&m_render_buffer_list, cam_ptr);

	renderbuffer_apply_world_to_cam_mat_llist(&m_render_buffer_list, cam_ptr);

	renderbuffer_clip_polygons_llist(&m_render_buffer_list, cam_ptr);

	transform_lights(cam_ptr, e_tm_transform_local_to_trans_list);
	// importantly! lighting only main buffer. all other buffers lighting apart
	renderbuffer_apply_light(&m_main_render_buffer, false);

	renderbuffer_apply_cam_to_perspective_screen_llist(&m_render_buffer_list, cam_ptr);

	// sort render buffer for correct alpha-blending order
	renderbuffer_z_sort(&m_main_render_buffer, sr::e_sort_mode::e_sm_avgz);

	renderbuffer_render_llist(&m_render_buffer_list, m_backbuffer.get());

	show_text_info();

	m_backbuffer->draw();
	m_backbuffer->unlock();
}


void bomberman_app_c::frame_changelevel_state()
{
	if (!m_current_game_level)
	{
		return;
	}

	auto game_level_state = m_current_game_level->get_state();

	clear_zbuffer(m_zbuffer_ptr.get());
	m_backbuffer->lock();
	m_backbuffer->fill_buffer(0);

	std::string msg_text;
	SDL_Color msg_color;

	if (game_level_state == game_level_c::e_level_state_finish)
	{
		msg_text = "LEVEL %d COMPLETED";
		msg_color = { 0, 255, 0 };
	}
	else // player loose
	{
		msg_text = "LEVEL %d FAILED";
		msg_color = { 255, 0, 0 };
	}

	draw_text
	(
		m_backbuffer->get_renderer(),
		m_game_change_font_win,
		msg_color, { 0, 0, 0 },
		(int)(m_viewport_settings.screen_width * 0.5f),
		(int)(m_viewport_settings.screen_height * 0.5f),
		e_text_align::e_text_align_center, sdl_text::e_tbm_blended,
		msg_text.c_str(),
		m_current_game_level_index
	);

	auto now_timepoint = std::chrono::system_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>
	(
		now_timepoint - m_change_level_startpoint
	);
	auto diff_ms = diff.count();

	if (diff_ms > 3000)
	{
		auto modifiers = m_current_game_level->get_game_modifiers();

		create_game_level();

		if (game_level_state == game_level_c::e_level_state_finish)
		{
			m_current_game_level_index++;

			m_current_game_level->set_game_modifiers(modifiers);
		}

		set_game_state(e_game_state_game);
	}

	m_backbuffer->draw();
	m_backbuffer->unlock();
}


void bomberman_app_c::frame_menu_state()
{
	clear_zbuffer(m_zbuffer_ptr.get());
	m_backbuffer->lock();

	m_main_screen.update();

	m_backbuffer->draw();
	m_backbuffer->unlock();
}


bool bomberman_app_c::create_game_level()
{
	m_current_game_level.reset(new game_level_c());
	m_current_game_level->assign_render_buffer_list(&m_render_buffer_list);
	m_current_game_level->assign_cam_view(m_current_camview);

	if (!m_current_game_level->create(13, 11, 64, 64))
	{
		m_current_game_level.reset(nullptr);

		return false;
	}

	return true;
}


void bomberman_app_c::show_text_info()
{
	if (m_show_text_info)
	{
		sdl_text::draw_text_formatted
		(
			m_backbuffer->get_renderer(),
			m_default_service_font,
			{ 255, 0, 0 }, 10, 10, sdl_text::e_tbm_blended,
			"FPS: %d, avg. frame dur.(ms): %lld",
			get_fps(),
			m_avg_frame_duration_for_last_1_second
		);

#if 0
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		sdl_text::draw_text_formatted
		(
			m_backbuffer->get_renderer(),
			m_default_service_font,
			{ 255, 0, 0 }, 10, 30, sdl_text::e_tbm_blended,
			"mouse pos: %d, %d",
			mouse_x, mouse_y
		);
#endif
	}

	// draw level label
	draw_text
	(
		m_backbuffer->get_renderer(),
		m_game_level_label_font,
		{ 0, 0, 255 }, {},
		m_viewport_settings.screen_width - 10, 10,
		e_text_align::e_text_align_right,
		e_text_blend_mode::e_tbm_blended,
		"LEVEL: %d", m_current_game_level_index
	);
}


void bomberman_app_c::on_new_game_event()
{
	if (!m_current_game_level)
		create_game_level();

	set_game_state(e_game_state_game);
}


void bomberman_app_c::on_exit_game_event()
{
	m_running = false;
}


void bomberman_app_c::on_need_to_restart_event()
{
	m_need_to_restart = true;
}
