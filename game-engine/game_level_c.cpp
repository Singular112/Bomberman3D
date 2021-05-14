#include "stdafx.h"
#include "game_level_c.h"
#include "modifier_object3d_c.h"
#include "object_exit3d_c.h"
#include "config_c.h"

using namespace sr;

game_level_c::game_level_c()
{
	m_current_mapper.reset(new input_mapper_c);
}


game_level_c::~game_level_c()
{
#if 0
	Mix_HaltMusic();
#endif
}


void game_level_c::on_enter_state()
{
	input_manager_c::me()->append_callback(std::bind(&character3d_c::handle_input,
		m_bomberman_character.get(),
		std::placeholders::_1));

	SDL_ShowCursor(SDL_DISABLE);
}


void game_level_c::assign_render_buffer_list(linked_list_s<render_buffer_t>* render_buffer_list)
{
	m_render_buffer_list_ptr = render_buffer_list;
}


void game_level_c::assign_cam_view(cam_view_c* cam_view)
{
	m_current_cam_view = cam_view;
}


void game_level_c::set_game_modifiers(const std::vector<e_game_modifier>& modifiers)
{
	m_game_modifiers = modifiers;

	// pass modifiers to character
	m_bomberman_character->set_modifiers(m_game_modifiers);
}


std::vector<e_game_modifier> game_level_c::get_game_modifiers()
{
	return m_game_modifiers;
}


void game_level_c::set_input_mapper()
{
	config_c::me()->get_mapping(*m_current_mapper.get());

	auto& mapping = m_current_mapper->get_mapping();
	mapping.actions_mapping[mapping_event_s(SDL_SCANCODE_ESCAPE)] = action_escape_key_pressed;

	input_manager_c::me()->set_input_mapper(m_current_mapper.get());
}


bool game_level_c::create(int map_x_cell_count, int map_y_cell_count,
	int map_cell_width, int map_cell_height)
{
	m_current_level_state = e_level_state_running;

	m_exit_cell = nullptr;

	m_map_x_cell_count = map_x_cell_count;
	m_map_y_cell_count = map_y_cell_count;
	m_map_cell_width = map_cell_width;
	m_map_cell_height = map_cell_height;

	m_game_map.reset(new game_map3d_c);

	m_game_map->set_absolute_position(glm::vec4(0, 0, 0, 0), game_map3d_c::e_mpm_by_center);

	m_game_map->create(m_map_x_cell_count, m_map_y_cell_count,
		m_map_cell_width, m_map_cell_height);

	m_game_map->add_on_boom_callback(std::bind(&game_level_c::on_bomb_explosion,
		this,
		std::placeholders::_1));

#if 0
	auto current_cell = m_game_map->get_cell(6, 1);

	game_object_c* object = nullptr;

	object = new object_bomb3d_c();

	assert(object != nullptr);

	auto global_pos = m_game_map->translate_local_pos_to_map_global_pos
	(
		current_cell->get_local_position(),
		true
	);

	object->set_pos(global_pos);

	if (object->is_animated())
	{
		object->start_animation();
	}

	current_cell->append_game_object(object);
#endif

#if 0
	auto current_cell = m_game_map->get_cell(12, 5);

	m_game_map->place_game_object(current_cell, e_game_obj_type::e_gt_exit);
	m_exit_cell = current_cell;
#endif

#if 0
	auto current_cell = m_game_map->get_cell(6, 1);
	auto current_cell2 = m_game_map->get_cell(5, 0);

	game_object_c* object = nullptr;
	game_object_c* object2 = nullptr;

	object = new modifier_object3d_c(e_modifier_bomb_amount);
	object2 = new modifier_object3d_c(e_modifier_explosion_power);

	assert(object != nullptr);

	auto global_pos = m_game_map->translate_local_pos_to_map_global_pos
	(
		current_cell->get_local_position(),
		true
	);

	auto global_pos2 = m_game_map->translate_local_pos_to_map_global_pos
	(
		current_cell2->get_local_position(),
		true
	);

	object->set_pos(global_pos);

	object2->set_pos(global_pos2);

	current_cell->append_game_object(object);

	current_cell2->append_game_object(object2);

	m_modifiers_cell_list.emplace_back(current_cell);
#endif

#if 1
	place_bomberman_on_map();
#endif

#if 1
	place_enemies_on_map();
#endif

#if 1
	place_objects_on_map();
#endif

#if 1
	if (!sound_manager_c::me()->is_music_playing())
	{
		sound_manager_c::me()->play_music(resource_manager_c::e_music_gamelevel);
	}
#endif

	return true;
}


