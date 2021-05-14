#include "stdafx.h"
#include "game_map2d_c.h"

extern sr::framebuffer_c g_backbuffer;

void game_map2d_c::render()
{
	int y_inverse_coord_shift = (m_cell_y_count - 1) * m_cell_height;

	for (int y = 0; y < m_cell_y_count; ++y)
	{
		int line_shift = y * m_cell_x_count;

		int tile_y_coord = y_inverse_coord_shift
			- (y * m_cell_width);

		for (int x = 0; x < m_cell_x_count; ++x)
		{
			int tile_x_coord = x * m_cell_height;

			const auto& cell = m_cells[line_shift + x];
			const auto cell_type = cell.get_cell_type();

			if (cell_type == e_game_obj_type::e_gt_wall)
			{
				draw_quad(g_backbuffer,
					tile_x_coord, tile_y_coord,
					m_cell_width, m_cell_width,
					m_black_color);
			}
			else if (cell_type == e_game_obj_type::e_gt_free_cell)
			{
				draw_quad(g_backbuffer,
					tile_x_coord, tile_y_coord,
					m_cell_width, m_cell_width,
					m_white_color);
			}
			else if (cell_type == e_game_obj_type::e_gt_box)
			{
				draw_quad(g_backbuffer,
					tile_x_coord, tile_y_coord,
					m_cell_width, m_cell_width,
					m_green_color);
			}

			//draw_text_formatted(tile_x_coord, tile_y_coord, "%d, %d", x, y);
		}
	}
}
