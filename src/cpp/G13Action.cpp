#include "G13Action.h"

G13Action::G13Action() = default;

G13Action::~G13Action() = default;

void G13Action::key_down()
{
	//std::cout << "G13Action::key_down()" << std::::endl;
}

void G13Action::key_up()
{
	//std::cout << "G13Action::key_up()" << std::::endl;
}

int G13Action::set(int state)
{
	int s = 0;
	if (state != 0)
	{
		s = 1;
	}
	if (s != pressed)
	{
		pressed = s;
		if (s)
		{
			key_down();
		}
		else
		{
			key_up();
		}
		return 1;
	}
	return 0;
}

int G13Action::isPressed() const
{
	return pressed;
}
