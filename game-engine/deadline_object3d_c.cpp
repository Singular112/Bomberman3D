#include "stdafx.h"
#include "deadline_object3d_c.h"

deadline_object3d_c::deadline_object3d_c(int timeout_ms)
	: game_object_c(false)
{
	m_current_dedline_object_state = e_dos_stopped;

	m_timeout_timepoint =
		std::chrono::system_clock::now() +
		std::chrono::milliseconds(timeout_ms);
}


void deadline_object3d_c::update(bool update_normals, bool update_bounding_box)
{
	game_object_c::update(update_normals, update_bounding_box);

	if (std::chrono::system_clock::now() > m_timeout_timepoint)
	{
		// set state destroyed
		m_current_dedline_object_state = e_dos_finished;
	}
}
