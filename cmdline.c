#pragma once
#include "cmdline.h"

//
// cmdline.c : CMD line parser
//

void CmdLine_Parse(int argc, char* argv[])
{
	// default values
	sys_mode = mode_client;
	sys_client_graphics_mode = true;

	for (int arg_num = 0; arg_num < argc; arg_num++)
	{
		char* arg = argv[arg_num];

		// skip if null
		if (arg == NULL) continue; 

		if (!strcmp(arg, "-server"))
		{
			sys_mode = mode_server;
		}

		// run the client without graphics
		if (!strcmp(arg, "-nographics"))
		{
			sys_client_graphics_mode = false;
		}
	}
}