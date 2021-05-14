#pragma once

#include "game_object_c.h"

class deadline_object3d_c
	: public game_object_c
{
protected:
	enum e_deadline_object_state
	{
		e_dos_stopped,
		e_dos_continues,
		e_dos_finished
	};

public:
	deadline_object3d_c(int timeout_ms);

	void update(bool update_normals = true, bool update_bounding_box = false);

protected:
	e_deadline_object_state m_current_dedline_object_state;

	int64_t m_deadline_duration = 0;
	std::chrono::time_point<std::chrono::system_clock> m_timeout_timepoint;
};
