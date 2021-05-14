#pragma once

#include "sr_lib.h"

// stl
#include <stdint.h>

// sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_ttf.h>

namespace sr
{

class SOFTRENDERLIB_API_DLL framebuffer_c
{
public:
	typedef uint32_t videobuffer_data_t;

public:
	framebuffer_c();
	virtual ~framebuffer_c();

#ifdef USE_SDL
	void set_target_window(SDL_Window* target_window);
#else
	void set_target_window(HWND target_window);
#endif

	bool create();

	videobuffer_data_t* get_data_ptr() const;
	size_t get_data_size() const;

	void clear();
	void fill_buffer(videobuffer_data_t color = 0x00B8B8B8);

	void lock();
	void unlock();

	void draw();

#ifdef WIN32
	void copy_datablock(int dest_x, int dest_y,
		int src_x, int src_y,
		int src_width, int src_height,
		const void* lpBits, const BITMAPINFO* lpbmi);
#else

#endif

	int get_buffer_width() const;
	int get_buffer_height() const;

	SDL_Renderer* get_renderer() const;

#ifndef USE_SDL
	const BITMAPINFO* get_bitmapinfo() const;
#endif

private:
	int m_width, m_height;

	videobuffer_data_t* m_data = nullptr;
#if defined(USE_SDL)
	SDL_Window* m_target_window = nullptr;

	SDL_Renderer* m_renderer = nullptr;

	SDL_Texture* m_buffer_texture = nullptr;
	SDL_Surface* m_window_surface = nullptr;
#else
	BITMAPINFO m_bmp_nfo;

	HWND m_target_window = 0;
#endif

	size_t m_data_size;
	size_t m_data_count;
};

}
