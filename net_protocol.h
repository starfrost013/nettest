#pragma once

#define NET_PROTOCOL_VERSION		1							// protocol version

//
// Network message types
// Send with NET_WriteByte*()
//
typedef enum netmessagetype_e
{
	msg_invalid = 0,											//Not a message. Used to check if the buffer was cleared

	msg_auth_challenge = 1,										//Server requests client authentication 
	//[byte] protocol version
	msg_auth_response = 2,										//Client sends protocol version [byte] protocol version

	msg_auth_clientinfo_request = 3,							//Request client info
																
	msg_auth_clientinfo_response = 4,							//[string] username [short] unreliable port

	msg_disconnect = 5,											//[string] reason. Disconnects
} netmessagetype;

// Network Message names for debug
extern char* net_message_names[];