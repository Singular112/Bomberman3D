#pragma once

// stl
#include <map>
#include <set>
#include <vector>
#include <atomic>
#include <functional>

// sdl
#include <SDL2/SDL.h>

//
#include "input_mapper.h"

struct input_state_s
{
	std::set<e_input_actions> actions;
	std::set<e_input_states> states;
	std::set<input_range_s> ranges;

	void clear()
	{
		actions.clear();
		ranges.clear();
	}
};


class input_manager_c
	: public singleton_c<input_manager_c>
{
public:
	typedef std::function<void(input_state_s*)> inpmgr_callback_t;

	enum e_input_mode
	{
		input_mode_on_event_occured,
		input_mode_on_every_frame
	};

	struct joystick_info_s
	{
		int id;
		int device_index;

		std::string name;
		std::string guid_str;
	};

public:
	input_manager_c();
	virtual ~input_manager_c();

	void set_mode(e_input_mode input_mode);

	void set_input_mapper(const input_mapper_c* mapper);

	void append_callback(const inpmgr_callback_t& func);

	void reset();

	void reset_state();

	void reset_callbacks();

	void handle_sdl_input(const SDL_Event* event);

	void dispatch();

	std::vector<joystick_info_s> enumerate_joysticks();

	bool open_joysticks();
	void close_joysticks();

private:
	std::atomic_bool m_event_occured;

	std::vector<inpmgr_callback_t> m_appended_callbacks;

	const input_mapper_c* m_current_mapper;
	input_state_s m_current_state;

	std::set<mapping_event_s, std::less<>> m_joystick_axis_events_history;
	std::set<mapping_event_s, std::less<>> m_joystick_hat_events_history;

	std::map<SDL_Joystick*, joystick_info_s> m_opened_joysticks;

	e_input_mode m_input_mode;
};
