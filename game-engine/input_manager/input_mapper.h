#pragma once

static const int joystick_edgezone_value = 20000;

//

enum e_input_action_type
{
	iat_action,
	iat_state,
	iat_range
};

enum e_input_actions
{
	action_none,

	action_mouse_lclick,
	action_mouse_mclick,
	action_mouse_rclick,

	action_plane_bomb,
	action_change_view,
	action_switch_music_on_off,

	action_turn_left,
	action_turn_right,
	action_turn_up,
	action_turn_down,

	action_left_key_pressed,
	action_right_key_pressed,
	action_up_key_pressed,
	action_down_key_pressed,
	action_escape_key_pressed,
	action_enter_key_pressed,

	action_show_fps,
	action_show_debug_info,

	action_quit_game,
	action_take_screenshot
};

enum e_input_states
{
	state_none,

	state_run_left,
	state_run_right,
	state_run_forward,
	state_run_backward
};

struct input_range_s
{
};

// mapping_event_s is a wrapper for raw SDL_Event
struct mapping_event_s
{
	decltype(SDL_Event::type) event_type = 0;

	// keyboard
	SDL_Scancode scancode;

	// joystick
	SDL_JoystickID joystick_id;
	uint8_t joystick_control_id;
	decltype(SDL_JoyAxisEvent::value) joystick_jvalue;
	uint8_t jbutton_id;

	// api
	mapping_event_s()
	{
	}

	explicit mapping_event_s(const SDL_Scancode scncode)
	{
		event_type = SDL_KEYDOWN;
		scancode = scncode;
	}

	explicit mapping_event_s(uint32_t event_type, const SDL_Scancode scncode)
	{
		event_type = event_type;
		scancode = scncode;
	}

	explicit mapping_event_s(const SDL_Event* event)
	{
		if (event->type == SDL_KEYDOWN)
		{
			event_type = event->type;
			scancode = event->key.keysym.scancode;
		}
		else if (event->type == SDL_KEYUP)
		{
			event_type = SDL_KEYDOWN;
			scancode = event->key.keysym.scancode;
		}
		else if (event->type == SDL_JOYAXISMOTION)
		{
			event_type = event->type;
			joystick_id = 0;// event->jaxis.which;
			joystick_control_id = event->jaxis.axis;
			joystick_jvalue = event->jaxis.value;
		}
		else if (event->type == SDL_JOYHATMOTION)
		{
			event_type = event->type;
			joystick_id = 0;// event->jhat.which;
			joystick_control_id = event->jhat.hat;
			joystick_jvalue = event->jhat.value;
		}
		else if (event->type == SDL_JOYBUTTONDOWN)
		{
			event_type = event->type;
			joystick_id = 0;// event->jbutton.which;
			jbutton_id = event->jbutton.button;
		}
		else if (event->type == SDL_JOYBUTTONUP)
		{
			event_type = SDL_JOYBUTTONDOWN;
			joystick_id = 0;// event->jbutton.which;
			jbutton_id = event->jbutton.button;
		}
	}

	explicit mapping_event_s(uint32_t joystick_event,
		SDL_JoystickID joystck_id,
		uint8_t cntrl_id,
		int16_t jvalue)
	{
		event_type = joystick_event;
		joystick_id = 0;// joystck_id;
		joystick_control_id = cntrl_id;
		joystick_jvalue = jvalue;
	}

	explicit mapping_event_s(uint32_t joystick_event,
		SDL_JoystickID joystck_id,
		uint8_t button_id)
	{
		event_type = joystick_event;
		joystick_id = 0;// joystck_id;
		jbutton_id = button_id;
	}

#if 0
	bool operator==(SDL_Event* event)
	{
		if (event_type == SDL_KEYDOWN)
		{
			return event_type == event->type &&
				event->key.keysym.scancode == scancode;
		}
	}
#endif

	bool operator<(mapping_event_s const& other) const
	{
		if (event_type < other.event_type)
		{
			return true;
		}
		else if (event_type == other.event_type)
		{
			if (event_type == SDL_KEYDOWN/* || event_type == SDL_KEYUP*/)
			{
				if (scancode < other.scancode)
				{
					return true;
				}
				else if (scancode == other.scancode)
				{
					return false;
				}
			}
			else if (event_type == SDL_JOYHATMOTION)
			{
				if (joystick_id < other.joystick_id)
				{
					return true;
				}
				else if (joystick_id == other.joystick_id)
				{
					if (joystick_control_id < other.joystick_control_id)
					{
						return true;
					}
					else if (joystick_control_id == other.joystick_control_id)
					{
						if (joystick_jvalue < other.joystick_jvalue)
						{
							return true;
						}
						else if (joystick_jvalue == other.joystick_jvalue)
						{
							return false;
						}
					}
				}
			}
			else if (event_type == SDL_JOYBUTTONDOWN)
			{
				if (joystick_id < other.joystick_id)
				{
					return true;
				}
				else if (joystick_id == other.joystick_id)
				{
					if (jbutton_id < other.jbutton_id)
					{
						return true;
					}
					else if (jbutton_id == other.jbutton_id)
					{
						return false;
					}
				}
			}
			else if (event_type == SDL_JOYAXISMOTION)
			{
				if (joystick_id < other.joystick_id)
				{
					return true;
				}
				else if (joystick_id == other.joystick_id)
				{
					if (joystick_control_id < other.joystick_control_id)
					{
						return true;
					}
					else if (joystick_control_id == other.joystick_control_id)
					{
						if (joystick_jvalue == 0)
						{
							return true;
						}

						auto j1_val = joystick_jvalue > 0 ? 1 : -1;
						auto j2_val = other.joystick_jvalue > 0 ? 1 : -1;

						if (j1_val < j2_val)
						{
							return true;
						}
						else if (j1_val == j2_val)
						{
							return false;
						}
					}
				}
			}
		}

		return false;
	}
};

struct input_mapping_s
{
	std::map<mapping_event_s, e_input_actions, std::less<>> actions_mapping;
	std::map<mapping_event_s, e_input_states, std::less<>> states_mapping;
	std::map<SDL_Scancode, input_range_s> ranges_mapping;
};

class input_mapper_c
{
public:
	input_mapper_c();
	virtual ~input_mapper_c();

	bool create_mapping_from_config();

	void set_mapping(const input_mapping_s& mapping);

	void set_mapping(const std::map<mapping_event_s, e_input_actions, std::less<>>& actions_mapping,
		const std::map<mapping_event_s, e_input_states, std::less<>>& states_mapping,
		const std::map<SDL_Scancode, input_range_s>& ranges_mapping);

	input_mapping_s& get_mapping();

	virtual bool convert_sdl_mouse_event_to_action(const SDL_Event* event, e_input_actions& action) const;

	bool convert_mapping_event_to_action(const mapping_event_s& mapping_event, e_input_actions& action) const;
	virtual bool convert_sdl_event_to_action(const SDL_Event* event, e_input_actions& action) const;

	virtual bool convert_mapping_event_to_state(const mapping_event_s& mapping_event, e_input_states& state) const;
	virtual bool convert_sdl_event_to_state(const SDL_Event* event, e_input_states& state) const;

	virtual bool convert_sdl_event_to_range(const SDL_Event* event, input_range_s& range) const;

protected:
	input_mapping_s m_mapping;
};
