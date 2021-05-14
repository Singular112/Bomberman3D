#pragma once

// bad, AWFUL, but fast implementation of main screen...

#include "cam_view_c.h"
#include "input_manager\input_manager_c.h"
#include "app_state_c.h"

#include <stack>

/*

Forte
Ink Free
Jokerman
Kristen ITC
Rockwell
Snap ITC
Wide Latin
Algerian
Buxton Sketch

Segoe Marker
Bauhaus 93

*/

using namespace sr::helpers::sdl_text;


struct base_handler
{
	base_handler() {}
	base_handler(const std::function<void(int)>& func)
		: m_func(func)
	{
	}

	operator bool() const
	{
		return m_func.operator bool();
	}

	std::function<void(int)> m_func;
};

struct on_mouse_over_handler
	: public base_handler
{
	on_mouse_over_handler() {}
	on_mouse_over_handler(const std::function<void(int)>& func)
		: base_handler(func)
	{
	}
};
struct on_mouse_out_handler
	: public base_handler
{
	on_mouse_out_handler() {}
	on_mouse_out_handler(const std::function<void(int)>& func)
		: base_handler(func)
	{
	}
};
struct on_mouse_click_handler
	: public base_handler
{
	on_mouse_click_handler() {}
	on_mouse_click_handler(const std::function<void(int)>& func)
		: base_handler(func)
	{
	}
};

