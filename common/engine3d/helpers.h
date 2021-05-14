#pragma once

#ifdef USE_SDL
#	include <SDL2\SDL_ttf.h>
#endif


namespace sr
{

namespace helpers
{

namespace sdl_text
{

enum e_text_blend_mode
{
	e_tbm_solid,
	e_tbm_blended
};

SOFTRENDERLIB_API void draw_text_formatted(SDL_Renderer* renderer,
	TTF_Font* font,
	const SDL_Color& color,
	int pos_x, int pos_y,
	e_text_blend_mode blend_mode,
	char* fmt, ...);

}

SOFTRENDERLIB_API bool take_screenshot(SDL_Window* wnd,
	const char* dest_path,
	FREE_IMAGE_FORMAT format);

}

}
