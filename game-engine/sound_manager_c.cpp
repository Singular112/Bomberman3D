#include "stdafx.h"
#include "sound_manager_c.h"

#include "config_c.h"

sound_manager_c::sound_manager_c()
{
}


sound_manager_c::~sound_manager_c()
{
}


int sound_manager_c::determine_channel_index(resource_manager_c::e_sound_kind snd)
{
	switch (snd)
	{
	case resource_manager_c::e_sound_explosion:
		return 1;
	case resource_manager_c::e_sound_pickup_modifier:
		return 2;
	}

	return -1; // first free channel
}


bool sound_manager_c::play_sound(resource_manager_c::e_sound_kind kind)
{
	if (!config_c::me()->get_bool("sounds_enabled", true))
	{
		return true;
	}

	auto chunk = resource_manager_c::me()->get_sound(kind);

	auto channel = determine_channel_index(kind);

	if (!chunk || Mix_PlayChannel(channel, chunk, 0) == -1)
	{
		return false;
	}

	return true;
}


bool sound_manager_c::play_music(resource_manager_c::e_music_kind kind)
{
	if (!config_c::me()->get_bool("music_enabled", true))
	{
		return true;
	}

	return Mix_PlayMusic(resource_manager_c::me()->get_music(kind), -1) != -1;
}


void sound_manager_c::stop_music()
{
	Mix_HaltMusic();
}


bool sound_manager_c::is_music_playing()
{
	return Mix_PlayingMusic() != 0;
}
