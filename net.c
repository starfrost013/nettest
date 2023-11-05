#pragma once
#include "net.h"
#include "net_client.h"
#include "net_server.h"

bool			msg_waiting;															// Is there a message waiting?
bool			last_socket_alive;															// Is the socket still alive?
Uint8			net_msg_buffer[NET_MESSAGE_MAX_LENGTH];									// Network Message buffer
char			net_string_buffer[NET_STRING_MAX_LENGTH-1];								// Buffer to hold strings we read from net (like Quake!)

void			NET_WriteDataReliable(SDLNet_StreamSocket* socket, int buflen);			// Write data to the reliable band
void			NET_WriteDataUnreliable(SDLNet_DatagramSocket* socket, int buflen);		// Write data to the unreliable band


// In the case of no packets, we assume ZERO

void NET_Init()
{
	if (SDLNet_Init())
	{
		printf("Error initialising SDL_net: %s", SDL_GetError());
		exit(1);
	}

	memset(&net_string_buffer, 0x00, sizeof(Uint8) * NET_STRING_MAX_LENGTH - 1);
	memset(&net_msg_buffer, 0x00, sizeof(Uint8) * NET_MESSAGE_MAX_LENGTH);
}

Uint8 NET_ReadByteReliable(SDLNet_StreamSocket* socket)
{
	if (NET_IncomingReliableMessage(socket, 1) 
		&& !msg_waiting)
	{
		return 0; // see last_msg_successful
	}

	return net_msg_buffer[0];
}

Sint16 NET_ReadShortReliable(SDLNet_StreamSocket* socket)
{
	if (NET_IncomingReliableMessage(socket, 2)
		&& !msg_waiting)
	{
		return 0;
	}

	// convert to native ordering on little-endian systems (network is always big-endian)
	return (net_msg_buffer[1] << 8) + net_msg_buffer[0];
}

Sint32 NET_ReadIntReliable(SDLNet_StreamSocket* socket)
{
	if (NET_IncomingReliableMessage(socket, 4)
		&& !msg_waiting)
	{
		return 0;
	}

	// convert to native ordering on little-endian systems (network is always big-endian)
	return (net_msg_buffer[3] << 24) + (net_msg_buffer[2] << 16) + (net_msg_buffer[1] << 8) + net_msg_buffer[0];
}

float NET_ReadFloatReliable(SDLNet_StreamSocket* socket)
{
	if (NET_IncomingReliableMessage(socket, 4)
		&& !msg_waiting)
	{
		return 0;
	}

	// convert to native ordering on little-endian systems (network is always big-endian)
	return (float)((net_msg_buffer[3] << 24) + (net_msg_buffer[2] << 16) + (net_msg_buffer[1] << 8) + net_msg_buffer[0]);
}

char* NET_ReadStringReliable(SDLNet_StreamSocket* socket)
{
	if (NET_IncomingReliableMessage(socket, 1)
		&& !msg_waiting)
	{
		return NULL;
	}

	// length is first byte
	Uint8 length = net_msg_buffer[0];

	// recv the rest of the string in a second message (this is in order to get around the fact that we don't know
	// the string length before receiving the string, and exploits the fact SDL3_net implements a continuous stream so we don't have to send two messages
	// in NET_WriteStringReliable


	if (NET_IncomingReliableMessage(socket, length)
		&& msg_waiting)
	{
		SDL_strlcpy(&net_string_buffer, &net_msg_buffer, length + 1);
	}

	return &net_string_buffer;
}

char* NET_ReadStringUnreliable(SDLNet_DatagramSocket* socket)
{
	// MAX STRING Length 256
	SDLNet_Datagram* msg = NET_IncomingUnreliableMessage(socket, NET_STRING_MAX_LENGTH);

	// length is first byte
	Uint8 length = msg->buf[0];

	// should never happen, so assert
	SDL_assert(msg->buflen < length);

	// length guaranteed by call to incomingunreliablemessage
	SDL_strlcpy(&net_string_buffer, msg->buf + 1, NET_STRING_MAX_LENGTH - 1);

	return &net_string_buffer;
}

SDLNet_Datagram* NET_IncomingUnreliableMessage(SDLNet_DatagramSocket* socket, int expected_length)
{
	msg_waiting = false;

	SDLNet_Datagram* msg;

	// dgram is NULL if there are no messages waiting
	if (!SDLNet_ReceiveDatagram(socket, &msg))
	{
		Logging_LogAll("Error - SDLNet_ReceiveDatagram failed!");
		Logging_LogChannel(SDL_GetError(), LogChannel_Fatal);
	}

	if (msg == NULL)
	{
		return NULL;
	}

	// as UDP datagrams can come from anywhere,
	// we make sure that if we're the client they're only coming from the server address 
	// and return NULL (meaning no messages) otherwise
	if (sys_mode == mode_client)
	{
		if (msg->addr != sys_client->server_address) return NULL;
	}

	// assert if size wrong
	SDL_assert(msg->buflen >= expected_length);

	msg_waiting = true;
	return msg;
}

