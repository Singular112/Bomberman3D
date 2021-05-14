#pragma once

#include "cam_view_c.h"


class resource_manager_c
	: public singleton_c<resource_manager_c>
{
	struct sound_closer_s { void operator()(Mix_Chunk* p) { if (p) Mix_FreeChunk(p); p = nullptr; } };
	struct music_closer_s { void operator()(Mix_Music* p) { if (p) Mix_FreeMusic(p); p = nullptr; } };
	struct font_closer_s { void operator()(TTF_Font* p) { if (p) TTF_CloseFont(p); p = nullptr; } };

	struct font_info_s
	{
		std::string path;

		int size;
	};

public:
	typedef int32_t resource_id_t;
	typedef std::unique_ptr<texture2d_c> texture2d_ptr_t;

	typedef std::unique_ptr<Mix_Music, music_closer_s> music_ptr_t;
	typedef std::unique_ptr<Mix_Chunk, sound_closer_s> sound_ptr_t;

	typedef std::unique_ptr<TTF_Font, font_closer_s> ttf_font_ptr_t;

	enum e_font_kind
	{
		e_font_arial_18,
		e_font_forte_28,
		e_font_ink_free_28,
		e_font_jokerman_28,
		e_font_kristen_itc_28,
		e_font_rockwell_28,
		e_font_snap_itc_28,
		e_font_wide_latin_28,
		e_font_algerian_28,
		e_font_buxton_sketch_28,
		e_font_segoe_marker_28,
		e_font_bauhaus_93_28,
		e_font_bauhaus_93_win,
		e_font_bauhaus_93_loose,
		e_font_game_label
	};

	enum e_resource_type
	{
		e_rt_texture,
		e_rt_music,
		e_rt_sound
	};

	enum e_texture_kind
	{
		e_texture_no_texture,

		e_texture_wall,
		e_texture_box,
		e_texture_exit_top_enabled,
		e_texture_exit_top,
		e_texture_exit_bottom_enabled,
		e_texture_exit_bottom_disabled,
		e_texture_floor,
		e_texture_skybox,
		e_texture_explosion,
		e_texture_modifier_explosion_power,
		e_texture_modifier_bomb_amount,
		e_texture_modifier_character_speed
	};

	enum e_music_kind
	{
		e_music_mainmenu,
		e_music_gamelevel
	};

	enum e_sound_kind
	{
		e_sound_plant_bomb,
		e_sound_pickup_modifier,
		e_sound_explosion,
		e_sound_mob_attack,
		e_sound_win,
		e_sound_loose,

		e_sound_menuitem_mouseover,
		e_sound_menuitem_click
	};

public:
	resource_manager_c();
	~resource_manager_c();

	bool initialize(const std::string& resource_dir,
		const viewport_settings_s& viewport_settings);

	TTF_Font* get_font(e_font_kind kind);
	texture2d_c* get_texture(e_texture_kind kind);
	Mix_Music* get_music(e_music_kind kind);
	Mix_Chunk* get_sound(e_sound_kind kind);

private:
	std::string m_resource_dir;

	std::map<e_font_kind, font_info_s> m_font_info;
	std::map<e_font_kind, ttf_font_ptr_t> m_font_library;

	std::map<e_texture_kind, std::string> m_texture_info;
	std::map<e_texture_kind, texture2d_ptr_t> m_texture_library;

	std::map<e_music_kind, std::string> m_music_info;
	std::map<e_music_kind, music_ptr_t> m_music_library;

	std::map<e_sound_kind, std::string> m_sound_info;
	std::map<e_sound_kind, sound_ptr_t> m_sound_library;
};
