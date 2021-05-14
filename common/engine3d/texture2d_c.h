#pragma once

#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#	include <windows.h>
#endif

// stl
#include <string>

// freeimage
#include "FreeImage.h"

namespace sr
{

namespace texture
{

class SOFTRENDERLIB_API_DLL texture2d_c
{
public:
	enum e_mimmap_level
	{
		e_mip_level_1x1 = 0,
		e_mip_level_2x2,
		e_mip_level_4x4,
		e_mip_level_8x8,
		e_mip_level_16x16,
		e_mip_level_32x32,
		e_mip_level_64x64,
		e_mip_level_128x128,
		e_mip_level_256x256,
		e_mip_level_512x512,
		e_mip_level_1024x1024
	};

	struct mimmap_data_s
	{
		FIBITMAP* fib;
		uint32_t* data;
	};

private:
	texture2d_c(texture2d_c&& tex) = delete;
	texture2d_c(const texture2d_c& tex) = delete;
	texture2d_c& operator= (const texture2d_c&) = delete;

public:
	texture2d_c();
	virtual ~texture2d_c();

	bool load(const char* path);
	void release();

	bool is_valid() const;

#ifdef WIN32
	static FIBITMAP* convert_hbitmap_to_fibitmap(HBITMAP hbmp);
	static void draw_fibitmap_in_hwnd(HWND hwnd, FIBITMAP* dib, const RECT* rect = nullptr);
#endif
	static bool save_fibitmap_as_file(FIBITMAP* dib, FREE_IMAGE_FORMAT format,
		const char* file_path, int flags = 0);

	void set_alpha_channel_value(uint8_t val);

	void generate_mip_maps();
	const uint32_t* get_mimmap_data(e_mimmap_level level);
	const uint32_t* get_mimmap_data_by_requested_size(int width, int height);

	FIBITMAP* get_dib_source() const;
	FIBITMAP* get_dib_argb32bits() const;

	const uint8_t* get_data_source() const;
	const uint32_t* get_data_argb32bits() const;

	uint32_t get_width() const;
	uint32_t get_height() const;
	uint32_t get_depth() const;
	uint32_t get_pitch() const;
	uint32_t get_dib_size() const;
	uint32_t get_memory_size() const;
	FREE_IMAGE_COLOR_TYPE get_color_type() const;

	uint32_t get_red_mask() const;
	uint32_t get_green_mask() const;
	uint32_t get_blue_mask() const;

	const BITMAPINFOHEADER* get_info_header() const;
	const BITMAPINFO* get_info() const;

#ifdef FORBID_NON_POW2_TEXTURES
	uint32_t get_log2shift() const;
#endif

private:
	static const int m_mimaps_levels = 11;
	mimmap_data_s m_mipmaps[m_mimaps_levels];	// from 1x1 to 1024x1024
	bool m_have_mipmaps;

#ifdef FORBID_NON_POW2_TEXTURES
	uint32_t m_log2shift = 0;
#endif

	FREE_IMAGE_FORMAT m_format;

	FIBITMAP* m_dib_source;
	FIBITMAP* m_dib_argb32bits;
	uint8_t* m_data_source;
	uint32_t* m_data_argb32bits;

	char* m_filepath;

	uint32_t m_width, m_height, m_depth;
	uint32_t m_pitch;
	uint32_t m_dib_size, m_memory_size;

	BITMAPINFOHEADER* m_info_header;
	BITMAPINFO* m_bitmap_info;

	FREE_IMAGE_COLOR_TYPE m_color_type;
	int32_t m_red_mask, m_green_mask, m_blue_mask;
};

}

}
