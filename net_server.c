#pragma once
#include "net.h"
#include "net_server.h"

bool			sys_server_running;											// Is the server running?
netserver_t*	sys_server;													// Current instance server object
bool			Server_CheckForNewClients();								// Check for new clients
bool			Server_AddClient(SDLNet_StreamSocket* next_client);			// Add a client - sets up its reliable and unreliable bads and authenticates it

bool Server_Init()
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

bool Server_CheckForNewClients()
{
	SDLNet_StreamSocket* next_client;

	if (!SDLNet_AcceptClient(sys_server->server, &next_client))
	{
		if (next_client != NULL)
		{
			return Server_AddClient(next_client);
		}

		// no client waiting
		return true;
	}

	Logging_LogAll("Error searching for clients!");
	return false;
}

bool Server_AddClient(SDLNet_StreamSocket* new_socket)
{
	Logging_LogAll("Client hello. Sending authentication...");

	NET_WriteByteReliable(new_socket, msg_auth_challenge);
	NET_WriteByteReliable(new_socket, NET_PROTOCOL_VERSION);

	//wait for the client to respond
	//this implements its own loop as there is not a client to iterate through
	Uint64	ticks = SDL_GetTicks();
	
	// Current phase of sign-on for this client
	int		auth_phase = 0;

	// Username the client auth'd with
	char*	net_username;
	Uint16	unreliable_port;

	memset(&net_username, 0x00, sizeof(net_username));		// Shutup compiler

	// Did the client successfully auth?
	bool	succeeded_fully = false;

	// start a timer
	while (SDL_GetTicks() < ticks + NET_AUTH_TIMEOUT
		&& !succeeded_fully)
	{
		switch (auth_phase)
		{
			case 0:
				Uint8 msg_id, protocol_version;

				msg_id = NET_ReadByteReliable(new_socket);

				if (msg_id == msg_auth_response)
				{
					protocol_version = NET_ReadByteReliable(new_socket);

					// make sure the right protocol version was sent
					if (protocol_version != NET_PROTOCOL_VERSION)
					{
						Logging_LogAll("Client connection failed - wrong protocol version");
						break;
					}
					else
					{
						Logging_LogAll("Client connection - verified protocol version");

						// send client request
						NET_WriteByteReliable(new_socket, msg_auth_clientinfo_request);
						auth_phase = 1;
					}
				}
				continue;
			case 1:
				// read from client
				msg_id = NET_ReadByteReliable(new_socket);

				if (msg_waiting
					&& msg_id == msg_auth_clientinfo_response)
				{
					net_username = NET_ReadStringReliable(new_socket);

					unreliable_port = (Uint16)NET_ReadShortReliable(new_socket);

					// check unreliable is on a real prot
					if (unreliable_port < NET_CLIENT_PORT_MIN
						|| unreliable_port > NET_CLIENT_PORT_MAX)
					{
						Logging_LogChannel("Client sent invalid port (must be in range 49152-65535)", LogChannel_Error);
						break;
					}

					// check the client provided a username
					if (strlen(net_username) == 0)
					{
						Logging_LogChannel("Client sent a zero length username", LogChannel_Error);
						break;
					}

					succeeded_fully = true;
					break;
				}
				continue;
		}	
	}

	// kill client
	if (!succeeded_fully)
	{
		SDLNet_DestroyStreamSocket(new_socket);
		return false;
	}

	// add client
	netclient_t* new_client = &sys_server->clients[sys_server->num_clients];

	memset(new_client, 0x00, sizeof(netclient_t));

	new_client->socket_reliable = new_socket;		// Set reliable client
	new_client->port_unreliable = unreliable_port;	// Set unreliable port
	new_client->connected = true;					// This is only used on client side, but set it here anyway
	new_client->signed_in = true;					// Set to true
	new_client->name = net_username;				// Set up the username

	sys_server->num_clients++;

	printf("User %s joined the game, using unreliable port %d!\n", new_client->name, new_client->port_unreliable);
	return true; 
}

// Main server func
void Server_Main()
{
	Logging_LogAll("The server is now up");

	while (sys_server_running)
	{
		// check 
		if (!Server_CheckForNewClients())
		{
			Logging_LogAll("FATAL - Error checking for new clients");
			Server_Shutdown();
		}

		// check for reliable messages from clients
		for (Sint32 client_num = 0; client_num < sys_server->num_clients; client_num++)
		{
			netclient_t client = sys_server->clients[client_num];

			Uint8 msg_id = NET_ReadByteReliable(client.socket_reliable);

			if (msg_waiting)
			{
				Logging_LogChannel(net_message_names[msg_id], LogChannel_Message);

				// switch msg num
				switch (msg_id)
				{
					case msg_invalid:
						Logging_LogChannel("INVALID message received from client!", LogChannel_Fatal);
						break;
				}
			}
			
			//todo: unreliable
		}

		// after processing, prevent it from being processed again by setting msg_waiting to false
		msg_waiting = false;
	}
}

void Server_Shutdown()
{

}