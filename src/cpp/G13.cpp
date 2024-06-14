#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <libusb-1.0/libusb.h>
#include <linux/uinput.h>
#include "Constants.h"
#include "G13.h"
#include "G13Action.h"
#include "MacroAction.h"
#include "Output.h"
#include "PassThroughAction.h"

void trim(char* s)
{
	// Trim spaces and tabs from beginning:
	size_t i = 0;
	size_t j = 0;
	while ((s[i] == ' ') || (s[i] == '\t'))
	{
		i++;
	}
	if (i > 0) {
		for (j = 0; j < strlen(s); j++)
		{
			s[j] = s[j + i];
		}
		s[j] = '\0';
	}

	// Trim spaces and tabs from end:
	i = strlen(s) - 1;
	while ((s[i] == ' ') || (s[i] == '\t'))
	{
		i--;
	}
	if (i < (strlen(s) - 1))
	{
		s[i + 1] = '\0';
	}
}

G13::G13(libusb_device* device)
{
	this->device = device;
	this->loaded = 0;
	this->bindings = 0;
	this->stick_mode = STICK_KEYS;
	for (auto& action: actions)
	{
		action = new G13Action();
	}

	if (libusb_open(device, &handle) != 0)
	{
		std::cerr << "Error opening G13 device" << std::endl;
		return;
	}

	if (libusb_kernel_driver_active(handle, 0) == 1)
	{
		if (libusb_detach_kernel_driver(handle, 0) == 0)
		{
			cout << "Kernel driver detached" << endl;
		}
	}

	if (libusb_claim_interface(handle, 0) < 0)
	{
		cerr << "Cannot Claim Interface" << endl;
		return;
	}

	setColor(128, 128, 128);

	this->loaded = 1;
}

G13::~G13()
{
	if (!this->loaded)
	{
		return;
	}
	setColor(128, 128, 128);
	libusb_release_interface(this->handle, 0);
	libusb_close(this->handle);
}

void G13::start()
{
	if (!this->loaded)
	{
		return;
	}
	loadBindings();
	keepGoing = 1;
	while (keepGoing)
	{
		read();
	}
}

void G13::stop()
{
	if (!this->loaded)
	{
		return;
	}
	keepGoing = 0;
}

Macro* G13::loadMacro(int num)
{
	char filename[1024];

	sprintf(filename, "%s/.g13/macro-%d.properties", getenv("HOME"), num);
	//cout << "G13::loadMacro(" << num << ") filename=" << filename << "\n";
	ifstream file(filename);

	if (!file.is_open())
	{
		cout << "Could not open config file: " << filename << "\n";
		return nullptr;
	}

	auto* macro = new Macro();
	macro->setId(num);
	while (file.good()) {
		string line;
		getline(file, line);
		//cout << line << "\n";

		char l[1024];
		strcpy(l, (char*) line.c_str());
		trim(l);
		if (strlen(l) > 0 && l[0] != '#')
		{
			char* key = strtok(l, "=");
			char* value = strtok(nullptr, "\n");
			trim(key);
			trim(value);
			//cout << "G13::loadMacro(" << num << ") key=" << key << ", value=" << value << "\n";
			if (strcmp(key, "name") == 0)
			{
				macro->setName(value);
			}
			else if (strcmp(key, "sequence") == 0)
			{
				macro->setSequence(value);
			}
		}
	}

	return macro;
}

void G13::loadBindings()
{
	char filename[1024];

	sprintf(filename, "%s/.g13/bindings-%d.properties", getenv("HOME"), bindings);
	cout << "loading " << filename << "\n";

	ifstream file(filename);
	if (!file.is_open())
	{
		cout << "Could not open config file: " << filename << "\n";
		setColor(128, 128, 128);
		return;
	}

	while (file.good())
	{
		string line;
		getline(file, line);

		char l[1024];
		strcpy(l, (char*) line.c_str());
		trim(l);
		if (strlen(l) > 0) {
			char* key = strtok(l, "=");
			if (key[0] == '#')
			{
				// ignore line
			}
			else if (strcmp(key, "color") == 0) {
				char* num = strtok(nullptr, ",");
				int r = atoi(num);
				num = strtok(nullptr, ",");
				int g = atoi(num);
				num = strtok(nullptr, ",");
				int b = atoi(num);

				setColor(r, g, b);
			}
			else if (strcmp(key, "stick_mode") == 0) {
			}
			else if (key[0] == 'G') {
				int gKey = atoi(&key[1]);
				//cout << "gKey = " << gKey << "\n";
				char* type = strtok(nullptr, ",");
				trim(type);
				//cout << "type = " << type << "\n";
				if (strcmp(type, "p") == 0) { /* passthrough */
					char* keytype = strtok(nullptr, ",\n ");
					trim(keytype);
					int keycode = atoi(&keytype[2]);

					if (actions[gKey] != nullptr) {
						delete actions[gKey];
					}

					//cout << "assigning G" << gKey << " to keycode " << keycode << "\n";
					G13Action* action = new PassThroughAction(keycode);
					actions[gKey] = action;
				}
				else if (strcmp(type, "m") == 0) { /* macro */
					int macroId = atoi(strtok(nullptr, ",\n "));
					int repeats = atoi(strtok(nullptr, ",\n "));
					//cout << "macroId = " << macroId << "\n";
					Macro* macro = loadMacro(macroId);
					auto* action = new MacroAction(macro->getSequence());
					action->setRepeats(repeats);
					actions[gKey] = action;
				}
				else {
					cout << "G13::loadBindings() unknown type '" << type << "\n";
				}
			}
			else {
				cout << "G13::loadBindings() Unknown first token: " << key << "\n";
			}
		}

		//cout << line << endl;
	}

	file.close();
}

