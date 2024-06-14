#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/uinput.h>
#include <unistd.h>

#include "Constants.h"
#include "Output.h"

int file = -1;
pthread_mutex_t p_lock{};

void send_event(int type, int code, int val)
{

	/*
	if (file == -1)
	{
		pthread_mutex_init(&lock_ptr, nullptr);
		file = create_uinput();
	}
	*/

	if (file == -2)
	{
		return;
	}

	pthread_mutex_lock(&p_lock);

	input_event event{};

	memset(&event, 0, sizeof(event));
	gettimeofday(&event.time, nullptr);
	event.type = type;
	event.code = code;
	event.value = val;

	write(file, &event, sizeof(event));

	//std::cout << "write(type=" << type << ", code=" <<  code << ", val=" << val << ")\n";

	pthread_mutex_unlock(&p_lock);
}

void flush()
{
	pthread_mutex_lock(&p_lock);
	fsync(file);
	pthread_mutex_unlock(&p_lock);
}

int create_uinput()
{
	//std::cout << "create uinput\n";

	uinput_user_dev uinp{};
	const char* dev_uinput_fname =
		access("/dev/input/uinput", F_OK) == 0 ? "/dev/input/uinput" : access("/dev/uinput", F_OK) == 0 ? "/dev/uinput"
																																																		: nullptr;

	if (!dev_uinput_fname) {
		std::cerr << "Could not find an uinput device" << std::endl;
		return -2;
	}

	if (access(dev_uinput_fname, W_OK) != 0)
	{
		std::cerr << dev_uinput_fname << " doesn't grant write permissions" << std::endl;
		return -2;
	}

	auto ufile = open(dev_uinput_fname, O_WRONLY | O_NDELAY);
	if (ufile <= 0)
	{
		std::cerr << "Could not open uinput" << std::endl;
		return -2;
	}

	memset(&uinp, 0, sizeof(uinp));
	char name[] = "G13";
	strncpy(uinp.name, name, sizeof(name));
	uinp.id.version = 1;
	uinp.id.bustype = BUS_USB;
	uinp.id.product = G13_PRODUCT_ID;
	uinp.id.vendor = G13_VENDOR_ID;
	uinp.absmin[ABS_X] = 0;
	uinp.absmin[ABS_Y] = 0;
	uinp.absmax[ABS_X] = 0xff;
	uinp.absmax[ABS_Y] = 0xff;
	//  uinp.absfuzz[ABS_X] = 4;
	//  uinp.absfuzz[ABS_Y] = 4;
	//  uinp.absflat[ABS_X] = 0x80;
	//  uinp.absflat[ABS_Y] = 0x80;

	ioctl(ufile, UI_SET_EVBIT, EV_KEY);
	ioctl(ufile, UI_SET_EVBIT, EV_ABS);
	/*  ioctl(ufile, UI_SET_EVBIT, EV_REL);*/
	ioctl(ufile, UI_SET_MSCBIT, MSC_SCAN);
	ioctl(ufile, UI_SET_ABSBIT, ABS_X);
	ioctl(ufile, UI_SET_ABSBIT, ABS_Y);
	/*  ioctl(ufile, UI_SET_RELBIT, REL_X);
	 ioctl(ufile, UI_SET_RELBIT, REL_Y);*/
	for (int i = 0; i < 256; i++)
		ioctl(ufile, UI_SET_KEYBIT, i);
	ioctl(ufile, UI_SET_KEYBIT, BTN_THUMB);

	auto written = write(ufile, &uinp, sizeof(uinp));
	if (written < 0)
	{
		std::cerr << "Could not write to uinput device (" << written << ")" << std::endl;
		return -2;
	}

	auto ret_code = ioctl(ufile, UI_DEV_CREATE);
	if (ret_code)
	{
		std::cerr << "Error creating uinput device for G13" << std::endl;
		return -2;
	}

	file = ufile;
	return ufile;
}
