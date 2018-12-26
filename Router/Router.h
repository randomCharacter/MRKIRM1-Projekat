#pragma once

#include <future>
#include <iostream>
#include <winsock2.h>

#include "Messages.h"

#pragma comment(lib, "Ws2_32.lib")

enum NodeType { DEVICE, ROUTER, FREE, OTHER_ROUTER };

struct Node {
	NodeType type = FREE;
	int addr;
	SOCKET sock;
};

class Router
{
private:
	int port;
	Node nodes[1024];
	
public:
	Router(int port);
	~Router();

	void start();
	int GetFreeAddr(NodeType type, SOCKET s);
	int SendMsg(int dst_port, char *msg);

	friend void on_client_connect(Router &r, SOCKET client);
};

void on_client_connect(Router &r, SOCKET client);
