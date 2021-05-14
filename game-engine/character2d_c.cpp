#include "stdafx.h"
#include "character2d_c.h"

extern sr::framebuffer_c g_backbuffer;

void character2d_c::render()
{
	auto data = g_backbuffer.get_data_ptr();
	auto buf_h = g_backbuffer.get_buffer_height();
	auto buf_w = g_backbuffer.get_buffer_width();

	auto sprite_fibitmap = m_sprites[m_angle].get_fibitmap();

	auto img_data = FreeImage_GetBits(sprite_fibitmap);
	auto bmpinfo = FreeImage_GetInfo(sprite_fibitmap);

	if (!sprite_fibitmap)
	{
		for (int y = m_y_bottom_pos; y < m_y_bottom_pos + m_width; ++y)
		{
			int line_shift = y * buf_w;

			for (int x = m_x_left_pos; x < (int)m_x_left_pos + m_width; ++x)
			{
				data[line_shift + x] = m_color;
			}
		}

		return;
	}

	int x_shift = (int)((m_width - bmpinfo->bmiHeader.biWidth) / 2.0 + 0.5f);
	int y_shift = (int)((m_width - bmpinfo->bmiHeader.biHeight) / 2.0 + 0.5f);

	int y_inverse_shift = (m_game_map_ptr->get_map_y_cells_count() - 1) *
		m_game_map_ptr->get_cell_width();

	auto x_pos = m_x_left_pos + x_shift;
	auto y_pos = y_inverse_shift - m_y_bottom_pos + y_shift;

	if (x_pos < 0)
		x_pos = 0;
	if (y_pos < 0)
		y_pos = 0;

	g_backbuffer.copy_datablock
	(
		x_pos, y_pos,
		0, 0, bmpinfo->bmiHeader.biWidth, bmpinfo->bmiHeader.biHeight,
		img_data, bmpinfo
	);
}