void game_level_c::place_bomberman_on_map()
{
	m_bomberman_character.reset(new bomberman3d_c());

	m_bomberman_character->create();

	m_bomberman_character->set_game_map_ptr(m_game_map.get());

	m_bomberman_character->set_cell_position(6, 0);

	m_game_map->add_on_boom_callback(std::bind(&bomberman3d_c::on_bomb_explosion,
		m_bomberman_character.get(),
		std::placeholders::_1));
}


enemy3d_c* game_level_c::spawn_enemy(e_enemy_type enemy_type,
	int map_x_cell, int map_y_cell)
{
	enemy3d_c* enemy = nullptr;

	if (enemy_type == e_enemy_type::e_enemy_flower)
		enemy = new flower_enemy3d_c();
	else if (enemy_type == e_enemy_type::e_enemy_devil)
		enemy = new devil_enemy3d_c();
	else
		return nullptr;

	enemy->create();

	enemy->set_game_map_ptr(m_game_map.get());
	enemy->set_game_level_ptr(this);

	enemy->set_cell_position(map_x_cell, map_y_cell);

	enemy->set_state(e_cs_findway);

	m_enemy_list.emplace_back(enemy);

	return enemy;
}


void game_level_c::place_enemies_on_map()
{
	int bomberman_cell_x_index = m_bomberman_character->get_cell_x_index();
	int bomberman_cell_y_index = m_bomberman_character->get_cell_y_index();

	// cells what already busy by mobs
	std::vector<cell_index_s> busy_cells;

	int rand_val = 0;
	for (int i = 0; i < 4; i++)
	{
		int rand_x = 0, rand_y = 0;

		// don't place enemy near of bomberman
		// place enemies only on free cells
		// roll random position
		while (true)
		{
			rand_x = rand() % m_game_map->get_map_x_cells_count();
			rand_y = rand() % m_game_map->get_map_y_cells_count();

			int x_len = abs(rand_x - bomberman_cell_x_index);
			int y_len = abs(rand_y - bomberman_cell_y_index);

			int len = (int)sqrt(x_len * x_len + y_len * y_len);

			bool cell_busy = false;
			for (const auto& cell_indices : busy_cells)
			{
				if (cell_indices.x == rand_x && cell_indices.y == rand_y)
				{
					cell_busy = true;
					break;
				}
			}

			auto cell = m_game_map->get_cell(rand_x, rand_y);

			if (len < 4 ||	// don't place enemies near the character
				cell_busy ||
				cell->get_cell_type() != e_gt_free_cell)
			{
				continue;
			}

			busy_cells.push_back({ rand_x, rand_y });

			break;
		}

		e_enemy_type enemy_type = (e_enemy_type)(rand_val % e_enemies_count);

		spawn_enemy(enemy_type, rand_x, rand_y);

		rand_val++;
	}
}


