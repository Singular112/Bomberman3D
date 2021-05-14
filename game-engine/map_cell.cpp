#include "stdafx.h"
#include "map_cell.h"

map_cell_c::map_cell_c()
	: m_cell_type(e_gt_free_cell)
{
}


map_cell_c::map_cell_c(int cell_size_px)
	: m_cell_size(cell_size_px)
	, m_cell_type(e_gt_free_cell)
{
}


map_cell_c::~map_cell_c()
{
}


// set

void map_cell_c::set_cell_size(int size_px)
{
	m_cell_size = size_px;
}


void map_cell_c::set_cell_type(e_game_obj_type type)
{
	m_cell_type = type;
}


void map_cell_c::set_cell_index(int index)
{
	m_cell_index = index;
}


void map_cell_c::set_cell_x_index(int index)
{
	m_cell_x_index = index;
}


void map_cell_c::set_cell_y_index(int index)
{
	m_cell_y_index = index;
}


void map_cell_c::set_cell_indexes(int cell_x_index, int cell_y_index)
{
	set_cell_x_index(cell_x_index);
	set_cell_y_index(cell_y_index);
}

// get

bool map_cell_c::is_free() const
{
	return m_cell_type == e_gt_free_cell;
}


bool map_cell_c::is_available() const
{
	return m_cell_type == e_game_obj_type::e_gt_free_cell
		|| m_cell_type == e_game_obj_type::e_gt_modifyer
		|| m_cell_type == e_game_obj_type::e_gt_exit;
}


bool map_cell_c::is_wall() const
{
	return m_cell_type == e_gt_wall;
}


int map_cell_c::get_cell_size() const
{
	return m_cell_size;
}


e_game_obj_type map_cell_c::get_cell_type() const
{
	return m_cell_type;
}


int map_cell_c::get_cell_index() const
{
	return m_cell_index;
}


int map_cell_c::get_cell_x_index() const
{
	return m_cell_x_index;
}


int map_cell_c::get_cell_y_index() const
{
	return m_cell_y_index;
}

// ==================================================

// set

map_cell3d_c::~map_cell3d_c()
{
	clear_all_game_objects();
}


void map_cell3d_c::append_game_object(game_object_c* obj)
{
	if (m_appended_objects.size() == 0)
	{
		set_cell_type(obj->get_game_type());
	}

	m_appended_objects.emplace_back(obj);
}


void map_cell3d_c::pop_object()
{
	if (m_appended_objects.size() == 0)
		return;

	delete m_appended_objects.front();
	m_appended_objects.pop_front();

	if (m_appended_objects.size() == 0)
	{
		set_cell_type(e_game_obj_type::e_gt_free_cell);
	}
	else
	{
		set_cell_type(m_appended_objects.front()->get_game_type());
	}
}


void map_cell3d_c::clear_all_game_objects()
{
	auto it = m_appended_objects.begin();

	while (it != m_appended_objects.end())
	{
		delete *it;
		it = m_appended_objects.erase(it);
	}
}


void map_cell3d_c::set_local_position(const glm::vec4& pos)
{
	m_pos_local = pos;
}


void map_cell3d_c::set_global_position(const glm::vec4& pos)
{
	m_pos_global = pos;
}

// get

game_object_c* map_cell3d_c::get_current_game_object()
{
	return m_appended_objects.front();
}


const glm::vec4& map_cell3d_c::get_local_position() const
{
	return m_pos_local;
}


const glm::vec4& map_cell3d_c::get_global_position() const
{
	return m_pos_global;
}
