#pragma once

#include "resource_manager_c.h"

class sound_manager_c
	: public singleton_c<sound_manager_c>
{
public:
	sound_manager_c();
	~sound_manager_c();

	bool play_sound(resource_manager_c::e_sound_kind kind);
	bool play_music(resource_manager_c::e_music_kind kind);

	void stop_music();

	bool is_music_playing();

protected:
	int determine_channel_index(resource_manager_c::e_sound_kind snd);
};
