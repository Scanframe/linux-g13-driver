#include <linux/uinput.h>

#include "Output.h"
#include "PassThroughAction.h"


PassThroughAction::PassThroughAction(int code)
{
	this->_keycode = code;
}
PassThroughAction::~PassThroughAction() = default;

[[maybe_unused]] int PassThroughAction::getKeyCode() const
{
	return _keycode;
}

void PassThroughAction::setKeyCode(int code)
{
	this->_keycode = code;
}

void PassThroughAction::key_down()
{
	send_event(EV_KEY, _keycode, 1);
}

void PassThroughAction::key_up()
{
	send_event(EV_KEY, _keycode, 0);
}
