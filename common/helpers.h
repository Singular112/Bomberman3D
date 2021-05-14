#pragma once

//
#include <memory>
#include <string>

//
#include <engine3d/frame_buffer.h>

//
template <class T>
class singleton_c
{
private:
	static std::unique_ptr<T> m_instance;

public:
	static T* me()
	{
		if (m_instance.get() == nullptr)
			m_instance.reset(new T);

		return m_instance.get();
	}

	static void free_instance()
	{
		m_instance.reset(nullptr);
	}
};
template <class T>
std::unique_ptr<T> singleton_c<T>::m_instance;

typedef struct rect_s
{
	int x_left, y_bottom, w, h;
} rect_t;

bool is_rects_overlapped(const rect_t& rect1, const rect_t& rect2);

//
std::string std_string_printf(char* fmt, size_t buf_sz, ...);
std::wstring std_wstring_printf(wchar_t* fmt, size_t buf_sz, ...);

void debug_output(char* fmt, size_t buf_sz, ...);

std::string get_localtime_string();

std::string app_directory();
std::string get_directory_path(const std::string& fname);

void draw_quad(sr::framebuffer_c& framebuf,
	int x_start, int y_start,
	int width, int height,
	uint32_t color);

void set_alpha_channel_value(FIBITMAP* fib, uint8_t val);

//
enum e_text_align
{
	e_text_align_left,
	e_text_align_right,
	e_text_align_center,
};
SDL_Rect draw_text(SDL_Renderer* renderer,
	TTF_Font* font,
	const SDL_Color& color,
	const SDL_Color& bg_color,
	int pos_x, int pos_y,
	e_text_align text_align,
	sr::helpers::sdl_text::e_text_blend_mode blend_mode,
	const char* fmt, ...);
