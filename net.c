#pragma once
#include "net.h"
#include "net_client.h"
#include "net_server.h"

char			net_string_buffer[NET_STRING_MAX_LENGTH-1];				// Buffer to hold strings we read from net (like Quake!)

// In the case of no packets, we assume ZERO

void NET_Init()
{
	if (SDLNet_Init())
	{
		printf("Error initialising SDL_net: %s", SDL_GetError());
		exit(1);
	}
}


Uint8 NET_ReadByte(Uint8 buf[], int start)
{
	SDL_assert(ARRAY_SIZE(buf) < start + 1);

	return buf[start];
}


Sint16 NET_ReadShort(Uint8 buf[], int start)
{
	SDL_assert(ARRAY_SIZE(buf) < start + 2);

	return (buf[start] << 8) + buf[start + 1];
}

Sint32 NET_ReadInt(Uint8 buf[], int start)
{
	SDL_assert(ARRAY_SIZE(buf) < start + 4);

	return (buf[start] << 24) + (buf[start + 1] << 16) + (buf[start + 2] << 8) + buf[start + 3];
}


float NET_ReadFloat(Uint8 buf[], int start)
{
	SDL_assert(ARRAY_SIZE(buf) < start + 4);

	return (float)((buf[start] << 24) + (buf[start + 1] << 16) + (buf[start + 2] << 8) + buf[start + 3]);
}

char* NET_ReadString(Uint8 buf[], int start)
{
	// length is first byte
	Uint8 length = buf[start];

	// should never happen, so assert
	SDL_assert(ARRAY_SIZE(buf) < length);

	// we already checked start + len
	SDL_strlcpy(&net_string_buffer, &buf + start, start + length); 

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
	SDLNet_Datagram* msg;

	// dgram is NULL if there are no messages waiting
	if (!SDLNet_ReceiveDatagram(socket, &msg))
	{
		Logging_LogAll("Error - SDLNet_ReceiveDatagram failed!");
		Logging_LogAll(SDL_GetError());
		exit(1);
	}

	// as UDP datagrams can come from anywhere,
	// we make sure that if we're the client they're only coming from the server address 
	// and return NULL (meaning no messages) otherwise
	if (sys_mode == mode_client)
	{
		if (msg->addr != sys_client->server_addr) return NULL;
	}
	
	// size of (s/u)int* types is always guaranteed, regardless of architecture
	if (msg->buflen < expected_length)
	{
		exit(1);
	}

	return msg;
}

bool NET_IncomingReliableMessage(SDLNet_StreamSocket* socket, void* buf, int buflen)
{
	// MAX STRING Length 256
	Sint32 bytes_read = SDLNet_ReadFromStreamSocket(socket, &buf, buflen);

	if (bytes_read <= 0)
	{
		if (bytes_read == -1)
		{
			Logging_LogAll("Warning: ERROR reading from reliable socket %s", SDL_GetError());
		}

		return false;
	}

	return true; 
}

void NET_WriteByteReliable(SDLNet_StreamSocket* socket, Uint8 data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	Sint8 buf[1];
	memcpy(&buf, &data, 1);

	SDLNet_WriteToStreamSocket(socket, &buf, 1);
}

void NET_WriteByteUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, Uint8 data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	Sint8 buf[1];
	memcpy(&buf, &data, 1);

	SDLNet_SendDatagram(socket, addr, port, &buf, 1);
}

void NET_WriteShortReliable(SDLNet_StreamSocket* socket, Sint16 data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	Sint8 buf[2];
	memcpy(&buf, &data, 2);

	SDLNet_WriteToStreamSocket(socket, &buf, 2);
}

void NET_WriteShortUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, Sint16 data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	Sint8 buf[2];
	memcpy(&buf, &data, 2);

	SDLNet_SendDatagram(socket, addr, port, &buf, 2);
}

void NET_WriteIntReliable(SDLNet_StreamSocket* socket, Sint32 data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	Sint8 buf[4];
	memcpy(&buf, &data, 4);

	SDLNet_WriteToStreamSocket(socket, &buf, 4);
}

void NET_WriteIntUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, Sint32 data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	Sint8 buf[4];
	memcpy(&buf, &data, 4);

	SDLNet_SendDatagram(socket, addr, port, &buf, 4);
}

void NET_WriteFloatReliable(SDLNet_StreamSocket* socket, float data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	Sint8 buf[4];
	memcpy(&buf, &data, 4);

	SDLNet_WriteToStreamSocket(socket, &buf, 4);
}

void NET_WriteFloatUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, float data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	Sint8 buf[4];
	memcpy(&buf, &data, 4);

	SDLNet_SendDatagram(socket, addr, port, &buf, 4);
}

void NET_WriteStringReliable(SDLNet_StreamSocket* socket, char* data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	Sint8 buf[NET_STRING_MAX_LENGTH];

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

	buf[0] = strlen(data);

	SDL_strlcpy(&buf[1], data, NET_STRING_MAX_LENGTH - 1);

	SDLNet_WriteToStreamSocket(socket, &buf, NET_STRING_MAX_LENGTH);
}

void NET_WriteStringUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, char* data)
{
	if (sys_mode == mode_client
		&& !sys_client->connected) return;

	Sint8 buf[NET_STRING_MAX_LENGTH];

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