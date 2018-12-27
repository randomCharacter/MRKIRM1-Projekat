#include "Device.h"



int main(int argc, char *argv[])
{
	int port;
	std::cout << "Enter router port:" << std::endl;
	std::cin >> port;
	Device d(port);
	d.start();

	return 0;
}