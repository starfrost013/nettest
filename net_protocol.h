#pragma once

#define NET_PROTOCOL_VERSION		1									// protocol version

//
// Network message types
// Send with NET_WriteByte*()
//
typedef enum netmessagetype_e
{
	msg_auth_challenge = 0,										//Server requests client authentication 
	//[byte] protocol version
	msg_auth_response = 1,										//Client sends protocol version [byte] protocol version

	msg_auth_clientinfo_request = 2,							//Request client info
																
	msg_auth_clientinfo_response = 3,							//[string] username [short] unreliable port
} netmessagetype;
