#pragma once

#include <future>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

class Device
{
private:
	int port;
public:
	Device(int port);
	~Device();

	void start();
};

