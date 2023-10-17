#pragma once
#include "net_client.h"

bool			sys_client_running;				// Is the client running?	
netclient_t*	sys_client;						// Current instance client object (cannot be the same as net_server)

void NET_InitClient()
{
	sys_client = malloc(sizeof(netclient_t));

	if (sys_client == NULL)
	{
		Logging_LogAll("Error initialising client - failed to allocate memory");
		return;
	}

	memset(sys_client, 0x00, sizeof(netclient_t));
}

bool NET_ConnectClient(char* address, Uint16 port)
{
	printf("Connecting to server at %s\n", address);

	// set port and timeout
	if (port == 0) port = NET_SERVER_PORT;

	sys_client->server_port = port;
	if (sys_client->timeout == 0) sys_client->timeout = 3000;

	// try and resolve the server. if we time out by sys_client->timeout, don't bother
	SDLNet_Address* addr = SDLNet_ResolveHostname(address);

	int result = SDLNet_WaitUntilResolved(addr, -1);

	if (addr == NULL
		|| result != NET_RESOLUTION_RESOLVED)
	{
		printf("Error connecting - failed to resolve hostname: %s\n", SDL_GetError());
		return false;
	}

	sys_client->server_addr = addr;

	Logging_LogAll("Successfully resolved hostname!");

	// set up the reliable band. this tries to connect to the server and is used for auth, chat, and low-frequency things.
	sys_client->socket_reliable = SDLNet_CreateClient(sys_client->server_addr, sys_client->server_port);

	if (sys_client->socket_reliable == NULL)
	{
		printf("Error connecting - failed to create reliable socket: %s\n", SDL_GetError());
		return false;
	}

	//TODO: Varargs support logging
	printf("Using port %d for reliable updates\n", sys_client->server_port);

	// RAND_MAX = 32767 so duplicate 
	sys_client->client_port = rand() * 2;

	while (sys_client->client_port < NET_CLIENT_PORT_MIN
		|| sys_client->client_port > NET_CLIENT_PORT_MAX) sys_client->client_port = rand() * 2;

	printf("Using port %d for unreliable updates\n", sys_client->client_port);

	// set up the unreliable band. this doesn't actually try and connect to the server, it just fires shit off at the server
	// does it go? fuck if i know!!!
	sys_client->socket_unreliable = SDLNet_CreateDatagramSocket(NULL, sys_client->client_port);

	int connected = SDLNet_WaitUntilConnected(sys_client->socket_reliable, -1);

	// See if the reliable band connected
	if (connected != NET_CONNECTION_CONNECTED)
	{
		printf("Connection timed out: %s\n", SDL_GetError());
		return false;
	}

	// go
	sys_client_running = true;
	Logging_LogAll("Connection accepted!\n");
	return true;
}

void NET_ClientMain()
{
	// TODO: packet check
	while (sys_client_running)
	{

	}
}

void NET_ClientShutdown()
{

}