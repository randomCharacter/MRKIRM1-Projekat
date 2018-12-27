#include "Router.h"

int main()
{
	int mainRouter, port , connectToPort = 0;
	std::cout << "Is this the main router? [1 - Yes, 0 - No]:" << std::endl;
	std::cin >> mainRouter;

	std::cout << "Enter the port:" << std::endl;
	std::cin >> port;
	
	if (!mainRouter)
	{
		std::cout << "Enter the port of main router:" << std::endl;
		std::cin >> connectToPort;
	}
	Router r(mainRouter, port, connectToPort);
	r.start();

	return 0;
}