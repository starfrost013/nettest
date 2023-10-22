#pragma once
#include "net.h"

//
// net_client.h : Client Core header
//

// Defines the client. It has a reliable and unreliable band for low and high frequency messgaes respectively,
// merging the best of both worlds.
typedef struct netclient_s
{
	SDLNet_StreamSocket*	socket_reliable;					//Socket used for communicating with the server. This is used for connecting and low-freq stuff.
	SDLNet_DatagramSocket*	socket_unreliable;					//Socket used for unreliably communicating with the server. This is used for movement updates and high-freq stuff.
	SDLNet_Address*			server_addr;						//Address of the server to connect to
	Uint16					server_port;						//Port of the server to connect to
	Uint16					client_port;						//Client port used for communicating with server on unreliable band.
	Sint32					timeout;							//Timeout. If the client isn't connected this amount of milliseconds after creation, it kills itself.
	char*					name;								//Name of this client
	SDL_bool				connected;							//Is this client connected?
	SDL_bool				signed_in;							//Is this client actually signed in? (Client Auth complete)
} netclient_t;

extern netclient_t*			sys_client;							//Current instance client - CANNOT be present at the same time as sys_server (see net_server.h)

extern bool					sys_client_running;					//Is the client running?

void	Client_Init();											//Initialises the client (sys_client)

bool	Client_Connect(char* address, Uint16 port);				//Connects the client to a server on address address using port port. If port 0 is provided it will use the default
																//specified by NET_SERVER_PORT define (port 9069 currently)
void	Client_Main();											//Client Main function

void	Client_Disconnect();									//Disconnects from the server

void	Client_Shutdown();										//Shutdown the client