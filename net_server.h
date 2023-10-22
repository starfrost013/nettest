#pragma once
#include "net.h"
#include "net_client.h"

//
// net_server.h : Server Core header
//

#define MAX_CLIENT_COUNT			32							//Maximum client count
#define NET_AUTH_TIMEOUT			5000						//How long the client has to authenticate with the server before it gets kicked (in milliseconds)
#define NET_PACKET_TIMEOUT			1000						//How long to wait to send a packet before giving up

// Defines the server.
typedef struct netserver_s
{
	SDLNet_Server*			server;								//The server!									
	Uint16					port;								//SDL type
	int						max_clients;						//Maximum number of clients
	int						num_clients;						//Current number of clients
	netclient_t				clients[MAX_CLIENT_COUNT];			//Local copy of client socket
	bool					connected;							//Is this client connected?
} netserver_t;

extern netserver_t* sys_server;									//Current instance server - CANNOT be present at the same time as sys_client

extern bool			sys_server_running;							//Is the server running?

bool	Server_Init();											//Initialises the server (sys_server)

void	Server_Main();											//Server Main function

void	Server_Shutdown();										//Shutdown the server