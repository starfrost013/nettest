#pragma once
#include "net.h"
#include "net_client.h"

//
// net_server.h : Server Core header
//

#define MAX_CLIENT_COUNT			32							//Maximum client count
#define NET_AUTH_TIMEOUT			5000						//How long the client has to authenticate with the server before it gets kicked (in milliseconds)

// Defines the server.
typedef struct netserver_s
{
	SDLNet_Server*			server;								//The server!									
	Uint16					port;								//SDL type
	int						max_clients;						//Maximum number of clients
	netclient_t				clients[MAX_CLIENT_COUNT];			//Local copy of client socket
	bool					connected;							//Is this client connected?
} netserver_t;

extern netserver_t* sys_server;									//Current instance server - CANNOT be present at the same time as sys_client

extern bool			sys_server_running;							//Is the server running?

bool	NET_InitServer();										//Initialises the server (sys_server)

void	NET_ServerMain();										//Server Main function

void	NET_ServerShutdown();									//Shutdown the server