#pragma once

#include <vector>

#include "map_cell.h"

template <typename CellT = map_cell_c>
class game_map_c
{
public:
	game_map_c();

	virtual ~game_map_c();

	virtual void create(int x_cell_count, int y_cell_count,
		int cell_width, int cell_height);

	virtual void update() = 0;

	virtual void render() = 0;

	// get

	int get_cell_index(int cell_x_index, int cell_y_index) const;

	virtual CellT* operator[](int cell_index);

	virtual const CellT* operator[](int cell_index) const;

	virtual CellT* get_cell(int cell_index);

	virtual const CellT* get_cell(int cell_index) const;

	virtual CellT* get_cell(int x_index, int y_index);

	virtual const CellT* get_cell(int x_index, int y_index) const;

	virtual bool is_cell_available(int cell_index) const;

	virtual bool is_cell_free(int cell_index) const;

	virtual bool is_cell_available(int cell_x_index, int cell_y_index) const;

	virtual bool is_cell_free(int cell_x_index, int cell_y_index) const;

	virtual bool is_cell_between(int cell_x_index, int cell_y_index,
		e_game_obj_type cell_type) const;

	int get_map_x_cells_count() const;
	int get_map_y_cells_count() const;

	int get_cell_width() const;
	int get_cell_height() const;

	int get_box_cell_count() const;

	virtual glm::vec3 translate_local_pos_to_map_global_pos(const glm::vec3& local_pos,
		bool center) const { return glm::vec3(); };

	// set

	void pop_object_from_cell(CellT* cell);

protected:
	int m_cell_x_count, m_cell_y_count;

	int m_cell_width, m_cell_height;

	std::vector<CellT> m_cells;

	int m_box_cells_count;
};
