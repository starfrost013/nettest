#pragma once
#include "net.h"
#include "net_server.h"

bool			sys_server_running;											// Is the server running?
netserver_t*	sys_server;													// Current instance server object
bool			NET_ServerCheckForNewClients();								// Check for new clients
bool			NET_ServerAddClient(SDLNet_StreamSocket* next_client);		// Add a client - sets up its unreliable band and authenticates it

bool NET_InitServer()
{
	// Allocate a server.
	sys_server = malloc(sizeof(netserver_t));

	if (sys_server == NULL)
	{
		Logging_LogAll("Error initialising server - failed to allocate memory");
		return false;
	}

	// set up the port
	memset(sys_server, 0x00, sizeof(netserver_t));
	sys_server->port = NET_SERVER_PORT;
	sys_server->server = SDLNet_CreateServer(NULL, NET_SERVER_PORT);

	if (sys_server->server == NULL)
	{
		Logging_LogAll("Error initialising server - failed to create server reliable socket");
		return false;
	}

	// set the server to runnign
	sys_server_running = true;
	return true;
}

void NET_ServerMain()
{
	Logging_LogAll("The server is now up");

	while (sys_server_running)
	{
		// check 
		if (!NET_ServerCheckForNewClients())
		{
			Logging_LogAll("FATAL - Error checking for new clients");
			NET_ServerShutdown();
		}
	}
}

bool NET_ServerCheckForNewClients()
{
	SDLNet_StreamSocket* next_client;

	if (!SDLNet_AcceptClient(sys_server->server, &next_client))
	{
		if (next_client != NULL)
		{
			return NET_ServerAddClient(next_client);
		}

		return true;
	}

	Logging_LogAll("Error searching for clients!");
	return false;
}

bool NET_ServerAddClient(SDLNet_StreamSocket* next_client)
{
	Logging_LogAll("Client hello. Sending authentication...");

	NET_WriteByteReliable(next_client, msg_auth_challenge);
	NET_WriteByteReliable(next_client, NET_PROTOCOL_VERSION);

	//wait for the client to respond
	//this implements its own loop as there is not a client to iterate through
	Uint64	ticks = SDL_GetTicks();
	
	// Current phase of sign-on for this client
	int		auth_phase = 0;

	// Did the client successfully auth?
	bool	succeeded_fully = false;

	Uint8	buf_phase1[2];			// Phase1: Protocol version check
	Uint8	buf_phase2[64];			// Phase2: Client Information

	// start a timer
	while (SDL_GetTicks() < ticks + NET_AUTH_TIMEOUT)
	{
		switch (auth_phase)
		{
			case 0:
				if (NET_IncomingReliableMessage(next_client, &buf_phase1, 2))
				{
					if (buf_phase1[0] != msg_auth_response
						|| buf_phase1[1] != NET_PROTOCOL_VERSION)
					{
						Logging_LogAll("Client connection failed - wrong protocol version");
						break;
					}
					else
					{
						Logging_LogAll("Client connection - verified protocol version");

						// send client request
						NET_WriteByteReliable(next_client, msg_auth_clientinfo_request);
						auth_phase = 1;
					}
				}
			case 1:

				if (NET_IncomingReliableMessage(next_client, &buf_phase2, 64))
				{
					if (buf_phase2[0] != msg_auth_clientinfo_response)
					{
						Logging_LogChannel("Client did NOT send clientinfo!", LogChannel_Error);
						break;
					}
					else
					{
						char* net_username = NET_ReadString(&buf_phase2, 1);
						Sint16 port = NET_ReadShort(&buf_phase2, 1 + strlen(net_username) + 1);

						if (port < NET_CLIENT_PORT_MIN
							|| port > NET_CLIENT_PORT_MAX)
						{
							Logging_LogChannel("Client sent invalid port (must be in range 49152-65535)", LogChannel_Error);
						}
					}
				}

		}	
	}
}

void NET_ServerShutdown()
{

}