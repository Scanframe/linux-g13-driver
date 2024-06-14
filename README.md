# Readme File

This code was forked from Jim Gupta's code at <https://code.google.com/p/linux-g13-driver/>

I've forked this code because Google Code is deprecated and I don't want it lost.
Since then, I've started playing with it a little bit (Mostly directory structure and semantic changes for use with git).
I haven't changed the fundamental code because I'm by no means a coder, but we'll see what happens from here.  I'll eventually write a .service file for systemd and perhaps package it for the AUR.  
The service file should work with Ubuntu 15.04 and above.

## Notes

I've tried this on 64-bit Arch Linux, and it works so far.  
I haven't tried it in 64-bit Ubuntu, but it should work there, too.

I'm having a hard time getting udev rules to work so this can run in user mode.  
It seems that `/dev/uinput` is in the kernel and loads before anything is writable and the 
file's permissions don't get modified.

## Requirements

### Install needed packages

For Ubuntu based distributions. 

```shell
sudo apt-get --yes install build-essential libusb-1.0-0-dev
``` 

For Arch Linux you can install it by typing:

```shell
sudo pacman -S libusb
```

### Java version 1.6 or higher

For Ubuntu, it can be installed by typing:

```shell
sudo apt-get install default-jre
```

For Arch Linux it can be installed by typing:

```shell
sudo pacman -S jre8-openjdk
```

## Download

Download zip file from <https://github.com/Tetz95/linux-g13-driver/releases/latest>  
Unzip into your favorite directory.

## Build

Open a console (command prompt)  
Go to the directory where you unzipped your download  
type `make`.

## Running Application

Run the config tool first!  
In a command prompt go to the directory where you unzipped your download and type:

```shell
java -jar Linux-G13-GUI.jar
```

This will bring up the UI and create the initial files needed for your driver.  
All config files are saved in `$(HOME)/.g13`

Run the driver  
In a command prompt go to the directory where you unzipped your download and type:

```shell
sudo -E ./G13-Linux-Driver
```

The `-E` is to run it using your environment variables so, it doesn't look for the `.g13` directory in `/root`  
If you want to run the command and then detach it so, you can close the terminal:

```shell
sudo -E ./G13-Linux-Driver &
```

If you are configuring the application while the driver is running, the driver will not pick up 
changes unless you select a different bindings set, or you can restart the driver.
The top 4 buttons under the LCD screen select the bindings.  
The joystick currently only supports key mappings.
