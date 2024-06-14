#pragma once

#include <map>
#include <string>

#include "Constants.h"
#include "G13Action.h"
#include "Macro.h"

using namespace std;

class G13
{
	private:
		G13Action* actions[G13_NUM_KEYS]{};

		struct libusb_device* device{nullptr};
		struct libusb_device_handle* handle{nullptr};

		int uinput_file;

		int loaded;
		int keepGoing;

		stick_mode_t stick_mode;
		int stick_keys[4];

		int bindings;

		Macro* loadMacro(int id);

		int read();
		void parse_joystick(const unsigned char* buf);
		void parse_key(int key, const unsigned char* byte);
		void parse_keys(const unsigned char* buf);

	public:
		explicit G13(libusb_device* device);
		~G13();

		void start();
		void stop();
		void loadBindings();
		void setColor(int r, int g, int b);
};
