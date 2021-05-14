#include "stdafx.h"
#include "input_manager_c.h"

input_manager_c::input_manager_c()
{
	m_event_occured = false;
	m_input_mode = input_mode_on_event_occured;
	m_current_mapper = nullptr;
}


input_manager_c::~input_manager_c()
{
	close_joysticks();
}


void input_manager_c::set_mode(e_input_mode input_mode)
{
	m_input_mode = input_mode;
}


void input_manager_c::set_input_mapper(const input_mapper_c* mapper)
{
	m_current_mapper = mapper;
}


void input_manager_c::append_callback(const inpmgr_callback_t& func)
{
	m_appended_callbacks.emplace_back(func);
}


void input_manager_c::reset()
{
	reset_state();
	reset_callbacks();

	m_current_mapper = nullptr;
}


void input_manager_c::reset_state()
{
	m_current_state.clear();
}


void input_manager_c::reset_callbacks()
{
	m_appended_callbacks.clear();
}


void input_manager_c::handle_sdl_input(const SDL_Event* event)
{
	if (!m_current_mapper)
		return;

	if (event->type == SDL_KEYDOWN)
	{
		const auto& key = event->key;

		if (key.repeat == 0)
		{
			e_input_actions action;
			if (m_current_mapper->convert_sdl_event_to_action(event, action))
			{
				m_current_state.actions.emplace(action);

				m_event_occured = true;
			}
		}

		{
			e_input_states state;
			if (m_current_mapper->convert_sdl_event_to_state(event, state))
			{
				if (m_current_state.states.find(state) == m_current_state.states.cend())
				{
					m_current_state.states.emplace(state);

					m_event_occured = true;
				}
			}
		}
	}
	else if (event->type == SDL_KEYUP || event->type == SDL_JOYBUTTONUP)
	{
		e_input_states state;
		if (m_current_mapper->convert_sdl_event_to_state(event, state))
		{
			auto it = m_current_state.states.find(state);
			if (it != m_current_state.states.cend())
			{
				m_current_state.states.erase(state);

				m_event_occured = true;
			}
		}
	}
	else if (event->type == SDL_MOUSEBUTTONDOWN)
	{
		e_input_actions action;
		if (m_current_mapper->convert_sdl_mouse_event_to_action(event, action))
		{
			m_current_state.actions.emplace(action);

			m_event_occured = true;
		}
	}
	else if (event->type == SDL_MOUSEMOTION)
	{
		/*
		char buf[1024];
		sprintf_s(buf, sizeof(buf),
			"key.keysym.scancode = %d, key.repeat = %d, key.state = %d\n",
			key.keysym.scancode,
			key.repeat, key.state);
		OutputDebugStringA(buf);
		*/
	}
	else if (event->type == SDL_JOYAXISMOTION)
	{
		auto jaxis_val = event->jaxis.value;

		//debug_output("jaxis_val [%d]: %d\n", 128,
		//	event->jaxis.axis,
		//	jaxis_val);

		mapping_event_s mapping_event(event);

		//auto it = m_joystick_axis_events_history.find(mapping_event);

		auto history_find = [&](const SDL_Event* ev) -> decltype(m_joystick_axis_events_history)::iterator
		{
			for (auto it = m_joystick_axis_events_history.begin(); it != m_joystick_axis_events_history.end(); ++it)
			{
				if (it->event_type == ev->type &&
					//it->joystick_id == ev->jaxis.which &&
					it->joystick_control_id == ev->jaxis.axis)
				{
					return it;
				}
			}

			return m_joystick_axis_events_history.end();
		};

		auto history_it = history_find(event);

		if (history_it == m_joystick_axis_events_history.end())
		{
			bool jaxis_in_allowed_ramge = jaxis_val < -joystick_edgezone_value
				|| jaxis_val > joystick_edgezone_value;

			if (jaxis_in_allowed_ramge)
			{
				// add action
				e_input_actions action;
				if (m_current_mapper->convert_sdl_event_to_action(event, action))
				{
					m_current_state.actions.emplace(action);

					m_joystick_axis_events_history.emplace(mapping_event);

					m_event_occured = true;
				}

				// add state
				e_input_states state;
				if (m_current_mapper->convert_sdl_event_to_state(event, state))
				{
					if (m_current_state.states.find(state) == m_current_state.states.cend())
					{
						//debug_output("add new state (%d) in m_current_state.states\n", 256,
						//	state);

						m_current_state.states.emplace(state);

						m_event_occured = true;
					}
					else
					{
						//debug_output("state (%d) not add in m_current_state.states\n", 256,
						//	state);
					}
				}
			}
		}
		else
		{
			if (jaxis_val >= -joystick_edgezone_value && jaxis_val <= joystick_edgezone_value)
			{
				//debug_output("erase [%d]: %d\n", 128,
				//	event->jaxis.axis,
				//	event->jaxis.value);

				e_input_states state;
				if (m_current_mapper->convert_mapping_event_to_state(*history_it, state))
				{
					//debug_output("found in history [%d]: %d\n", 256,
					//	history_it->joystick_control_id,
					//	history_it->joystick_jvalue);

					auto it = m_current_state.states.find(state);
					if (it != m_current_state.states.cend())
					{
						//debug_output("state %d founded in m_current_state.states\n", 256,
						//	state);

						m_current_state.states.erase(it);

						m_event_occured = true;
					}
					else
					{
						//debug_output("state %d NOT founded in m_current_state.states\n", 256,
						//	state);
					}
				}
				else
				{
					//debug_output("not found in history [%d]: %d\n", 256,
					//	history_it->joystick_control_id,
					//	history_it->joystick_jvalue);
				}

				m_joystick_axis_events_history.erase(history_it);
			}
		}
	}
	else if (event->type == SDL_JOYBUTTONDOWN)
	{
		// add action
		e_input_actions action;
		if (m_current_mapper->convert_sdl_event_to_action(event, action))
		{
			m_current_state.actions.emplace(action);

			m_event_occured = true;
		}

		// add state
		e_input_states state;
		if (m_current_mapper->convert_sdl_event_to_state(event, state))
		{
			if (m_current_state.states.find(state) == m_current_state.states.cend())
			{
				m_current_state.states.emplace(state);

				m_event_occured = true;
			}
		}
	}
#if 0
	else if (event->type == SDL_JOYHATMOTION)
	{
		auto jhat_val = event->jhat.value;

		//debug_output("jaxis_val [%d]: %d\n", 128,
		//	event->jaxis.axis,
		//	jaxis_val);

		mapping_event_s mapping_event(event);

		//auto it = m_joystick_hat_events_history.find(mapping_event);

		auto history_find = [&](const SDL_Event* ev) -> decltype(m_joystick_hat_events_history)::iterator
		{
			for (auto it = m_joystick_hat_events_history.begin(); it != m_joystick_hat_events_history.end(); ++it)
			{
				if (it->event_type == ev->type &&
					it->joystick_id == ev->jaxis.which &&
					it->joystick_control_id == ev->jaxis.axis)
				{
					return it;
				}
			}

			return m_joystick_hat_events_history.end();
		};

		auto history_it = history_find(event);

		if (history_it == m_joystick_hat_events_history.end())
		{
			// add action
			e_input_actions action;
			if (m_current_mapper->convert_sdl_event_to_action(event, action))
			{
				m_current_state.actions.emplace(action);

				m_joystick_hat_events_history.emplace(mapping_event);

				m_event_occured = true;
			}

			// add state
			e_input_states state;
			if (m_current_mapper->convert_sdl_event_to_state(event, state))
			{
				if (m_current_state.states.find(state) == m_current_state.states.cend())
				{
					//debug_output("add new state (%d) in m_current_state.states\n", 256,
					//	state);

					m_current_state.states.emplace(state);

					m_event_occured = true;
				}
				else
				{
					//debug_output("state (%d) not add in m_current_state.states\n", 256,
					//	state);
				}
			}
		}
		else
		{
			//debug_output("erase [%d]: %d\n", 128,
			//	event->jaxis.axis,
			//	event->jaxis.value);

			e_input_states state;
			if (m_current_mapper->convert_mapping_event_to_state(*history_it, state))
			{
				//debug_output("found in history [%d]: %d\n", 256,
				//	history_it->joystick_control_id,
				//	history_it->joystick_jvalue);

				auto it = m_current_state.states.find(state);
				if (it != m_current_state.states.cend())
				{
					//debug_output("state %d founded in m_current_state.states\n", 256,
					//	state);

					m_current_state.states.erase(it);

					m_event_occured = true;
				}
				else
				{
					//debug_output("state %d NOT founded in m_current_state.states\n", 256,
					//	state);
				}
			}
			else
			{
				//debug_output("not found in history [%d]: %d\n", 256,
				//	history_it->joystick_control_id,
				//	history_it->joystick_jvalue);
			}

			m_joystick_hat_events_history.erase(history_it);
		}
	}
#endif
}


