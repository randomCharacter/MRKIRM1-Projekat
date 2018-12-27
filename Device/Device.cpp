#include "Device.h"
#include "../Router/Messages.h"

using namespace std;

Device::Device(int port) : port(port)
{
}


Device::~Device()
{
}

void Device::start()
{
	char buffer[1024] = { 0 };
	strcpy_s(buffer, 5, MSG_CONNECT);

	WSADATA wsa_data;
	SOCKADDR_IN addr;

	WSAStartup(MAKEWORD(2, 0), &wsa_data);
	
	const auto server = socket(AF_INET, SOCK_STREAM, 0);

	InetPton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (connect(server, reinterpret_cast<SOCKADDR *>(&addr), sizeof(addr)) < 0)
	{
		cout << "Failed to connect to router on port " << port << endl;
		exit(1);
	}
	cout << "Connected to server!" << endl;

	send(server, buffer, strlen(buffer) + 1, 0);
	cout << "Connection request sent!" << endl;

	recv(server, (char *)&port, sizeof(int), 0);
	cout << "Received address:" << port << endl;

	for (;;)
	{
		int option;
		cout << "Choose action:\n1. Send message\n2. Receive message\n3. Disconect\n>";
		cin >> option;
		if (option == 1)
		{
			int dstAddr;
			char message[1000];
			char op[5] = MSG_MESSAGE;
			cout << "Enter destination address: " << endl;
			cin >> dstAddr;
			cout << "Enter message" << endl;
			scanf_s("%s", message, 1000);
			memcpy(buffer, op, 4);
			memcpy(buffer + 4, &port, sizeof(int));
			memcpy(buffer + 4 + sizeof(int), &dstAddr, sizeof(int));
			memcpy(buffer + 4 + 2 * sizeof(int), message, strlen(message) + 1);
			send(server, buffer, 1024, 0);
		}
		else if (option == 2)
		{
			recv(server, buffer, 1024, 0);
			printf("Received mesage: %s\n", buffer + 4 + 2*sizeof(int));
		}
		else if (option == 3)
		{
			memcpy(buffer, MSG_DISCONNECT, 5);
			send(server, buffer, sizeof(buffer), 0);
			break;
		}

	}

	closesocket(server);

	WSACleanup();
	cout << "Socket closed." << endl << endl;

	system("PAUSE");
}