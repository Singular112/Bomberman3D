#pragma once

#ifdef WIN32
//#	include <vld.h>

#	define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#	include <windows.h>

#	pragma comment(lib, "Gdi32.lib")
#	pragma comment(lib, "FreeImage.lib")
#	pragma comment(lib, "SoftRenderLib.lib")
#else
#	define sprintf_s sprintf
#	define strcpy_s(dest, size_in_bytes, src) strcpy(dest, src)
#	define vswprintf_s(buf_ptr, buf_sz, fmt, va) vswprintf(buf_ptr, fmt, va)
#   define __forceinline __attribute__((always_inline)) inline
#   define __fastcall
#   define __stdcall
#endif

// stl
#include <set>
#include <map>
#include <list>
#include <ctime>
#include <chrono>
#include <vector>
#include <math.h>
#include <time.h>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <assert.h>

#ifndef WIN32
#	include <x86intrin.h>
#endif

// sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

// engine3d
#include <engine3d/setup.h>
#include <engine3d/FreeImage.h>
#include <engine3d/glm_includes.h>
#include <engine3d/engine.h>
#include <engine3d/helpers.h>
using namespace sr;
using namespace sr::helpers;

// helpers
#include "../common/helpers.h"
#include "../common/tinyxml/tinyxml.h"
#include "../common/benchmark.hpp"

// game engine common
#include "../game-engine/resource_manager_c.h"
#include "../game-engine/sound_manager_c.h"
//#include "../game-engine/config_c.h"

#ifdef WIN32
#	pragma comment(lib, "GLu32.lib")
#	pragma comment(lib, "OpenGL32.lib")

#	pragma comment(lib, "SDL/Win32/SDL2.lib")
#	pragma comment(lib, "SDL/Win32/SDL2_ttf.lib")
#	pragma comment(lib, "SDL/Win32/SDL2_mixer.lib")
#endif

// shared variables
extern int64_t g_last_frame_duration;