void G13::setColor(int red, int green, int blue)
{
	int error;
	unsigned char usb_data[] = {5, 0, 0, 0, 0};
	usb_data[1] = red;
	usb_data[2] = green;
	usb_data[3] = blue;

	error = libusb_control_transfer(
		handle,
		(uint8_t) LIBUSB_REQUEST_TYPE_CLASS | (uint8_t) LIBUSB_RECIPIENT_INTERFACE,
		9, 0x307, 0, usb_data, 5, 1000
	);

	if (error != 5) {
		cerr << "Problem sending data" << endl;
	}
}

int G13::read()
{
	unsigned char buffer[G13_REPORT_SIZE];
	int size;
	int error = libusb_interrupt_transfer(handle, LIBUSB_ENDPOINT_IN | G13_KEY_ENDPOINT, buffer, G13_REPORT_SIZE, &size, 1000);
	if (error && error != LIBUSB_ERROR_TIMEOUT)
	{
		const char* error_str;
		switch (error)
		{
			case LIBUSB_SUCCESS:
				error_str = "LIBUSB_SUCCESS";
				break;
			case LIBUSB_ERROR_IO:
				error_str = "LIBUSB_ERROR_IO";
				break;
			case LIBUSB_ERROR_INVALID_PARAM:
				error_str = "LIBUSB_ERROR_INVALID_PARAM";
				break;
			case LIBUSB_ERROR_ACCESS:
				error_str = "LIBUSB_ERROR_ACCESS";
				break;
			case LIBUSB_ERROR_NO_DEVICE:
				error_str = "LIBUSB_ERROR_NO_DEVICE";
				break;
			case LIBUSB_ERROR_NOT_FOUND:
				error_str = "LIBUSB_ERROR_NOT_FOUND";
				break;
			case LIBUSB_ERROR_BUSY:
				error_str = "LIBUSB_ERROR_BUSY";
				break;
			case LIBUSB_ERROR_TIMEOUT:
				error_str = "LIBUSB_ERROR_TIMEOUT";
				break;
			case LIBUSB_ERROR_OVERFLOW:
				error_str = "LIBUSB_ERROR_OVERFLOW";
				break;
			case LIBUSB_ERROR_PIPE:
				error_str = "LIBUSB_ERROR_PIPE";
				break;
			case LIBUSB_ERROR_INTERRUPTED:
				error_str = "LIBUSB_ERROR_INTERRUPTED";
				break;
			case LIBUSB_ERROR_NO_MEM:
				error_str = "LIBUSB_ERROR_NO_MEM";
				break;
			case LIBUSB_ERROR_NOT_SUPPORTED:
				error_str = "LIBUSB_ERROR_NOT_SUPPORTED";
				break;
			case LIBUSB_ERROR_OTHER:
			default:
				error_str = "LIBUSB_ERROR_OTHER";
				break;
		}
		cerr << "Error while reading keys: " << error << " (" << error_str << ")" << endl;
		cerr << "Stopping daemon" << endl;
		return -1;
	}

	if (size == G13_REPORT_SIZE) {
		parse_joystick(buffer);
		parse_keys(buffer);
		send_event(EV_SYN, SYN_REPORT, 0);
	}
	return 0;
}

