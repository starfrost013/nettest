#pragma once

#include "sys.h"

// Sys.c: Base code

bool Sys_Init(int argc, char* argv[])
{
    // Seed random
    srand(SDL_GetTicks());

    if (SDL_Init(SDL_INIT_EVERYTHING))
    {
        Logging_LogAll("Failed to initialise SDL");
        Logging_LogAll(SDL_GetError());
        return false;
    }

    Logging_Init();

    Logging_LogAll(VERSION);
    Logging_LogAll(BUILD_DATE);

    Logging_LogChannel("==== Initialising network ====", LogChannel_Message);

    NET_Init();

    Logging_LogChannel("Parsing command line", LogChannel_Message);
    CmdLine_Parse(argc, argv);

    if (sys_mode == mode_server)
    {
        Logging_LogChannel("NET_InitServer", LogChannel_Message);
        NET_InitServer();
    }
    else
    {
        Logging_LogChannel("NET_InitClient", LogChannel_Message);
        NET_InitClient();
    }

    return true;
}

void Sys_Shutdown()
{

}