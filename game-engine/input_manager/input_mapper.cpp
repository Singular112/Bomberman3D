#include "stdafx.h"
#include "input_mapper.h"

input_mapper_c::input_mapper_c()
{
}


input_mapper_c::~input_mapper_c()
{
}


bool input_mapper_c::create_mapping_from_config()
{
	return true;
}


void input_mapper_c::set_mapping(const std::map<mapping_event_s, e_input_actions, std::less<>>& actions_mapping,
	const std::map<mapping_event_s, e_input_states, std::less<>>& states_mapping,
	const std::map<SDL_Scancode, input_range_s>& ranges_mapping)
{
	m_mapping.actions_mapping = actions_mapping;
	m_mapping.states_mapping = states_mapping;
	m_mapping.ranges_mapping = ranges_mapping;
}


void input_mapper_c::set_mapping(const input_mapping_s& mapping)
{
	m_mapping = mapping;
}


input_mapping_s& input_mapper_c::get_mapping()
{
	return m_mapping;
}


bool input_mapper_c::convert_sdl_mouse_event_to_action(const SDL_Event* event, e_input_actions& action) const
{
	if (event->button.button == 1)
	{
		action = e_input_actions::action_mouse_lclick;
	}
	else if (event->button.button == 2)
	{
		action = e_input_actions::action_mouse_mclick;
	}
	else if (event->button.button == 3)
	{
		action = e_input_actions::action_mouse_rclick;
	}
	else
	{
		return false;
	}

	return true;
}


bool input_mapper_c::convert_mapping_event_to_action(const mapping_event_s& mapping_event, e_input_actions& action) const
{
	auto it = m_mapping.actions_mapping.find(mapping_event);
	if (it != m_mapping.actions_mapping.cend())
	{
		action = it->second;
		return true;
	}

	return false;
}


bool input_mapper_c::convert_sdl_event_to_action(const SDL_Event* event, e_input_actions& action) const
{
	return convert_mapping_event_to_action(mapping_event_s(event), action);
}


bool input_mapper_c::convert_mapping_event_to_state(const mapping_event_s& mapping_event, e_input_states& state) const
{
	auto it = m_mapping.states_mapping.find(mapping_event);
	if (it != m_mapping.states_mapping.cend())
	{
		state = it->second;
		return true;
	}

	return false;
}


bool input_mapper_c::convert_sdl_event_to_state(const SDL_Event* event, e_input_states& state) const
{
	return convert_mapping_event_to_state(mapping_event_s(event), state);
}


bool input_mapper_c::convert_sdl_event_to_range(const SDL_Event* event, input_range_s& range) const
{
	/*auto it = m_mapping.ranges_mapping.find();
	if (it != m_mapping.ranges_mapping.cend())
	{
		range = it->second;
		return true;
	}*/

	return false;
}
