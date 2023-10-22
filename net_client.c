#pragma once
#include "net_client.h"

bool			sys_client_running;												// Is the client running?	
netclient_t*	sys_client;														// Current instance client object (cannot be the same as net_server)

void			Client_ReadReliableMessage(Uint8 buf[NET_MESSAGE_MAX_LENGTH]);	// Read client reliable message
void			Client_ReadUnreliableMessage(Uint8 buf[NET_MESSAGE_MAX_LENGTH]);// Read client unreliable message

void Client_Init()
{
	sys_client = malloc(sizeof(netclient_t));

	if (sys_client == NULL)
	{
		Logging_LogAll("Error initialising client - failed to allocate memory");
		return;
	}

	memset(sys_client, 0x00, sizeof(netclient_t));	//Shutup compiler
}

bool Client_Connect(char* address, Uint16 port)
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
	sys_client->name = "Player";

	int connected = SDLNet_WaitUntilConnected(sys_client->socket_reliable, -1);

	// See if the reliable band connected
	if (connected != NET_CONNECTION_CONNECTED)
	{
		printf("Connection timed out: %s\n", SDL_GetError());
		return false;
	}

	// this line is temp
	sys_client_running = true;
	Logging_LogAll("Connection accepted!\n");
	sys_client->connected = true;
	return true;
}

void Client_Main()
{
	// TODO: packet check
	while (sys_client_running)
	{
		Uint8 buf[NET_MESSAGE_MAX_LENGTH];

		Client_ReadReliableMessage(buf);

		// Read unreliable socket message

		/*
		if (NET_IncomingUnreliableMessage(sys_client->socket_reliable, buf, NET_MESSAGE_MAX_LENGTH) != NULL)
		{
			Client_ReadUnreliableMessage(buf);
		}
		*/
	}
}

void Client_ReadReliableMessage(Uint8 buf[NET_MESSAGE_MAX_LENGTH])
{
	Uint8 msg_id = NET_ReadByteReliable(sys_client->socket_reliable, &buf, 1);

	if (!last_msg_successful) return;

	// switch msg num
	switch (msg_id)
	{
		case msg_invalid:
			Logging_LogChannel("INVALID message received from server!", LogChannel_Fatal);
			break;
		case msg_auth_challenge: // send protocol version
			NET_WriteByteReliable(sys_client->socket_reliable, msg_auth_response);
			NET_WriteByteReliable(sys_client->socket_reliable, NET_PROTOCOL_VERSION);
			break;
		case msg_auth_clientinfo_request:
			NET_WriteStringReliable(sys_client->socket_reliable, sys_client->name);
			NET_WriteShortReliable(sys_client->socket_reliable, sys_client->client_port);
			break;
	}
}

void Client_ReadUnreliableMessage(Uint8 buf[NET_MESSAGE_MAX_LENGTH])
{

}

void Client_Disconnect()
{

}

void Client_Shutdown()
{

}