void game_level_c::place_objects_on_map()
{
	// place exit, bonuses on map
	//

	int bomberman_cell_x_index = m_bomberman_character->get_cell_x_index();
	int bomberman_cell_y_index = m_bomberman_character->get_cell_y_index();

	// cells what already busy by another objects
	std::vector<cell_index_s> busy_cells;

	std::list<e_game_obj_type> gameobjs_set =
	{
		e_gt_exit,
		e_gt_modifyer
	};

	while (gameobjs_set.size() > 0)
	{
		int rand_x = rand() % m_game_map->get_map_x_cells_count();
		int rand_y = rand() % m_game_map->get_map_y_cells_count();

		int x_len = abs(rand_x - bomberman_cell_x_index);
		int y_len = abs(rand_y - bomberman_cell_y_index);

		int len = (int)sqrt(x_len * x_len + y_len * y_len);

		bool cell_busy = false;
		for (const auto& cell_indices : busy_cells)
		{
			if (cell_indices.x == rand_x && cell_indices.y == rand_y)
			{
				cell_busy = true;
				break;
			}
		}

		auto cell = m_game_map->get_cell(rand_x, rand_y);

		if (len < 4 ||	// don't place objects near the character
			cell_busy ||
			cell->get_cell_type() != e_gt_box)
		{
			continue;
		}

		auto obj_type = gameobjs_set.front();
		gameobjs_set.pop_front();

		if (obj_type == e_gt_modifyer)
		{
			auto modifier_type = (e_game_modifier)(rand() % e_modifier_unknown);

			m_game_map->place_game_object(cell,
				e_game_obj_type::e_gt_modifyer,
				modifier_type);
		}
		else if (obj_type == e_gt_exit)
		{
			m_game_map->place_game_object(cell, e_game_obj_type::e_gt_exit);
		}

		busy_cells.push_back({ rand_x, rand_y });
	}
}


