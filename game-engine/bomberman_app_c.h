#pragma once

#include "cam_view_c.h"
#include "game_level_c.h"
#include "main_screen_c.h"
#include "blinking_star_c.h"

class bomberman_app_c
{
public:
	enum e_game_state
	{
		e_game_state_menu,
		e_game_state_game,
		e_game_state_changelevel
	};

	enum e_return_codes : int32_t
	{
		e_code_ok = 0,
		e_code_initialize_failed = -1,
		e_code_restart = 5
	};

private:
	typedef std::unique_ptr<cam_view_c> camview_ptr_t;
	typedef std::unique_ptr<blinking_star_c> blinking_star_ptr_t;

public:
	bomberman_app_c();

	~bomberman_app_c();

	int exec();

private:
	bool load_config(const std::string& config_path);

	bool initialize();

	void initialize_game_background();

	bool initialize_window();

	void initialize_lighting();

	bool load_resources();

	void initialize_camviews();

	bool initialize_fonts();

	bool initialize_sound_subsystem();

	bool initialize_input_manager();

	void handle_input(input_state_s* state);

	void set_game_state(e_game_state new_state);

	void set_free_fly_mode(bool state);

	void frame();

	void frame_game_state();

	void frame_changelevel_state();

	void frame_menu_state();

	bool create_game_level();

	void show_text_info();

	// callbacks from menu
	void on_new_game_event();
	void on_exit_game_event();
	void on_need_to_restart_event();

private:
	volatile bool m_running = false;
	bool m_need_to_restart = false;

	bool m_state_changed = true;
	app_state_c* m_current_app_state = nullptr;

	SDL_Thread* m_app_thread = nullptr;

	//
	main_screen_c m_main_screen;

	// backbuffer background
	FIBITMAP* m_background_fib = nullptr;
	uint32_t* m_background_data_argb32bits = nullptr;
	FIBITMAP* m_star_fib;
	std::vector<blinking_star_ptr_t> m_starmap;
	std::vector<int> m_blinking_stars_indices;
	int m_current_star_index = 0;

	//
	e_game_state m_current_game_state, m_prev_game_state;

	//
	viewport_settings_s m_viewport_settings;
	SDL_Window* m_sdl_window = nullptr;
	std::unique_ptr<framebuffer_c> m_backbuffer;

	//
	std::string m_application_dir,
		m_resource_dir;

	//
	TTF_Font* m_font_arial18 = nullptr;
	TTF_Font* m_default_service_font = nullptr;
	TTF_Font* m_game_change_font_win = nullptr;
	TTF_Font* m_game_change_font_loose = nullptr;
	TTF_Font* m_game_level_label_font = nullptr;

	// states
	bool m_show_text_info = false;

	//
	linked_list_s<render_buffer_t> m_render_buffer_list;
	render_buffer_t m_main_render_buffer;

	//
	std::vector<std::unique_ptr<input_mapper_c>> m_input_mappers;

	//
	std::unique_ptr<zbuffer_t> m_zbuffer_ptr;

	std::vector<camview_ptr_t> m_camviews;
	cam_view_c* m_current_camview = nullptr;
	int m_current_camview_index = 0;
	bool m_free_fly_mode = false;

	//
	std::unique_ptr<game_level_c> m_current_game_level;

	std::chrono::time_point<std::chrono::system_clock> m_change_level_startpoint;

	int m_current_game_level_index = 1;

	//
	int64_t m_avg_frame_duration_for_last_1_second = 0;
};