void input_manager_c::dispatch()
{
	if (m_input_mode == input_mode_on_every_frame || m_event_occured)
	{
		m_event_occured = false;

		for (auto& callback : m_appended_callbacks)
		{
			callback(&m_current_state);
		}
	}

	m_current_state.clear();
}


bool input_manager_c::open_joysticks()
{
	auto num_j = SDL_NumJoysticks();

	for (auto i = 0; i < num_j; i++)
	{
		auto joystick = SDL_JoystickOpen(i);

		if (joystick)
		{
			auto joystick_id = SDL_JoystickInstanceID(joystick);
			auto joystick_guid = SDL_JoystickGetDeviceGUID(i);

			char guid_buf[33] = { 0 };
			SDL_JoystickGetGUIDString(joystick_guid, guid_buf, (int)sizeof(guid_buf));

			const char* joystick_name = SDL_JoystickName(joystick);
			m_opened_joysticks.emplace(std::make_pair(joystick,
				joystick_info_s
				{
					joystick_id,
					i,
					joystick_name,
					guid_buf
				}));
		}
	}

	return num_j == m_opened_joysticks.size();
}


void input_manager_c::close_joysticks()
{
	for (auto& joystick_it : m_opened_joysticks)
	{
		SDL_JoystickClose(joystick_it.first);
	}

	m_opened_joysticks.clear();
}


std::vector<input_manager_c::joystick_info_s> input_manager_c::enumerate_joysticks()
{
	open_joysticks();

	std::vector<input_manager_c::joystick_info_s> result;

	for (auto& j : m_opened_joysticks)
	{
		result.emplace_back(j.second);
	}

	close_joysticks();

	return result;
}