void game_level_c::update()
{
	m_game_map->update();

	auto cam_ptr = m_current_cam_view->get_camera_ptr();

	// update explosions
	{
		auto it = m_explosion_objects.begin();

		while (it != m_explosion_objects.end())
		{
			auto& explosion = *it;

			explosion->update();

			if (explosion->is_finished())
			{
				delete *it;
				it = m_explosion_objects.erase(it);
				continue;
			}

			if (explosion->get_power() < 0.8f)
			{
				// burn characters only on high power
				++it;
				continue;
			}

#if 1
			// check is explosion collide with bomberman
			if (m_bomberman_character->is_collide_with(explosion))
			{
				m_bomberman_character->kill(character3d_c::e_kill_by_fire);

				m_current_level_state = e_level_state_finish_by_bomb;

				m_bomberman_character->disable_input(true);
			}
#endif

			// check is explosion collide with enemies
			for (auto& enemy : m_enemy_list)
			{
				if (!enemy->is_dead() &&
					enemy->is_collide_with(explosion))
				{
					enemy->kill(character3d_c::e_kill_by_fire);
				}
			}

			++it;
		}
	}

	// to correct lighting of clipped objects we need to follow the rules:
	// 1. update object to compute matrices
	// 2. fill render buffer with actual data
	// 3. IMPORTANT. light the object
	// 4. do another work (including clipping)

#if 1
	if (m_bomberman_character)
	{
		m_bomberman_character->update();
		m_bomberman_character->fill_renderbuffer();
		renderbuffer_apply_light(m_bomberman_character->get_render_buffer(), true);

		// clip object by Y and fire ashfall
		m_bomberman_character->update_ashfalls(cam_ptr);

		linkedlist_insert_front(m_render_buffer_list_ptr,
			m_bomberman_character->get_render_buffer());

		if (m_current_level_state == e_level_state_running)
		{
			// check collision with modifiers & pick up it
			auto modifier_cell_it = m_modifiers_cell_list.begin();
			while (modifier_cell_it != m_modifiers_cell_list.cend())
			{
				if (m_bomberman_character->is_collide_with(*modifier_cell_it,
					e_game_obj_type::e_gt_modifyer,
					0.5f, 0.2f))
				{
					sound_manager_c::me()->play_sound(resource_manager_c::e_sound_pickup_modifier);

					auto collision_cell = *modifier_cell_it;

					auto modifier_obj = (modifier_object3d_c*)collision_cell->get_current_game_object();

					if (modifier_obj)
					{
						auto modifier_type = modifier_obj->get_modifier_type();

						//
						m_bomberman_character->add_modifier(modifier_type);

						// save modifier in common modifier-list
						m_game_modifiers.emplace_back(modifier_type);

						// play sound
						sound_manager_c::me()->play_sound(resource_manager_c::e_sound_pickup_modifier);
					}

					m_game_map->pop_object_from_cell(collision_cell);

					m_modifiers_cell_list.erase(modifier_cell_it);
					break;
				}

				modifier_cell_it++;
			}

			// check collision with exit
			{
				auto enemies_count = m_enemy_list.size();
				if (m_exit_cell &&
					enemies_count == 0 &&
					m_bomberman_character->is_collide_with(m_exit_cell, e_gt_exit, 0.1f, 0.1f))
				{
					// successfully level finish

					m_current_level_state = e_level_state_finish;

					m_exit_cell->pop_object();

					m_bomberman_character->disable_input(true);
					m_bomberman_character->set_direction(e_way_direction::e_wd_backward);
					m_bomberman_character->enter_win_state();

					// play sound
					Mix_HaltMusic();
					sound_manager_c::me()->play_sound(resource_manager_c::e_sound_win);
				}
			}
		}
	}

	auto enemy_it = m_enemy_list.begin();
	while (enemy_it != m_enemy_list.end())
	{
		auto& enemy = *enemy_it;

		if (enemy->is_death_completed())
		{
			enemy_it = m_enemy_list.erase(enemy_it);

			if (m_enemy_list.size() == 0)
			{
				enable_finish_on_map();
			}

			continue;
		}

		enemy->update();
		enemy->fill_renderbuffer();
		renderbuffer_apply_light(enemy->get_render_buffer(), true);

		enemy->update_ashfalls(cam_ptr);

		linkedlist_insert_front(m_render_buffer_list_ptr,
			enemy->get_render_buffer());

		if (m_current_level_state == e_level_state_running)
		{
			if (!enemy->is_dead() &&
				m_bomberman_character->is_collide_with(enemy.get()))
			{
				m_finish_state_enemy_ptr = enemy.get();

				//
				m_current_level_state = e_level_state_finish_by_mob;
				m_current_mobfinish_state = e_mobfinish_substate_start;

				m_finish_state_enemy_ptr->set_state(e_cs_idle);
				m_bomberman_character->disable_input(true);

				auto new_enemy_direction =
					m_finish_state_enemy_ptr->get_way_direction(m_finish_state_enemy_ptr->get_current_cell(),
					m_bomberman_character->get_current_cell());

				auto new_player_direction =
					m_finish_state_enemy_ptr->get_way_direction(m_bomberman_character->get_current_cell(),
					m_finish_state_enemy_ptr->get_current_cell());

				m_finish_state_enemy_ptr->set_direction(new_enemy_direction);
				m_bomberman_character->set_direction(new_player_direction);

				m_current_mobfinish_state = e_mobfinish_substate_turn_to_player;

				return;
			}
		}

		//
		++enemy_it;
	}
#endif

	if (m_current_level_state != e_level_state::e_level_state_running)
	{
		// stop bombs detonation on finish state
		m_disable_bombs_on_map = true;
	}

	if (m_current_level_state == e_level_state_finish)
	{
		if (m_bomberman_character->is_win_completed())
		{
			m_is_finished = true;
		}
	}

	else if (m_current_level_state == e_level_state_finish_by_mob)
	{
		if (m_current_mobfinish_state == e_mobfinish_substate_turn_to_player)
		{
			if (!m_finish_state_enemy_ptr->is_rotation_completed() ||
				!m_bomberman_character->is_rotation_completed())
			{
				return;
			}
			else
			{
				m_current_mobfinish_state = e_mobfinish_substate_mob_attack;

				m_finish_state_enemy_ptr->attack();

				// play punch sound
				sound_manager_c::me()->play_sound(resource_manager_c::e_sound_mob_attack);

				return;
			}
		}
		else if (m_current_mobfinish_state == e_mobfinish_substate_mob_attack)
		{
			if (m_finish_state_enemy_ptr->is_attack_completed())
			{
				m_current_mobfinish_state = e_mobfinish_substate_player_death;

				m_finish_state_enemy_ptr->on_stop_movement();
				m_finish_state_enemy_ptr->set_state(e_cs_idle);

				m_bomberman_character->kill(character3d_c::e_kill_by_enemy);
			}
		}
		else if (m_current_mobfinish_state == e_mobfinish_substate_player_death)
		{
			if (m_bomberman_character->is_death_completed())
			{
				m_is_finished = true;

				// play sound
				Mix_HaltMusic();
				sound_manager_c::me()->play_sound(resource_manager_c::e_sound_loose);
			}
		}
	}
	else if (m_current_level_state == e_level_state_finish_by_bomb)
	{
		if (m_bomberman_character->is_death_completed())
		{
			m_is_finished = true;

			// play sound
			Mix_HaltMusic();
			sound_manager_c::me()->play_sound(resource_manager_c::e_sound_loose);
		}
	}
}