//
class main_screen_c
	: public app_state_c
{
	class menu_item_base_c
	{
	public:
		menu_item_base_c() {}
		virtual ~menu_item_base_c() {}

		virtual void render(framebuffer_c* buffer) = 0;

		virtual void update(int mouse_x, int mouse_y) = 0;

		virtual void handle_input(input_state_s* state) = 0;

		virtual bool is_in_edit_mode() const { return false; }

		virtual void finish_edit_mode() {}
	};

	class menu_item_button_c
		: public menu_item_base_c
	{
	public:
		menu_item_button_c(int id,
			int x, int y,
			TTF_Font* font,
			const SDL_Color& color_selected,
			const SDL_Color& color_unselected,
			const std::string& text)
		{
			memset(&m_rect, 0, sizeof(m_rect));

			m_id = id;
			m_x = x;
			m_y = y;
			m_font = font;
			m_color_selected = color_selected;
			m_current_color = m_color_unselected = color_unselected;
			m_text = text;

			m_is_mouse_over_action_continues = false;
		}

		virtual ~menu_item_button_c()
		{
		}

		virtual void render(framebuffer_c* buffer)
		{
			m_rect = draw_text
			(
				buffer->get_renderer(),
				m_font,
				m_current_color,
				{},
				m_x, m_y, e_text_align_center,
				e_text_blend_mode::e_tbm_blended,
				"%s",
				m_text.c_str()
			);
		}

		virtual void update(int mouse_x, int mouse_y)
		{
			if (mouse_x >= m_rect.x &&
				mouse_y >= m_rect.y &&
				mouse_x < (m_rect.x + m_rect.w) &&
				mouse_y < (m_rect.y + m_rect.h))
			{
				m_is_mouse_out_action_continues = false;

				if (!m_is_mouse_over_action_continues)
				{
					m_is_mouse_over_action_continues = true;

					// change color
					m_current_color = m_color_selected;

					// fire once
					if (m_on_mouse_over_handler.m_func)
						m_on_mouse_over_handler.m_func(m_id);

					sound_manager_c::me()->play_sound(resource_manager_c::e_sound_menuitem_mouseover);
				}
			}
			else
			{
				m_is_mouse_over_action_continues = false;

				if (!m_is_mouse_out_action_continues)
				{
					m_is_mouse_out_action_continues = true;

					// change color
					m_current_color = m_color_unselected;

					// fire once
					if (m_on_mouse_out_handler.m_func)
						m_on_mouse_out_handler.m_func(m_id);
				}
			}
		}

		virtual void handle_input(input_state_s* state)
		{
			if (state->actions.find(e_input_actions::action_mouse_lclick)
				!= state->actions.cend() && m_is_mouse_over_action_continues)
			{
				if (m_on_mouse_click_handler.m_func)
					m_on_mouse_click_handler.m_func(m_id);

				sound_manager_c::me()->play_sound(resource_manager_c::e_sound_menuitem_click);
			}
		}

		virtual void set_text(const std::string& text)
		{
			m_text = text;
		}

		virtual const std::string& get_text() const
		{
			return m_text;
		}

		virtual void set_selected_color(SDL_Color color)
		{
			m_color_selected = color;
		}

		virtual void set_unselected_color(SDL_Color color)
		{
			m_color_unselected = color;
		}

		menu_item_button_c& operator +=(const on_mouse_over_handler& rhs)
		{
			m_on_mouse_over_handler = rhs;

			return *this;
		}

		menu_item_button_c& operator +=(const on_mouse_out_handler& rhs)
		{
			m_on_mouse_out_handler = rhs;

			return *this;
		}

		menu_item_button_c& operator +=(const on_mouse_click_handler& rhs)
		{
			m_on_mouse_click_handler = rhs;

			return *this;
		}

	private:
		int m_id;

		int m_x, m_y;
		TTF_Font* m_font;
		SDL_Color m_color_selected, m_color_unselected;
		std::string m_text;

		SDL_Color m_current_color;

		SDL_Rect m_rect;

		on_mouse_over_handler m_on_mouse_over_handler;
		on_mouse_out_handler m_on_mouse_out_handler;
		on_mouse_click_handler m_on_mouse_click_handler;

		bool m_is_mouse_over_action_continues,
			m_is_mouse_out_action_continues;
	};

	template <typename ParamT>
	class menu_item_param_c
		: public menu_item_base_c
	{
	public:
		menu_item_param_c(int id,
			int x1, int x2, int y,
			TTF_Font* font,
			const SDL_Color& color_selected,
			const SDL_Color& color_unselected,
			const std::string& param_caption,
			const std::vector<std::string>& param_values,
			const std::vector<ParamT>& param_data,
			int selected_param_value)
		{
			memset(&m_rect, 0, sizeof(m_rect));

			m_id = id;
			m_x1 = x1;
			m_x2 = x2;
			m_y = y;

			m_font = font;
			m_color_selected = color_selected;
			m_current_color = m_color_unselected = color_unselected;

			m_selected_param_value = selected_param_value;

			m_param_values = param_values;
			m_param_caption = param_caption;

			m_param_data = param_data;

			m_is_mouse_over_action_continues = false;
		}

		virtual ~menu_item_param_c() {}

		virtual void render(framebuffer_c* buffer)
		{
			draw_text
			(
				buffer->get_renderer(),
				m_font, m_color_unselected, {},
				m_x1, m_y, e_text_align_center,
				e_tbm_blended,
				"%s",
				m_param_caption.c_str()
			);

			m_rect = draw_text
			(
				buffer->get_renderer(),
				m_font, m_current_color, {},
				m_x2, m_y, e_text_align_center,
				e_tbm_blended,
				"%s",
				m_param_values[m_selected_param_value].c_str()
			);
		}

		virtual void update(int mouse_x, int mouse_y)
		{
			if (mouse_x >= m_rect.x &&
				mouse_y >= m_rect.y &&
				mouse_x < (m_rect.x + m_rect.w) &&
				mouse_y < (m_rect.y + m_rect.h))
			{
				m_is_mouse_out_action_continues = false;

				if (!m_is_mouse_over_action_continues)
				{
					m_is_mouse_over_action_continues = true;


					if (!m_entered_edit_mode)
					{
						// change color
						m_current_color = m_color_selected;
					}
				}
			}
			else
			{
				m_is_mouse_over_action_continues = false;

				if (!m_is_mouse_out_action_continues)
				{
					m_is_mouse_out_action_continues = true;


					if (!m_entered_edit_mode)
					{
						// change color
						m_current_color = m_color_unselected;
					}
				}
			}

			if (m_entered_edit_mode)
			{
				int period = (int)(SDL_GetTicks() * 0.4f);
				m_color_dim_factor = abs(sin(SR_DEG_TO_RAD(period % 360)));
				int color = (int)(m_color_dim_factor * 255);
				m_current_color = { color, color, color };
			}
		}

		virtual void handle_input(input_state_s* state)
		{
			if (m_is_mouse_over_action_continues)
			{
				if (state->actions.find(e_input_actions::action_mouse_lclick)
					!= state->actions.cend())
				{
					on_click_item(m_id, e_input_actions::action_mouse_lclick);
				}
				else if (state->actions.find(e_input_actions::action_mouse_rclick)
					!= state->actions.cend())
				{
					on_click_item(m_id, e_input_actions::action_mouse_rclick);
				}
			}
			else if (m_is_mouse_out_action_continues)
			{
				if (state->actions.find(e_input_actions::action_mouse_lclick)
					!= state->actions.cend())
				{
					if (m_param_values.size() == 1)
					{
						m_entered_edit_mode = false;

						m_current_color = m_color_unselected;
					}
				}
			}
		}

		virtual void set_text(const std::string& text)
		{
			m_param_values = { text };
		}

		virtual const std::string& get_param_value() const
		{
			return m_param_values[m_selected_param_value];
		}

		virtual void set_param_data(const ParamT& data)
		{
			m_param_data = { data };
		}

		virtual ParamT get_param_data() const
		{
			return m_param_data[m_selected_param_value];
		}

		virtual int get_selected_index() const
		{
			return m_selected_param_value;
		}

		virtual bool is_in_edit_mode() const
		{
			return m_entered_edit_mode;
		}

		virtual void finish_edit_mode()
		{
			m_entered_edit_mode = false;

			m_current_color = m_color_unselected;
		}

	private:
		virtual void on_click_item(int item_id, e_input_actions action)
		{
			if (m_param_values.size() == 1)
			{
				if (!m_entered_edit_mode && action == e_input_actions::action_mouse_lclick)
				{
					m_entered_edit_mode = true;

					m_current_color = m_color_selected;
				}
			}
			else
			{
				if (action == e_input_actions::action_mouse_lclick)
				{
					m_selected_param_value =
						(m_selected_param_value + 1) % m_param_values.size();
				}
				else if (action == e_input_actions::action_mouse_rclick)
				{
					m_selected_param_value--;

					if (m_selected_param_value < 0)
					{
						m_selected_param_value = m_param_values.size() - 1;
					}
				}
			}
		}

	private:
		int m_id;

		int m_x1, m_x2, m_y;

		SDL_Rect m_rect;

		TTF_Font* m_font;
		SDL_Color m_color_selected,
			m_color_unselected,
			m_current_color;

		float m_color_dim_factor = 1.0f;

		std::string m_param_caption;
		std::vector<std::string> m_param_values;
		std::vector<ParamT> m_param_data;
		int m_selected_param_value;

		bool m_entered_edit_mode = false;

		bool m_is_mouse_over_action_continues,
			m_is_mouse_out_action_continues;
	};

	struct page_s
	{
		int x, y, w, h;
		int position_y_pointer;

		void add_new_item() {}
	};

	enum e_menu_state
	{
		e_menu_mainscreen,
		e_menu_settings,
		e_menu_graphics_settings,
		e_menu_sound_settings,
		e_menu_control_settings
	};

	struct screen_resoulution_s
	{
		int w, h;
	};

public:
	main_screen_c();
	~main_screen_c();

	bool initialize(const std::string& resource_dir,
		const viewport_settings_s& viewport_settings,
		SDL_Window* sdl_window,
		framebuffer_c* buffer);

	void on_enter_state();

	bool is_on_main_page() const;

	void set_input_mapper();

	void set_callbacks(const std::function<void()>& on_new_game_callback,
		const std::function<void()>& on_exit_game_callback,
		const std::function<void()>& on_need_to_restart_callback)
	{
		m_on_new_game_callback = on_new_game_callback;
		m_on_exit_game_callback = on_exit_game_callback;
		m_on_need_to_restart = on_need_to_restart_callback;
	}

	void handle_input(input_state_s* state);

	void handle_sdl_input(const SDL_Event* event);

	void update();

private:
	void change_state(e_menu_state new_state);
	void set_prev_state();

	template <typename MenuItemContainerT>
	void update_menu(MenuItemContainerT& menu_items_container)
	{
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);

		for (auto& menu_item : menu_items_container)
		{
			menu_item->update(mouse_x, mouse_y);
			menu_item->render(m_framebuffer);
		}
	}

	void on_mouse_over_item(int item_id);
	void on_mouse_out_item(int item_id);
	void on_mouse_click_item(int item_id);

	void initialize_input_mapper();

	std::string get_resolution_str(const screen_resoulution_s& res);

