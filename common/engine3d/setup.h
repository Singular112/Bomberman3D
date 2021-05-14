#pragma once

#define USE_GLM_MATH

#ifndef WIN32
#   define BAN_SIMD
#	define USE_SDL
#else
#	define USE_SDL
#	define SR_USE_FAST_MEMSET	// ahtung: it's slower on some computers
#endif

//#define FRAME_BUFFER_USE_TEXTURE	// slow method on windows. check linux performance

#define USE_LIGHT_INTENSITY_TABLE

#define MAX_LIGHT_SOURCES						8

#define ALPHA_NUM_LEVELS						(10)
#define ALPHA_NUM_COLORS						(256 * 256 * 256)

#define STATIC_RENDERLIST_POLYGONS_LIMIT		(65536)

#define USE_FAST_POINT_LIGHTS

//#define RENDER_WIREFRAME_CLIPPED_POLYGONS

// textures setup
//#define FORBID_NON_SQUARE_TEXTURES
//#define FORBID_NON_POW2_TEXTURES

// GLM setup
#ifdef USE_GLM_MATH
#	include "glm_includes.h"
#endif