void game_level_c::fill_renderbuffer(render_buffer_t& render_buffer)
{
	m_game_map->fill_renderbuffer(render_buffer);

	for (auto& explosion : m_explosion_objects)
	{
		explosion->fill_renderbuffer(render_buffer);
	}

	// ashfall
	{
		if (m_bomberman_character)
			m_bomberman_character->ashfall_fill_renderbuffer(render_buffer);

		for (auto& enemy : m_enemy_list)
		{
			enemy->ashfall_fill_renderbuffer(render_buffer);
		}
	}

#if 0 // for characters doing all work in game_level_c::update()
	if (m_bomberman_character)
	{
		m_bomberman_character->render();
		linkedlist_insert_front(m_render_buffer_list_ptr,
			m_bomberman_character->get_render_buffer());
	}

	for (auto& enemy : m_enemy_list)
	{
		enemy->render();
		linkedlist_insert_front(m_render_buffer_list_ptr,
			enemy->get_render_buffer());
	}
#endif
}


bool game_level_c::is_finished() const
{
	return m_is_finished;
}


game_level_c::e_level_state game_level_c::get_state() const
{
	return m_current_level_state;
}


bool game_level_c::add_explosion(map_cell3d_c* cell)
{
	bool continue_explosion = true;

	bool skip_cell_explosion = false;

	auto cell_type = cell->get_cell_type();

	if (cell_type == e_game_obj_type::e_gt_box ||
		cell_type == e_game_obj_type::e_gt_modifyer)
	{
		// check if current cell is modifier then from cache-list of modifier-list
		if (cell_type == e_game_obj_type::e_gt_modifyer)
		{
			auto it = std::find(m_modifiers_cell_list.begin(),
				m_modifiers_cell_list.end(),
				cell);

			if (it != m_modifiers_cell_list.cend())
			{
				m_modifiers_cell_list.erase(it);
			}
		}

		// pop object from cell, cell will contain next appended object
		m_game_map->pop_object_from_cell(cell);

		// check if next object in this cell is modifier
#if 0
		auto next_appended_obj = cell->get_current_game_object();
		if (next_appended_obj && next_appended_obj->get_game_type() == e_game_obj_type::e_gt_modifyer)
#else
		auto new_cell_type = cell->get_cell_type();
		if (new_cell_type == e_game_obj_type::e_gt_modifyer)
#endif
		{
			m_modifiers_cell_list.emplace_back(cell);
		}
		else if (new_cell_type == e_game_obj_type::e_gt_exit)
		{
			m_exit_cell = cell;

			if (m_enemy_list.size() > 0)
			{
				disable_finish_on_map();
			}
			else
			{
				enable_finish_on_map();
			}
		}

		continue_explosion = false;
	}
	else if (cell_type == e_game_obj_type::e_gt_bomb)
	{
		auto bomb_object = dynamic_cast<object_bomb3d_c*>
		(
			cell->get_current_game_object()
		);

		bomb_object->explode();

		continue_explosion = false;
	}
	else if (cell_type == e_game_obj_type::e_gt_exit)
	{
		continue_explosion = false;

		skip_cell_explosion = true;

		if (
#if 0
			m_enemy_list.size() > 0 &&
#endif
			m_enemy_list.size() < (size_t)m_max_mobs_limit_on_map)
		{
			// spawn 2 mobs

			int mobs_required = 2;
			int enemies_estimated_limit = m_max_mobs_limit_on_map - m_enemy_list.size();

			if (mobs_required > enemies_estimated_limit)
				mobs_required = enemies_estimated_limit;

			// spawn new random mobs
			for (int i = 0; i < mobs_required; ++i)
			{
				e_enemy_type enemy_type = (e_enemy_type)(rand() % e_enemies_count);
				spawn_enemy
				(
					enemy_type,
					cell->get_cell_x_index(),
					cell->get_cell_y_index()
				);
			}

			disable_finish_on_map();
		}
	}

	// fire explosion!
	if (!skip_cell_explosion)
		m_explosion_objects.emplace_back(new explosion3d_c(m_game_map.get(), cell));

	return continue_explosion;
}


