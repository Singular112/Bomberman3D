#pragma once

class state_context_c;

class state_base_c
{
public:
	state_base_c() {}
	virtual ~state_base_c() {}

	virtual void on_enter_state() = 0;
	virtual void on_exit_state() = 0;

	virtual void update_state(state_context_c* context) = 0;
};

class state_context_c
{
public:
	void set_state(state_base_c* new_state);

	void update_state();

private:
	state_base_c* m_current_state;
};
