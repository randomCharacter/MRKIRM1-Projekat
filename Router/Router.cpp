#include "Router.h"

using namespace std;

Router::Router(int port) : port(port)
{
}

void on_client_connect(Router &r, SOCKET client)
{
	int port = r.GetFreeAddr(DEVICE, client);
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
			if (r.SendMsg(dst_port, buffer))
			{
				printf("Message: \"%s\" sent to address %d\n", buffer + 4 + 2 * sizeof(int), dst_port);
			}
			else
			{
				printf("Failed to send message to given address\n");
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
}

void Router::start()
{
	WSADATA wsa_data;
	SOCKADDR_IN router_addr, client_addr;

	WSAStartup(MAKEWORD(2, 2), &wsa_data);
	
	const auto server = socket(AF_INET, SOCK_STREAM, 0);

	router_addr.sin_addr.s_addr = INADDR_ANY;
	router_addr.sin_family = AF_INET;
	router_addr.sin_port = htons(5555);

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
}

int Router::SendMsg(int dst_port, char *msg)
{
	if (nodes[dst_port].type == FREE) {
		return 0;
	}
	send(nodes[dst_port].sock, msg, 1024, 0);

	return 1;
}