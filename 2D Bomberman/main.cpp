//
#include "stdafx.h"

// stl
#include <time.h>
#include <stdint.h>
#include <vector>
#include <assert.h>

// game engine
#include "../game-engine/game_map_c.h"
#include "../game-engine/character_base_c.h"
#include "../game-engine/character_ai_c.h"
#include "../game-engine/character2d_c.h"
#include "../game-engine/bomberman2d_c.h"
#include "../game-engine/enemy2d_c.h"
#include "../game-engine/input_manager/input_manager_c.h"

#include "../common/helpers.h"

int64_t g_last_frame_duration = 0;
int64_t g_avg_frame_duration_for_last_1_second = 0;

input_manager_c g_input_manager;

SDL_Window* g_sdl_window = nullptr;
framebuffer_c g_backbuffer;

render_list_t g_render_list;

int g_monitor_index = 0;
#if 1
int g_screen_width = 1280;
int g_screen_height = 768;
bool g_window_fullscreen_mode = false;
#else
int g_screen_width = 1920;
int g_screen_height = 1080;
bool g_window_fullscreen_mode = false;
#endif

TTF_Font* g_font_arial = nullptr;
TTF_Font* g_default_service_font = nullptr;

game_map2d_c g_game_map;

enemy2d_c* enemy2d = nullptr;

bomberman2d_c* bomberman2d = nullptr;


bool initialize_engine3d()
{
	if (initialize_sr_engine(g_screen_width, g_screen_height) != 0)
	{
		return false;
	}

	// initialize window
	{
		g_sdl_window = SDL_CreateWindow
		(
			"3D Bomberman",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			g_screen_width,
			g_screen_height,
			SDL_WINDOW_SHOWN// | SDL_WINDOW_UTILITY | SDL_WINDOWPOS_CENTERED
			//SDL_WINDOW_OPENGL
			| (g_window_fullscreen_mode ? SDL_WINDOW_FULLSCREEN : 0)
		);

		if (g_sdl_window == nullptr)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
				"Error",
				std_string_printf
				(
					"Failed to create window with error: %s",
					1024,
					SDL_GetError()
				).c_str(),
				g_sdl_window);
			return 1;
		}

#if 0
		SDL_HideWindow(g_sdl_window);
#endif
	}

	g_backbuffer.set_target_window(g_sdl_window);
	g_backbuffer.create();

	auto cur_dir = app_directory();
	std::string arial_font_path = cur_dir + "/Media/fonts/arial.ttf";
	g_font_arial = TTF_OpenFont(arial_font_path.c_str(), 18);
	if (!g_font_arial)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"Error", "Failed to load arial.ttf", g_sdl_window);

		return false;
	}

	g_default_service_font = g_font_arial;

	return true;
}


bool initialize_input_manager()
{
	g_input_manager.set_mode(input_manager_c::input_mode_on_event_occured);

	auto j = g_input_manager.enumerate_joysticks();

	//load_input_mapping_from_config();

	input_mapper_c* mapper = new input_mapper_c();
	{
		std::map<int32_t, e_input_actions> actions_mapping;
		std::map<int32_t, e_input_states> states_mapping;
		std::map<int32_t, input_range_s> ranges_mapping;

		states_mapping[4] = states_mapping[80] = e_input_states::state_run_left;
		states_mapping[7] = states_mapping[79] = e_input_states::state_run_right;
		states_mapping[26] = states_mapping[82] = e_input_states::state_run_forward;
		states_mapping[22] = states_mapping[81] = e_input_states::state_run_backward;

		actions_mapping[4] = actions_mapping[80] = e_input_actions::action_turn_left;
		actions_mapping[7] = actions_mapping[79] = e_input_actions::action_turn_right;
		actions_mapping[26] = actions_mapping[82] = e_input_actions::action_turn_up;
		actions_mapping[22] = actions_mapping[81] = e_input_actions::action_turn_down;
		actions_mapping[SDL_SCANCODE_SPACE] = e_input_actions::action_plane_bomb;

		mapper->set_mapping(actions_mapping, states_mapping, ranges_mapping);
	}
	g_input_manager.set_input_mapper(mapper);

	return true;
}


void initialize()
{
	srand((unsigned int)time(NULL));

	initialize_input_manager();

	g_game_map.create(13, 11, 64, 64);

	enemy2d = new enemy2d_c();
	{
		enemy2d->create();
		enemy2d->set_game_map_ptr(&g_game_map);
		enemy2d->set_cell_position(10, 4);
	}

	bomberman2d = new bomberman2d_c();
	{
		bomberman2d->create();
		bomberman2d->set_game_map_ptr(&g_game_map);
		bomberman2d->set_cell_position(2, 0);
	}

	g_input_manager.append_callback(std::bind(&character_base_c<game_map2d_c>::handle_input,
		bomberman2d,
		std::placeholders::_1));
}


FIBITMAP* starsky_fib;
FIBITMAP* star_fib;
bool initialize_background()
{
	auto fib_original = FreeImage_Load(FIF_JPEG,
		"D:\\Projects\\Games\\3D-Soft Bomberman\\Bin\\Media\\serenity_5k-1920x1080.jpg");

	starsky_fib = FreeImage_Rescale(fib_original, g_screen_width, g_screen_height);

	star_fib = FreeImage_Load(FIF_PNG,
		"D:\\Projects\\Games\\3D-Soft Bomberman\\Bin\\Media\\star_v1.png");

	return true;
}


void frame()
{
	static auto next_stat_time_point =
		std::chrono::steady_clock::now() +
		std::chrono::seconds(1);

	static int64_t total_frame_time = 0;
	static int64_t total_frame_count = 0;

	auto frame_begin_timepoint = std::chrono::steady_clock::now();

	//Sleep(10);

	g_backbuffer.clear();

	g_game_map.render();

	bomberman2d->update();
	bomberman2d->render();

	//enemy2d->update();
	//enemy2d->render();

	//
	int xMouse, yMouse;
	SDL_GetMouseState(&xMouse, &yMouse);

	sdl_text::draw_text_formatted(g_backbuffer.get_renderer(),
		g_default_service_font,
		{ 255, 0, 0 }, 10, 10, sdl_text::e_tbm_blended,
		"FPS: %d, avg. frame dur.(ms): %lld, xMouse: %d, yMouser: %d",
		get_fps(),
		g_avg_frame_duration_for_last_1_second,
		xMouse, yMouse);

	//
	g_backbuffer.draw();

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

		g_avg_frame_duration_for_last_1_second =
			(int64_t)(total_frame_time / (float)total_frame_count);

		total_frame_time = 0;
		total_frame_count = 0;
	}
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
	initialize_engine3d();

	initialize();

	initialize_background();

	while (true)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			//OutputDebugStringA(std_string_printf("event.type: %d\n", 100, event.type).c_str());

			if (event.type == SDL_QUIT)
			{
				return 0;
			}
			else if (event.type == SDL_MOUSEMOTION
				|| event.type == SDL_KEYDOWN
				|| event.type == SDL_KEYUP)
			{
				g_input_manager.handle_sdl_input(&event);
			}
		}

		g_input_manager.dispatch();
		frame();
	}

	return 0;
}
