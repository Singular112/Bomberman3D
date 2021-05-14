#include "stdafx.h"
#include "helpers.h"

bool is_rects_overlapped(const rect_t& rect1, const rect_t& rect2)
{
	bool x_left_check1 = rect1.x_left < (rect2.x_left + rect2.w);
	bool x_right_check1 = (rect1.x_left + rect1.w) > rect2.x_left;

	bool y_top_check1 = (rect1.y_bottom + rect1.h) > rect2.y_bottom;
	bool y_bottom_check1 = rect1.y_bottom < (rect2.y_bottom + rect2.h);

	return (x_left_check1 && x_right_check1) &&
		(y_top_check1 && y_bottom_check1);
}


std::string std_string_printf(char* fmt, size_t buf_sz, ...)
{
	std::string buf;
	buf.resize(buf_sz);

	va_list va;
	va_start(va, buf_sz);
#ifdef WIN32
	auto symb_count = vsprintf_s(&buf[0], buf_sz, fmt, va);
#else
	auto symb_count = vsprintf(&buf[0], fmt, va);
#endif
	va_end(va);

	buf.resize(symb_count);

	return buf;
}


std::wstring std_wstring_printf(wchar_t* fmt, size_t buf_sz, ...)
{
	std::wstring buf;
	buf.resize(buf_sz);

	va_list va;
	va_start(va, buf_sz);
#ifdef WIN32
	auto symb_count = vswprintf_s(&buf[0], buf_sz, fmt, va);
#else
	auto symb_count = vswprintf(&buf[0], fmt, va);
#endif
	va_end(va);

	buf.resize(symb_count);

	return buf;
}


void debug_output(char* fmt, size_t buf_sz, ...)
{
	std::string buf;
	buf.resize(buf_sz);

	va_list va;
	va_start(va, buf_sz);
#ifdef WIN32
	auto symb_count = vsprintf_s(&buf[0], buf_sz, fmt, va);
#else
	auto symb_count = vsprintf(&buf[0], fmt, va);
#endif
	va_end(va);

	buf.resize(symb_count);

	OutputDebugStringA(buf.c_str());
}


std::string get_localtime_string()
{
	auto cur_dt = std::chrono::system_clock::now();
	time_t cur_dt2 = std::chrono::system_clock::to_time_t(cur_dt);
	tm local_tm;
	
#ifdef WIN32
	if (localtime_s(&local_tm, &cur_dt2) != 0)
		return std::string();
#else
	local_tm = *localtime(&cur_dt2);
#endif

	return std_string_printf("%d%0.2d%0.2d_%0.2d%0.2d%0.2d", 128,
		1900 + local_tm.tm_year, local_tm.tm_mon + 1, local_tm.tm_mday,
		local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
}


std::string app_directory()
{
#ifdef WIN32
	char buffer[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	return get_directory_path(buffer);
#else
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	return std::string(result, (count > 0) ? count : 0);
#endif
}


std::string get_directory_path(const std::string& fname)
{
	size_t index = fname.find_last_of("\\/");

	return index == std::string::npos ?
		std::string() :
		fname.substr(0, index);
}


void draw_quad(sr::framebuffer_c& framebuf,
	int x_start, int y_start,
	int width, int height,
	uint32_t color)
{
	auto data = framebuf.get_data_ptr();
	auto buf_h = framebuf.get_buffer_height();
	auto buf_w = framebuf.get_buffer_width();

	for (int y = y_start; y < y_start + height; ++y)
	{
		int line_shift = y * buf_w;

		for (int x = x_start; x < x_start + width; ++x)
		{
			data[line_shift + x] = color;
		}
	}
}


void set_alpha_channel_value(FIBITMAP* fib , uint8_t val)
{
	auto argb32bits = (uint32_t*)FreeImage_GetBits(fib);
	auto width = FreeImage_GetWidth(fib);
	auto height = FreeImage_GetHeight(fib);

	if (argb32bits)
	{
		size_t pixel_count = width * height;
		uint32_t alpha_set_mask = (uint32_t)val << 24;
		for (size_t i = 0; i < pixel_count; ++i)
		{
			auto& color = argb32bits[i];
			color = color & 0x00FFFFFF;
			color = color | alpha_set_mask;
		}
	}
}


SDL_Rect draw_text(SDL_Renderer* renderer,
	TTF_Font* font,
	const SDL_Color& color,
	const SDL_Color& bg_color,
	int pos_x, int pos_y,
	e_text_align text_align,
	sr::helpers::sdl_text::e_text_blend_mode blend_mode,
	const char* fmt, ...)
{
	if (renderer == nullptr || font == nullptr)
		return SDL_Rect();

	char buf[1024];

	va_list va;
	va_start(va, fmt);
	vsprintf_s(buf, fmt, va);
	va_end(va);

	SDL_Surface* msg_surface = nullptr;

	if (blend_mode == sr::helpers::sdl_text::e_tbm_blended)
	{
		msg_surface = TTF_RenderText_Blended(font, buf, color);
	}
	else
	{
		msg_surface = TTF_RenderText_Shaded(font, buf, color, bg_color);
	}

	if (msg_surface == nullptr)
		return SDL_Rect();

	SDL_Texture* msg_texture
		= SDL_CreateTextureFromSurface(renderer, msg_surface);

	if (msg_texture == nullptr)
		return SDL_Rect();

	SDL_Rect rect;
	{
		rect.x = pos_x;
		rect.y = pos_y;
		rect.w = msg_surface->w;
		rect.h = msg_surface->h;
	}

	if (text_align == e_text_align::e_text_align_center)
	{
		rect.x -= (int)(msg_surface->w * 0.5f);
		//rect.y -= (int)(msg_surface->h * 0.5f);
	}
	else if (text_align == e_text_align::e_text_align_right)
	{
		rect.x -= msg_surface->w;
		//rect.y -= (int)(msg_surface->h * 0.5f);
	}

	SDL_RenderCopy(renderer, msg_texture, nullptr, &rect);

	SDL_FreeSurface(msg_surface);
	SDL_DestroyTexture(msg_texture);

	return rect;
}
