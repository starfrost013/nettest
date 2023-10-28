#pragma once
#include "logging.h"
#include "Util.h"
#include "util_console.h"

Logger* Logger_new()
{
	Logger* logger = (Logger*)malloc(sizeof(Logger));

	SDL_assert(logger != NULL);
	if (logger == NULL) return NULL;		//Shutup MSVC warnings 

	memset(logger, 0x00, sizeof logger);

	//todo: allow this to be configured in settings
	logger->settings = LogSettings_new(u8"latest.log", LogChannel_Message | LogChannel_Warning | LogChannel_Error | LogChannel_Fatal, 
		LogSource_Printf | LogSource_File, false);
	logger->initialised = true;
	logger->handle = malloc(sizeof(FILE));

	SDL_assert(logger->handle != NULL);
	if (logger->handle == NULL) return NULL;		//Shutup MSVC warnings 

	memset(logger->handle, 0x00, sizeof(FILE));

	if (Util_EnumHasFlag(logger->settings->source, LogSource_File))
	{
		errno_t error = fopen_s(&(logger->handle), logger->settings->fileName, "w+");

		// TODO: delete old logs
		char* errorString[10];
		memset(&errorString, 0x00, sizeof(char) * 10);

		strerror_s(errorString, 10, error);

		if (error != 0)
		{
			printf(u8"Log failed: 0x0002DEAD Error opening logfile %s: %s\n", logger->settings->fileName, *errorString);
			return NULL;
		}
	}

	return logger;
}

void Logger_destroy(Logger* logger)
{
	fclose(logger->handle);
	free(logger->handle);
	LogSettings_destroy(logger->settings);
	free(logger);
}

LogSettings* LogSettings_new(const char* fileName, LogChannel channels, LogSource source, bool keepOldLogs)
{
	SDL_assert(fileName != NULL);

	LogSettings* settings = (LogSettings*)malloc(sizeof(LogSettings));

	SDL_assert(settings != NULL);
	if (settings == NULL) return NULL;		//Shutup MSVC warnings 

	memset(settings, 0x00, sizeof settings);
	settings->fileName = fileName;
	settings->channels = channels;
	settings->source = source;
	settings->keepOldLogs = keepOldLogs;
	return settings;
}

void LogSettings_destroy(LogSettings* settings)
{
	free(settings);
}

bool Logging_Init()
{
	sys_logger = Logger_new();

	return (sys_logger != NULL);
}

void Logging_LogChannel(const char* text, LogChannel channel, ...)
{
	if (!Util_EnumHasFlag(sys_logger->settings->channels, channel))
	{
		printf(u8"Log failed: 0x0003DEAD Tried to output to a closed log channel (%d). See logging.h.", channel);
		return; 
	}

	char* dateBuffer[32];
	char textBuffer[512];
	char logStringBuffer[512 + 32];

	memset(&dateBuffer, 0x00, sizeof dateBuffer);
	memset(&textBuffer, 0x00, sizeof(textBuffer));
	memset(&logStringBuffer, 0x00, sizeof(logStringBuffer));

	Util_DateGetCurrentString(&dateBuffer);

	if (strlen(text) > 512)
	{
		printf(u8"Log failed: 0x0004DEAD cannot log string above 512 bytes!");
		return;
	}

	// lop off the last character so it doesn't have a new line

	char* finalDateBuffer = *dateBuffer;
	finalDateBuffer[strlen(finalDateBuffer) - 1] = '\0';

	const char* prefix = "[";
	const char* dateSuffix = "]: ";

	// print separate colours
	switch (channel)
	{
		case LogChannel_Warning:
			prefix = "[WARNING] [";
			break;
		case LogChannel_Error:
			prefix = "[ERROR] [";
			break;
		case LogChannel_Fatal:
			prefix = "[FATAL] [";
			break;
	}

	const char* suffix = " \n";

	SDL_assert(dateBuffer[0] != NULL);

	strcat_s(logStringBuffer, sizeof(logStringBuffer), prefix);
	strcat_s(logStringBuffer, sizeof(logStringBuffer), finalDateBuffer);
	strcat_s(textBuffer, sizeof(textBuffer), text);
	strcat_s(logStringBuffer, sizeof(logStringBuffer), dateSuffix);
	strcat_s(logStringBuffer, sizeof(logStringBuffer), textBuffer);
	strcat_s(logStringBuffer, sizeof(logStringBuffer), suffix);

	if (Util_EnumHasFlag(sys_logger->settings->source, LogSource_Printf))
	{	
		switch (channel)
		{
			case LogChannel_Warning:
				Util_ConsoleSetForegroundColor(ConsoleColor_Yellow);
				break;
			case LogChannel_Error:
			case LogChannel_Fatal:
				Util_ConsoleSetForegroundColor(ConsoleColor_Red);
				break;
		}

		printf(logStringBuffer);

		Util_ConsoleResetForegroundColor();
	}

	if (Util_EnumHasFlag(sys_logger->settings->source, LogSource_File))
	{
		fwrite(logStringBuffer, strlen(logStringBuffer), 1, sys_logger->handle);
	}
}

void Logging_LogAll(const char* text)
{
	Logging_LogChannel(text, LogChannel_Message | LogChannel_Warning | LogChannel_Error | LogChannel_Fatal);
}

void Logging_Shutdown()
{
	Logger_destroy(sys_logger);
}