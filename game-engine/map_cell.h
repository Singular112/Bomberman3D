#pragma once

#include "game_object_c.h"
#include "game_types.h"

struct cell_index_s
{
	int x, y;
};


class map_cell_c
{
public:
	map_cell_c();
	map_cell_c(int cell_size_px);
	virtual ~map_cell_c();

	// set

	virtual void set_cell_size(int size_px);

	virtual void set_cell_type(e_game_obj_type type);

	virtual void set_cell_index(int index);

	virtual void set_cell_x_index(int index);

	virtual void set_cell_y_index(int index);

	virtual void set_cell_indexes(int cell_x_index, int cell_y_index);

	// get

	virtual bool is_free() const;

	virtual bool is_available() const;

	virtual bool is_wall() const;

	virtual int get_cell_size() const;

	virtual e_game_obj_type get_cell_type() const;

	virtual int get_cell_index() const;

	virtual int get_cell_x_index() const;

	virtual int get_cell_y_index() const;

protected:
	int m_cell_size;
	int m_cell_index;
	int m_cell_x_index, m_cell_y_index;

	e_game_obj_type m_cell_type;
};


class map_cell3d_c
	: public map_cell_c
{
public:
	~map_cell3d_c();

	// set

	void append_game_object(game_object_c* obj);

	void pop_object();

	void clear_all_game_objects();

	void set_local_position(const glm::vec4& pos);

	void set_global_position(const glm::vec4& pos);

	// get

	game_object_c* get_current_game_object();

	const glm::vec4& get_local_position() const;

	const glm::vec4& get_global_position() const;

private:
	glm::vec4 m_pos_local, m_pos_global;
	std::list<game_object_c*> m_appended_objects;
};
