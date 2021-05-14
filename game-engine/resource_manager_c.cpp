#include "stdafx.h"
#include "resource_manager_c.h"


resource_manager_c::resource_manager_c()
{
	m_font_info[e_font_arial_18] = { "/fonts/arial.ttf", 18 };
	m_font_info[e_font_bauhaus_93_28] = { "/fonts/Bauhaus 93 Regular.ttf", 0 };

	m_font_info[e_font_bauhaus_93_win] = { "/fonts/TERMINAT.TTF", 40 };
	m_font_info[e_font_bauhaus_93_loose] = { "/fonts/TERMINAT.TTF", 40 };

	m_font_info[e_font_game_label] = { "/fonts/TERMINAT.TTF", 24 };

#if 0
	m_font_info[e_font_forte_28] = { "/fonts/FORTE.ttf", 0 };
	m_font_info[e_font_ink_free_28] = { "/fonts/ink-free-normal.ttf", 0 };
	m_font_info[e_font_jokerman_28] = { "/fonts/Jokerman.ttf", 0 };
	m_font_info[e_font_kristen_itc_28] = { "/fonts/ITCKRIST.ttf", 0 };
	m_font_info[e_font_rockwell_28] = { "/fonts/Rockwell_MT_Bold.ttf", 0 };
	m_font_info[e_font_snap_itc_28] = { "/fonts/snap itc.ttf", 0 };
	m_font_info[e_font_wide_latin_28] = { "/fonts/Wide Latin Regular.ttf", 0 };
	m_font_info[e_font_algerian_28] = { "/fonts/Algerian Regular.ttf", 0 };
	m_font_info[e_font_buxton_sketch_28] = { "/fonts/buxton-sketch.ttf", 0 };
	m_font_info[e_font_segoe_marker_28] = { "/fonts/segoe-marker.ttf", 0 };
#endif

	m_texture_info[e_texture_explosion] = "/textures/exp2_0.png";
	m_texture_info[e_texture_wall] = "/textures/wall.png";
	m_texture_info[e_texture_box] = "/textures/box.png";
	m_texture_info[e_texture_floor] = "/textures/floor.png";
	m_texture_info[e_texture_modifier_explosion_power] = "/textures/fire.png";
	m_texture_info[e_texture_modifier_bomb_amount] = "/textures/bomb_obj.png";

	m_texture_info[e_texture_exit_top] = "/textures/exit_top.png";
	m_texture_info[e_texture_exit_bottom_enabled] = "/textures/exit_bottom_enabled.png";
	m_texture_info[e_texture_exit_bottom_disabled] = "/textures/exit_bottom_disabled.png";

	m_music_info[e_music_mainmenu] = "/audio/Crystal Room.wav";
	m_music_info[e_music_gamelevel] = "/audio/02 - Mr.B Bee (ORIGINAL KARAOKE).mp3";

	m_sound_info[e_sound_explosion] = "/audio/explodemini.wav";

	m_sound_info[e_sound_pickup_modifier] = "/audio/SUCCESS PICKUP Collect Chime 01.wav";
	m_sound_info[e_sound_mob_attack] = "/audio/hit28.wav";
	m_sound_info[e_sound_win] = "/audio/track 1.mp3";
	m_sound_info[e_sound_loose] = "/audio/track 2.mp3";

#if 0
	m_sound_info[e_sound_plant_bomb] = "/audio/.wav";
#endif

	m_sound_info[e_sound_menuitem_mouseover] = "/audio/misc_menu.wav";
	m_sound_info[e_sound_menuitem_click] = "/audio/misc_menu_3.wav";
}


resource_manager_c::~resource_manager_c()
{
}


bool resource_manager_c::initialize(const std::string& resource_dir,
	const viewport_settings_s& viewport_settings)
{
	m_resource_dir = resource_dir;

	int minimal_height = 600;
	int maximum_height = 1152;

	int minimum_font_height = 28;
	int maximum_font_height = 36;

	// 28 - 1920 x 1080
	// 

	viewport_settings.screen_height;

	for (const auto& info : m_font_info)
	{
		auto font_size = info.second.size;

		if (font_size == 0)
		{
			float t = (viewport_settings.screen_height - minimal_height)
				/ (float)(maximum_height - minimal_height); // [0.0f, 1.0f] 

			font_size = (decltype(font_size))(minimum_font_height + (maximum_font_height - minimum_font_height) * t);
		}

		ttf_font_ptr_t font(TTF_OpenFont((m_resource_dir + info.second.path).c_str(), font_size));
		if (!font)
		{
			OutputDebugStringA(std_string_printf("Failed to load resource '%s'", 1024,
				info.second.path.c_str()).c_str());

			return false;
		}
		m_font_library.emplace(std::make_pair(info.first, std::move(font)));
	}

	for (const auto& info : m_texture_info)
	{
		texture2d_ptr_t texture(new texture2d_c);
		if (!texture->load((m_resource_dir + info.second).c_str()))
		{
			OutputDebugStringA(std_string_printf("Failed to load resource '%s'", 1024,
				info.second.c_str()).c_str());

			return false;
		}
		texture->set_alpha_channel_value(0);	// set alpha byte of all pixels to 0
		m_texture_library.emplace(std::make_pair(info.first, std::move(texture)));
	}

	for (const auto& info : m_sound_info)
	{
		std::string resource_path = m_resource_dir + info.second;

		sound_ptr_t sound(Mix_LoadWAV(resource_path.c_str()));
		if (!sound)
		{
			OutputDebugStringA(std_string_printf("Failed to load resource '%s' with error: %s", 1024,
				info.second.c_str(),
				Mix_GetError()).c_str());

			return false;
		}
		m_sound_library.emplace(std::make_pair(info.first, std::move(sound)));
	}

	for (const auto& info : m_music_info)
	{
		std::string resource_path = m_resource_dir + info.second;

		music_ptr_t music(Mix_LoadMUS(resource_path.c_str()));
		if (!music)
		{
			OutputDebugStringA(std_string_printf("Failed to load resource '%s' with error: %s", 1024,
				info.second.c_str(),
				Mix_GetError()).c_str());

			return false;
		}
		m_music_library.emplace(std::make_pair(info.first, std::move(music)));
	}

	return true;
}


TTF_Font* resource_manager_c::get_font(e_font_kind kind)
{
	auto it = m_font_library.find(kind);

	if (it != m_font_library.cend())
	{
		return it->second.get();
	}

	return nullptr;
}


texture2d_c* resource_manager_c::get_texture(e_texture_kind kind)
{
	auto it = m_texture_library.find(kind);

	if (it != m_texture_library.cend())
	{
		return it->second.get();
	}

	return nullptr;
}


Mix_Music* resource_manager_c::get_music(e_music_kind kind)
{
	auto it = m_music_library.find(kind);

	if (it != m_music_library.cend())
	{
		return it->second.get();
	}

	return nullptr;
}


Mix_Chunk* resource_manager_c::get_sound(e_sound_kind kind)
{
	auto it = m_sound_library.find(kind);

	if (it != m_sound_library.cend())
	{
		return it->second.get();
	}

	return nullptr;
}
