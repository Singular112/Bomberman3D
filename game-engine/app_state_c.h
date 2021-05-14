#pragma once

class app_state_c
{
public:
	app_state_c() {}
	virtual ~app_state_c() {}

	virtual void on_enter_state() = 0;
};
