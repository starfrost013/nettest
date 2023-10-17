#pragma once
#include "core.h"

// Net.h: "base" networking code, shared between client and server
// Address resolution defines
#define	NET_RESOLUTION_FAILED		-1
#define NET_RESOLUTION_RESOLVING	0
#define NET_RESOLUTION_RESOLVED		1

// Connection defines
#define	NET_CONNECTION_FAILED		-1
#define NET_CONNECTION_CONNECTING	0
#define NET_CONNECTION_CONNECTED	1

// Port used for reliable talking between server-client
#define NET_SERVER_PORT				9069						

// TCP/IP Transient ports used for unreliable band talking (movement updates)
#define NET_CLIENT_PORT_MIN			49152
#define NET_CLIENT_PORT_MAX			65536

// actual length 255 (1 byte for length)
#define NET_STRING_MAX_LENGTH		256							

#define NET_MESSAGE_MAX_LENGTH		512

// For code organisation reasons
#include "net_protocol.h"										

//
// Initialises NET subsystem
//
void	NET_Init();	

//
// Read functions 
//
Uint8	NET_ReadByte(Uint8 buf[], int start);
Sint16	NET_ReadShort(Uint8 buf[], int start);
Sint32	NET_ReadInt(Uint8 buf[], int start);
float	NET_ReadFloat(Uint8 buf[], int start);
char*	NET_ReadString(Uint8 buf[], int start);

//
// Reliable band (high-freq) functions
// 

void	NET_WriteByteReliable(SDLNet_StreamSocket* socket, Uint8 data);
void	NET_WriteShortReliable(SDLNet_StreamSocket* socket, Sint16 data);
void	NET_WriteIntReliable(SDLNet_StreamSocket* socket, Sint32 data);
void	NET_WriteFloatReliable(SDLNet_StreamSocket* socket, float data);
void	NET_WriteStringReliable(SDLNet_StreamSocket* socket, char* data);

//
// Unreliable band (high-freq) functions
//
void	NET_WriteByteUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, Uint8 data);
void	NET_WriteShortUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, Sint16 data);
void	NET_WriteIntUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, Sint32 data);
void	NET_WriteFloatUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, float data);
void	NET_WriteStringUnreliable(SDLNet_DatagramSocket* socket, SDLNet_Address* addr, Uint16 port, char* data);

bool				NET_IncomingReliableMessage(SDLNet_StreamSocket* socket, void* buf, int buflen);		//Checks for incoming reliable message. Returns TRUE if successful.
SDLNet_Datagram*	NET_IncomingUnreliableMessage(SDLNet_DatagramSocket* socket, int expected_length);		//Checks for incoming unreliable messages. Returns NULL if no messages, 
																											//a pointer to a dgram containing the message if there are messages waiting

//
// Shutdown
//
void	NET_Shutdown();