private:
	SDL_Window* m_sdl_window = nullptr;
	framebuffer_c* m_framebuffer = nullptr;

	std::vector<screen_resoulution_s> m_screen_resoulutions;

	FIBITMAP* m_background_fib = nullptr;
	uint32_t* m_data_argb32bits = nullptr;

	uint32_t m_menu_entry_count = 0;

	viewport_settings_s m_viewport_settings;

	input_mapper_c m_menu_input_mapper;
	input_mapper_c m_game_input_mapper;	// copy of actual config input mapping

	e_menu_state m_current_state;
	std::stack<e_menu_state> m_state_stack; // store states transmissions history

	std::map<e_menu_state, std::function<void()>> m_menu_states; // map of update-state functions

	//
	TTF_Font* m_mainmenu_font = nullptr;

	int m_selected_item_id = 0;

	std::vector<e_input_actions> m_input_actions_list;

	//
	std::vector<std::unique_ptr<menu_item_base_c>> m_mainmenu_items;

	std::vector<std::unique_ptr<menu_item_base_c>> m_settings_menu_items;

	std::vector<std::unique_ptr<menu_item_base_c>> m_graphics_settings_items;

	std::vector<std::unique_ptr<menu_item_base_c>> m_sound_settings_items;

	std::vector<std::unique_ptr<menu_item_base_c>> m_control_settings_items;

	// callbacks
	std::function<void()> m_on_new_game_callback;
	std::function<void()> m_on_exit_game_callback;
	std::function<void()> m_on_need_to_restart;

	// input handlers
	typedef std::vector<std::unique_ptr<menu_item_base_c>>* items_ptr_t;
	std::map<e_menu_state, items_ptr_t> m_input_handlers;
};