void G13::parse_joystick(const unsigned char* buf)
{
	int stick_x = buf[1];
	int stick_y = buf[2];

	//cout << "stick = (" << stick_x << ", " << stick_y << ")\n";

	if (stick_mode == STICK_ABSOLUTE) {
		send_event(EV_ABS, ABS_X, stick_x);
		send_event(EV_ABS, ABS_Y, stick_y);
	}
	else if (stick_mode == STICK_KEYS) {

		// 36=up, 37=left, 38=right, 39=down
		int pressed[4];

		if (stick_y <= 96) {
			pressed[0] = 1;
			pressed[3] = 0;
		}
		else if (stick_y >= 160) {
			pressed[0] = 0;
			pressed[3] = 1;
		}
		else {
			pressed[0] = 0;
			pressed[3] = 0;
		}

		if (stick_x <= 96) {
			pressed[1] = 1;
			pressed[2] = 0;
		}
		else if (stick_x >= 160) {
			pressed[1] = 0;
			pressed[2] = 1;
		}
		else {
			pressed[1] = 0;
			pressed[2] = 0;
		}

		int codes[4] = {36, 37, 38, 39};
		for (int i = 0; i < 4; i++) {
			int key = codes[i];
			int p = pressed[i];
			if (actions[key]->set(p)) {
				//cout << "key " << key << ", pressed=" << p << ", actions[key]->isPressed()="
				//		<< actions[key]->isPressed() <<  ", x=" << stick_x << "\n";
			}
		}
	}
	else {
		/*    send_event(g13->uinput_file, EV_REL, REL_X, stick_x/16 - 8);
		 send_event(g13->uinput_file, EV_REL, REL_Y, stick_y/16 - 8);*/
	}
}
void G13::parse_key(int key, const unsigned char* byte)
{
	unsigned char actual_byte = byte[key / 8];
	unsigned char mask = 1 << (key % 8);

	int pressed = actual_byte & mask;

	switch (key)
	{
		case 25:// key 25-28 are mapped to change bindings
		case 26:
		case 27:
		case 28:
			if (pressed) {
				//cout << "key " << key << "\n";
				bindings = key - 25;
				loadBindings();
			}
			return;

		case 36:// key 36-39 are mapped as joystick keys
		case 37:
		case 38:
		case 39:
		default:
			return;
	}

	int changed = actions[key]->set(pressed);

	/*
	if (changed) {
		string type = "released";
		if (actions[key]->isPressed()) {
			type = "pressed";
		}
		cout << "G" << (key+1) << " " << type << "\n";
	}
	*/
}

void G13::parse_keys(const unsigned char* buf)
{
	parse_key(G13_KEY_G1, buf + 3);
	parse_key(G13_KEY_G2, buf + 3);
	parse_key(G13_KEY_G3, buf + 3);
	parse_key(G13_KEY_G4, buf + 3);
	parse_key(G13_KEY_G5, buf + 3);
	parse_key(G13_KEY_G6, buf + 3);
	parse_key(G13_KEY_G7, buf + 3);
	parse_key(G13_KEY_G8, buf + 3);

	parse_key(G13_KEY_G9, buf + 3);
	parse_key(G13_KEY_G10, buf + 3);
	parse_key(G13_KEY_G11, buf + 3);
	parse_key(G13_KEY_G12, buf + 3);
	parse_key(G13_KEY_G13, buf + 3);
	parse_key(G13_KEY_G14, buf + 3);
	parse_key(G13_KEY_G15, buf + 3);
	parse_key(G13_KEY_G16, buf + 3);

	parse_key(G13_KEY_G17, buf + 3);
	parse_key(G13_KEY_G18, buf + 3);
	parse_key(G13_KEY_G19, buf + 3);
	parse_key(G13_KEY_G20, buf + 3);
	parse_key(G13_KEY_G21, buf + 3);
	parse_key(G13_KEY_G22, buf + 3);
	//  parse_key(G13_KEY_LIGHT_STATE, buf+3);

	parse_key(G13_KEY_BD, buf + 3);
	parse_key(G13_KEY_L1, buf + 3);
	parse_key(G13_KEY_L2, buf + 3);
	parse_key(G13_KEY_L3, buf + 3);
	parse_key(G13_KEY_L4, buf + 3);
	parse_key(G13_KEY_M1, buf + 3);
	parse_key(G13_KEY_M2, buf + 3);

	parse_key(G13_KEY_M3, buf + 3);
	parse_key(G13_KEY_MR, buf + 3);
	parse_key(G13_KEY_LEFT, buf + 3);
	parse_key(G13_KEY_DOWN, buf + 3);
	parse_key(G13_KEY_TOP, buf + 3);
	parse_key(G13_KEY_LIGHT, buf + 3);
	//  parse_key(G13_KEY_LIGHT2, buf+3, file);
	/*  cout << hex << setw(2) << setfill('0') << (int)buf[7];
	 cout << hex << setw(2) << setfill('0') << (int)buf[6];
	 cout << hex << setw(2) << setfill('0') << (int)buf[5];
	 cout << hex << setw(2) << setfill('0') << (int)buf[4];
	 cout << hex << setw(2) << setfill('0') << (int)buf[3] << endl;*/
}