void game_level_c::enable_finish_on_map()
{
	if (m_exit_cell)
	{
		object_exit3d_c* obj_exit = (object_exit3d_c*)m_exit_cell->get_current_game_object();
		obj_exit->enable_exit();
	}
}


void game_level_c::disable_finish_on_map()
{
	if (m_exit_cell)
	{
		object_exit3d_c* obj_exit = (object_exit3d_c*)m_exit_cell->get_current_game_object();
		obj_exit->disable_exit();
	}
}


void game_level_c::on_bomb_explosion(map_cell3d_c* bomb_cell)
{
	if (m_disable_bombs_on_map)
	{
		return;
	}

	// remove bomb object
	m_game_map->pop_object_from_cell(bomb_cell);

	// get current explosion power
	int explosion_power = m_bomberman_character->get_explosion_power();

	// current cell indices
	auto center_x_index = bomb_cell->get_cell_x_index();
	auto center_y_index = bomb_cell->get_cell_y_index();

	// add explosion to bomb cell
	add_explosion(bomb_cell);

	// create explosions in each direction
	int signs[2] = { +1, -1 };

	for (int i = 0; i < 2; ++i)
	{
		// horizontal cells explosion
		for (int x = 1; x <= explosion_power; ++x)
		{
			int next_cell_x_index = center_x_index - x * signs[i];

			if (next_cell_x_index < 0 ||
				next_cell_x_index >= m_map_x_cell_count)
			{
				break;
			}

			auto next_cell = m_game_map->get_cell(next_cell_x_index, center_y_index);
			if (next_cell->get_cell_type() == e_game_obj_type::e_gt_wall)
			{
				break;
			}

			if (!add_explosion(next_cell))
				break;
		}

		// vertical cells explosion
		for (int y = 1; y <= explosion_power; ++y)
		{
			int next_cell_y_index = center_y_index + y * signs[i];

			if (next_cell_y_index < 0 ||
				next_cell_y_index >= m_map_y_cell_count)
			{
				break;
			}

			auto next_cell = m_game_map->get_cell(center_x_index, next_cell_y_index);
			if (next_cell->get_cell_type() == e_game_obj_type::e_gt_wall)
			{
				break;
			}

			if (!add_explosion(next_cell))
				break;
		}
	}

	// play sound
	sound_manager_c::me()->play_sound(resource_manager_c::e_sound_explosion);

	// sort explosions by z-order for correct alpha-blending render
	std::sort
	(
		m_explosion_objects.begin(), m_explosion_objects.end(),

		[this](const explosion3d_c* e1, const explosion3d_c* e2) -> bool
		{
			return e1->get_global_position().z > e2->get_global_position().z;
		}
	);
}
