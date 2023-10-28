#pragma once
#include "net_protocol.h"

char* net_message_names[] =
{
	"msg_invalid",
	"msg_auth_challenge",
	"msg_auth_response",
	"msg_auth_clientinfo_request",
	"msg_auth_clientinfo_response",
	"msg_disconnect",
};