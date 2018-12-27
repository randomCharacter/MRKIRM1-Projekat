#include "Router.h"

using namespace std;

Router::Router(int isMain, int port, int mainPort) : port(port)
{
	if (!isMain)
	{
		char buff[1024] = MSG_R_CONNECT;

		for (int i = 0; i < 1024; i++)
		{
			nodes[i].type = OTHER_ROUTER;
			nodes[i].sock = 0;
		}

		WSADATA wsa_data;
		SOCKADDR_IN addr;

		WSAStartup(MAKEWORD(2, 0), &wsa_data);

		mainRouter = socket(AF_INET, SOCK_STREAM, 0);

		InetPton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

		addr.sin_family = AF_INET;
		addr.sin_port = htons(mainPort);

		if (connect(mainRouter, reinterpret_cast<SOCKADDR *>(&addr), sizeof(addr)) < 0)
		{
			cout << "Failed to connect to router on port " << port << endl;
			exit(1);
		}
		cout << "Connected to main server!" << endl;

		send(mainRouter, buff, strlen(buff) + 1, 0);
		recv(mainRouter, (char *)&localAddress, sizeof(int), 0);
		nodes[localAddress].type = MYSELF;
		nodes[localAddress].sock = 0;
		nodes[localAddress].addr = localAddress;

		cout << "My local address is " << localAddress << endl;

		memcpy(buff, MSG_R_REQUEST_ADDRESSES, 5);

		for (int i = 0; i < 10; i++)
		{
			int newAddr = 0;
			send(mainRouter, buff, strlen(buff) + 1, 0);
			recv(mainRouter, (char*)&newAddr, sizeof(int), 0);

			nodes[newAddr].addr = 0;
			nodes[newAddr].sock = 0;
			nodes[newAddr].type = FREE;
			cout << "Got new address for devices:" << newAddr << endl;
		}

		nodes[0].addr = 0;
		nodes[0].sock = mainRouter;
		nodes[0].type = ROUTER;

		thread *t = new thread(on_client_connect, ref(*this), mainRouter);
	}
	else
	{
		nodes[0].addr = 0;
		nodes[0].sock = 0;
		nodes[0].type = MYSELF;
	}
}

void on_client_connect(Router &r, SOCKET client)
{
	int port;
	char buffer[1024];
	for (;;)
	{
		recv(client, buffer, sizeof(buffer), 0);

		char op[5];
		for (int i = 0; i < 4; i++)
		{
			op[i] = buffer[i];
		}
		op[4] = 0;

		if (strcmp(op, MSG_CONNECT) == 0)
		{
			port = r.GetFreeAddr(DEVICE, client);
			send(client, (const char *)&port, sizeof(int), 0);
			cout << "Sent port number to client" << port << endl;
			if (port == -1) {
				break;
			}
		}
		else if (strcmp(op, MSG_DISCONNECT) == 0)
		{
			r.nodes[port].type = FREE;
			break;
		}
		else if (strcmp(op, MSG_MESSAGE) == 0)
		{
			int src_port, dst_port;
			memcpy(&src_port, buffer + 4, sizeof(int));
			memcpy(&dst_port, buffer + 4 + sizeof(int), sizeof(int));

			if (r.nodes[src_port].addr == 0)
			{
				r.nodes[src_port].sock = client;
			}

			if (r.SendMsg(dst_port, buffer))
			{
				printf("Message: \"%s\" sent to address %d\n", buffer + 4 + 2 * sizeof(int), dst_port);
			}
			else
			{
				printf("Failed to send message to given address\n");
			}
		}
		else if (strcmp(op, MSG_R_CONNECT))
		{
			port = r.GetFreeAddr(ROUTER, client);
			send(client, (const char *)&port, sizeof(int), 0);
			cout << "New router connected on address: " << port << endl;
			if (port == -1) {
				break;
			}
		}
		else if (strcmp(op, MSG_R_REQUEST_ADDRESSES))
		{
			port = r.GetFreeAddr(OTHER_ROUTER, client);
			send(client, (const char *)&port, sizeof(int), 0);
			cout << "Sent new address:" << port << endl;
			if (port == -1) {
				break;
			}
		}

		cout << "Client says: " << buffer << endl;
		memset(buffer, 0, sizeof(buffer));
	}

	closesocket(client);
	cout << "Client disconnected." << endl;
}

Router::~Router()
{
	if (!isMain)
	{
		closesocket(mainRouter);

		WSACleanup();
		cout << "Socket closed." << endl << endl;
	}
}

void Router::start()
{
	WSADATA wsa_data;
	SOCKADDR_IN router_addr, client_addr;

	WSAStartup(MAKEWORD(2, 2), &wsa_data);
	
	const auto server = socket(AF_INET, SOCK_STREAM, 0);

	router_addr.sin_addr.s_addr = INADDR_ANY;
	router_addr.sin_family = AF_INET;
	router_addr.sin_port = htons(port);

	::bind(server, reinterpret_cast<SOCKADDR *>(&router_addr), sizeof(router_addr));
	listen(server, 0);

	cout << "Router started on port " << port << endl;

	int client_addr_size = sizeof(client_addr);

	for (;;)
	{
		SOCKET client;

		if ((client = accept(server, reinterpret_cast<SOCKADDR *>(&client_addr), &client_addr_size)) != INVALID_SOCKET)
		{
			thread *t = new thread(on_client_connect, ref(*this), client);
		}

		const auto last_error = WSAGetLastError();

		if (last_error > 0)
		{
			cout << "Error: " << last_error << endl;
		}
	}
}

int Router::GetFreeAddr(NodeType type, SOCKET s)
{
	for (int i = 0; i < 1024; i++) {
		if (nodes[i].type == FREE) {
			cout << "NODE" << i << nodes[i].type << " " << type << endl;
			nodes[i].type = DEVICE;
			cout << "NODE" << i << nodes[i].type << " " << type << endl;
			nodes[i].sock = s;
			nodes[i].addr = i;
			return i;
		}
	}
	return -1;
}

int Router::SendMsg(int dst_port, char *msg)
{
	cout << "TYPE" << nodes[dst_port].type << endl;
	if (nodes[dst_port].type == FREE || nodes[dst_port].type == OTHER_ROUTER)
	{
		//send(mainRouter, msg, 1024, 0);
		for (int i = 0; i < 1024; i++)
		{
			if (nodes[i].type == ROUTER)
			{
				send(nodes[i].sock, msg, 1024, 0);
			}
		}
	}
	else
	{
		send(nodes[dst_port].sock, msg, 1024, 0);
	}
	return 1;
}