bool NET_IncomingReliableMessage(SDLNet_StreamSocket* socket, int buflen)
{
	// set message waiting value to false, and "socket alive?" value to true
	msg_waiting = false;
	last_socket_alive = true;

	// MAX STRING Length 256
	Sint32 bytes_read = SDLNet_ReadFromStreamSocket(socket, &net_msg_buffer, buflen);

	if (bytes_read == -1)
	{
		//TODO: va_args
		Logging_LogAll("Error reading from reliable socket");
		Logging_LogAll(SDL_GetError());

		last_socket_alive = false;
		return false; // only returns false on failure
	}
	else if (bytes_read == 0)
	{
		// if we didn't get the message first time...

		if (!msg_waiting)
		{
			// ...wait until NET_MESSAGE_TIMEOUT to see if there is a message coming
			// TODO: MAKE THIS SHIT ASYNC (although it can't take that long after the first packet?)
			Uint64 ticks = SDL_GetTicks();

			while (SDL_GetTicks() < ticks + NET_PACKET_TIMEOUT
				&& bytes_read < buflen)
			{
				bytes_read = SDLNet_ReadFromStreamSocket(socket, &net_msg_buffer, buflen);
			}
		}

		// there is actually no message waiting
		if (bytes_read < buflen) return false;
	}

	// tell everyone else there is actually a message here
	msg_waiting = true;

#ifdef _DEBUG
	printf("NetDebug: recv %d bytes\n", bytes_read);

	for (int i = 0; i < bytes_read; i++)
	{
		printf("0x%X ", net_msg_buffer[i]);
	}

	printf("\n");

#endif

	return true; 
}

void NET_WriteDataReliable(SDLNet_StreamSocket* socket, int buflen)
{
	// disconnect if we failed to write data
	if (SDLNet_WriteToStreamSocket(socket, &net_msg_buffer, buflen) != 0)
	{
		Logging_LogChannel("Failed to write data to reliable socket.", LogChannel_Error);
		Logging_LogChannel(SDL_GetError(), LogChannel_Error);

		if (sys_mode == mode_client)
		{
			Client_Shutdown();
		}
		else
		{
			//TODO: Kick Client
			Server_Shutdown();
		}
	}

	if (SDLNet_WaitUntilStreamSocketDrained(socket, NET_PACKET_TIMEOUT) > 0)
	{
		Logging_LogChannel("Timed out", LogChannel_Error);
	}

#ifdef _DEBUG
	printf("NetDebug: sent %d bytes\n", buflen);

	for (int i = 0; i < buflen; i++)
	{
		printf("0x%X ", net_msg_buffer[i]);
	}

	printf("\n");

#endif
}

void NET_WriteByteReliable(SDLNet_StreamSocket* socket, Uint8 data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	memcpy(&net_msg_buffer, &data, 1);

	NET_WriteDataReliable(socket, 1);
}

void NET_WriteByteUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, Uint8 data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	memcpy(&net_msg_buffer, &data, 1);

	SDLNet_SendDatagram(socket, addr, port, &net_msg_buffer, 1);
}

void NET_WriteShortReliable(SDLNet_StreamSocket* socket, Sint16 data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	memcpy(&net_msg_buffer, &data, 2);

	NET_WriteDataReliable(socket, 2);
}

void NET_WriteShortUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, Sint16 data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	memcpy(&net_msg_buffer, &data, 2);

	SDLNet_SendDatagram(socket, addr, port, &net_msg_buffer, 2);
}

void NET_WriteIntReliable(SDLNet_StreamSocket* socket, Sint32 data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	memcpy(&net_msg_buffer, &data, 4);

	NET_WriteDataReliable(socket, 4);
}

void NET_WriteIntUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, Sint32 data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	memcpy(&net_msg_buffer, &data, 4);

	SDLNet_SendDatagram(socket, addr, port, &net_msg_buffer, 4);
}

void NET_WriteFloatReliable(SDLNet_StreamSocket* socket, float data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	memcpy(&net_msg_buffer, &data, 4);

	NET_WriteDataReliable(socket, 4);
}

void NET_WriteFloatUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, float data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	memcpy(&net_msg_buffer, &data, 4);

	SDLNet_SendDatagram(socket, addr, port, net_msg_buffer, 4);
}

void NET_WriteStringReliable(SDLNet_StreamSocket* socket, char* data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	if (data == NULL
		|| strlen(data) == 0)
	{
		Logging_LogAll("Warning: Tried to send a string of no, or NULL, length.\nThe string will not be sent.");
		return;
	}

	if (strlen(data) > NET_STRING_MAX_LENGTH - 1)
	{
		Logging_LogAll("Warning: Tried to send a string above 255 characters");
		return;
	}

	net_msg_buffer[0] = strlen(data);

	SDL_strlcpy(&net_msg_buffer[1], data, NET_STRING_MAX_LENGTH - 1);

	NET_WriteDataReliable(socket, strlen(data) + 1);
}

void NET_WriteStringUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, char* data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	Uint8 buf[NET_STRING_MAX_LENGTH];

	if (data == NULL
		|| strlen(data) == 0)
	{
		Logging_LogAll("Warning: Tried to send a string of no, or NULL, length.\nThe string will not be sent.");
		return;
	}

	if (strlen(data) > NET_STRING_MAX_LENGTH - 1)
	{
		Logging_LogAll("Warning: Tried to send a string above 255 characters in length!");
		return;
	}

	// first byte is length
	buf[0] = strlen(data);

	// copy to buffer
	SDL_strlcpy(&buf[1], data, NET_STRING_MAX_LENGTH - 1);

	// send string!
	SDLNet_SendDatagram(socket, addr, port, &buf, NET_STRING_MAX_LENGTH);
}

void NET_Shutdown()
{
	// Client and server are already shut down at this point
	SDLNet_Quit();
}