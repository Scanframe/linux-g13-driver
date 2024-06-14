#include <cstdlib>
#include <cstring>
#include <iostream>
#include <linux/uinput.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>

#include "MacroAction.h"

void* execute_macro(void* args)
{
	auto* t = (MacroAction::MultiEventThread*) args;
	t->execute();
	return nullptr;
}

MacroAction::Event* MacroAction::tokenToEvent(char* token)
{
	if (token == nullptr)
	{
		return nullptr;
	}

	if (strncmp(token, "kd.", 3) == 0)
	{
		int code = atoi(&(token[3]));
		return (MacroAction::Event*) new KeyDownEvent(code);
	}
	else if (strncmp(token, "ku.", 3) == 0)
	{
		int code = atoi(&(token[3]));
		return (MacroAction::Event*) new KeyUpEvent(code);
	}
	else if (strncmp(token, "d.", 2) == 0)
	{
		int delay = atoi(&(token[2]));
		return (MacroAction::Event*) new DelayEvent(delay);
	}
	else
	{
		cout << "MacroAction::tokenToEvent() unknown token: " << token << "\n";
	}
	return nullptr;
}

MacroAction::MacroAction(char* tokens)
{
	//std::cout << "MacroAction::MacroAction() tokens=" << tokens << "\n";
	pthread_attr_init(&_attr);
	_repeats = 0;
	_thread = nullptr;
	if (tokens == nullptr)
	{
		return;
	}
	// kd.keycode,ku.keycode,d.time
	char* token = strtok(tokens, ",");
	while (token != nullptr) {
		MacroAction::Event* event = tokenToEvent(token);
		if (event != nullptr) {
			events.push_back(event);
		}
		token = strtok(nullptr, ",");
	}
}

MacroAction::~MacroAction() = default;

void MacroAction::key_down()
{

	if (_thread != nullptr)
	{
		cout << "MacroAction::key_down(): current thread in action\n";
		return;
	}

	//cout << "MacroAction::key_down()\n";

	_thread = new MultiEventThread();
	_thread->keepRepeating = _repeats;
	_thread->local_events = events;

	pthread_t pthread;
	pthread_create(&pthread, &_attr, execute_macro, _thread);
	//cout << "MacroAction::key_down() thread created\n";
}

void MacroAction::key_up()
{

	if (_thread != nullptr) {
		_thread->keepRepeating = false;
	}

	_thread = nullptr;
}

int MacroAction::getRepeats() const
{
	return _repeats;
}

void MacroAction::setRepeats(int repeats)
{
	_repeats = repeats;
}

vector<MacroAction::Event*> MacroAction::getEvents()
{
	return events;
}
