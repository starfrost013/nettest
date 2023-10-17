#pragma once
#include "core.h"
#include "cmdline.h"
#include "net.h"

// Sys.h: Base defines
bool Sys_Init(int argc, char* argv[]);				// Initialises the core
void Sys_Shutdown();								// Shuts down the core. THIS CANNOT FAIL OR CAUSE AN EXCEPTION EVER!