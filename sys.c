#pragma once

#include "sys.h"
#include "net_client.h"
#include "net_server.h"

// Sys.c: Base code

bool Sys_Init(int argc, char* argv[])
{
    // Seed random, should be random enough 
    srand(time(NULL));

    // Init SDL
    if (SDL_Init(SDL_INIT_EVERYTHING))
    {
        Logging_LogAll("Failed to initialise SDL");
        Logging_LogAll(SDL_GetError());
        return false;
    }

    // Log sign-on information
    Logging_Init();

    Logging_LogAll(VERSION);
    Logging_LogAll(BUILD_DATE);

    Logging_LogChannel("==== Initialising network ====", LogChannel_Message);

    // Initialise network
    NET_Init();

    // Parse command lin
    Logging_LogChannel("Parsing command line", LogChannel_Message);
    CmdLine_Parse(argc, argv);

    // Initialise server.
    if (sys_mode == mode_server)
    {
        Logging_LogChannel("Server_Init", LogChannel_Message);
        Server_Init();
    }
    else
    {
        Logging_LogChannel("Client_Init", LogChannel_Message);
        Client_Init();
    }

    return true;
}

void Sys_Shutdown()
{
    // quit
    SDL_Quit();
}