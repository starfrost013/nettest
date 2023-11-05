#pragma once
#include "core.h"

#define EUPHORIA_UTIL_CONSOLE_TERMINAL_COMMAND_PREFIX "\x1B["

void Util_ConsoleSetForegroundColor(ConsoleColor color)
{
	Sint32 finalColor = 0;

	finalColor = 30 + color;
	if (color >= CONSOLECOLOR_FIRST_BRIGHT) finalColor = 90 + color;

	// 10 (max) + 1 + 4 + 1 for safety
	char* finalString[17];
	memset(finalString, 0x00, sizeof(char) * 17);

	char* string = Util_StringFromInt(finalColor);

	strcat_s(finalString, 17, EUPHORIA_UTIL_CONSOLE_TERMINAL_COMMAND_PREFIX);
	strcat_s(finalString, 17, string);
	strcat_s(finalString, 17, "m");

	printf(finalString);
}

void Util_ConsoleSetBackgroundColor(ConsoleColor color)
{
	Sint32 finalColor = 0;

	finalColor = 40 + color;
	if (color >= CONSOLECOLOR_FIRST_BRIGHT) finalColor = 100 + color;

	// 10 (max) + 1 + 4 + 1 for safety
	char* finalString[17];
	memset(finalString, 0x00, sizeof(char) * 17);

	char* string = Util_StringFromInt(finalColor);

	strcat_s(finalString, 17, EUPHORIA_UTIL_CONSOLE_TERMINAL_COMMAND_PREFIX);
	strcat_s(finalString, 17, string);
	strcat_s(finalString, 17, "m");

	printf(finalString);
}

void Util_ConsoleResetForegroundColor()
{
	printf(EUPHORIA_UTIL_CONSOLE_TERMINAL_COMMAND_PREFIX "39m");
}

void Util_ConsoleResetBackgroundColor()
{
	printf(EUPHORIA_UTIL_CONSOLE_TERMINAL_COMMAND_PREFIX "49m");
}

void Util_ConsoleClearScreen()
{
	printf(EUPHORIA_UTIL_CONSOLE_TERMINAL_COMMAND_PREFIX "2J"); // Clear screen.
	printf(EUPHORIA_UTIL_CONSOLE_TERMINAL_COMMAND_PREFIX "3J"); // Clear scrollback.
	printf(EUPHORIA_UTIL_CONSOLE_TERMINAL_COMMAND_PREFIX "0;0H"); // Move to 0,0.
}