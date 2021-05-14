#include "stdafx.h"
#include "state_context_c.h"


void state_context_c::set_state(state_base_c* new_state)
{
	if (m_current_state)
	{
		m_current_state->on_exit_state();
	}

	m_current_state = new_state;

	m_current_state->on_enter_state();
}

void state_context_c::update_state()
{
	if (!m_current_state)
		return;

	m_current_state->update_state(this);
}
