// nettest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "core.h"
#include "sys.h"
#include "cmdline.h"
#include "net_client.h"
#include "net_server.h"

int main(int argc, char* argv[])
{   
    // Temporary MAIN function

    if (!Sys_Init(argc, argv))
    {
        exit(1);
    }

    if (sys_mode == mode_server)
    {
        Server_Main();
    }
    else
    {
        // SDL3_net CRASHES if you try and connect to localhost
        // FILE BUG REPORT !!!
        if (Client_Connect("localhost", NET_SERVER_PORT))
        {
            Client_Main();
        }
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
