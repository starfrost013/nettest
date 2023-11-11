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

	bool	sign_on_started = false;						// Did the client start auth?
	bool	sign_on_completed = false;						// Did the client successfully auth?

	// start a timer
	while (SDL_GetTicks() < ticks + NET_AUTH_TIMEOUT
		&& !sign_on_completed)
	{
		switch (auth_phase)
		{
			case 0:
				Uint8 msg_id, protocol_version;

				msg_id = NET_ReadByteReliable(new_socket);

				if (msg_id == msg_auth_response)
				{
					sign_on_started = true;

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

					sign_on_completed = true;
					break;
				}
				continue;
		}	
	}

	// kill client
	if (!sign_on_completed)
	{
		SDLNet_DestroyStreamSocket(new_socket);
		// return true if there is no client there (sign_on_started == false and sign_on_completed = false)
		// return false if a client failed halfway through signon (sign_on_started == true and sign_on_completed = false)
		return !sign_on_started;
	}

	// add client
	int new_client_number = sys_server->num_clients;

	// check if a player that wasn't the most recently joined player left, so that slots aren't used permanently
	for (int client_number = 0; client_number < new_client_number; client_number++)
	{
		if (!sys_server->clients[client_number].signed_in) new_client_number = client_number;
	}

	client_t* new_client = &sys_server->clients[new_client_number];

	memset(new_client, 0x00, sizeof(client_t));

	new_client->socket_reliable = new_socket;		// Set reliable client
	new_client->port_unreliable = unreliable_port;	// Set unreliable port
	new_client->connected = true;					// This is only used on client side, but set it here anyway
	new_client->signed_in = true;					// Set to true
	new_client->name = net_username;				// Set up the username

	sys_server->num_clients++;

	Logging_LogChannel("User %s joined the game, using unreliable port %d!\n", LogChannel_Message, new_client->name, new_client->port_unreliable);
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
			Logging_LogChannel("FATAL - Error checking for new clients. Shutting down", LogChannel_Fatal);
			Server_Shutdown();
		}

		// check for reliable messages from clients
		for (Sint32 client_num = 0; client_num < sys_server->num_clients; client_num++)
		{
			client_t* client = &sys_server->clients[client_num];

			// make sure the client is actually connected
			if (client->signed_in)
			{
				Uint8 msg_id = NET_ReadByteReliable(client->socket_reliable);

				// skip client
				if (!msg_waiting) continue; 
				if (!last_socket_alive)
				{
					Server_DisconnectClient(client);
					continue; // bad, but we need to not run through a phantom loop
				}

				// at this point we have a client that actually sent us a message
				Logging_LogChannel(net_message_names[msg_id], LogChannel_Message);

				// switch msg num
				switch (msg_id)
				{
					case msg_invalid:
						Logging_LogChannel("INVALID message received from client!", LogChannel_Fatal);
						break;
					case msg_disconnect:
						Server_DisconnectClient(client);
						break;

				}
			}
			
			//todo: unreliable
		}

		// after processing, prevent it from being processed again by setting msg_waiting to false
		msg_waiting = false;
	}
}

void Server_DisconnectClient(client_t* client)
{
	Logging_LogChannel("Client disconnected: ", LogChannel_Message);
	Logging_LogChannel(NET_ReadStringReliable(client->socket_reliable), LogChannel_Message);
	// just set the client->signed_in to false
	client->signed_in = false;
	sys_server->num_clients--;
}

void Server_Shutdown()
{
	// disconnect all clients

	// sys_server->num_clients will fall as we disconnect clients so store it here
	int current_num_clients = sys_server->num_clients;

	for (Sint32 client_num = 0; client_num < current_num_clients; client_num++)
	{
		Server_DisconnectClient(&sys_server->clients[client_num]);
	}

	SDLNet_DestroyServer(sys_server->server);
}