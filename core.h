#pragma once
// Core Headers
// SDL libs and stdbool are used

#define VERSION_NUMBER		"1.0"
#define VERSION				"Version " VERSION_NUMBER
#define	BUILD_DATE			"Built " __DATE__ " " __TIME__

#define ARRAY_SIZE(x)		sizeof(x)/sizeof(x[0])

#include "SDL3_net/SDL_net.h"
#ifdef _MSC_VER
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#endif

#include "logging.h"
#include "Util.h"

//
// Holds modes
// A mode is a mode the game can operate in - simples :)
//
typedef enum mode_e
{
	mode_server = 0,

	mode_client = 1,
} mode;

mode sys_mode;