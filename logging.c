#pragma once
#include "logging.h"
#include "Util.h"
#include "util_console.h"

void	Logging_Log(const char* text, LogChannel channel, va_list args);

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
	va_list	args;

	va_start(args, channel);

	Logging_Log(text, channel, args);
}

void Logging_LogAll(const char* text, ...)
{
	va_list args;

	va_start(args, text);

	Logging_Log(text, LogChannel_Message | LogChannel_Warning | LogChannel_Error | LogChannel_Fatal, args);
}

// yes this is required
void Logging_Log(const char* text, LogChannel channel, va_list args)
{
	if (!Util_EnumHasFlag(sys_logger->settings->channels, channel))
	{
		printf(u8"Log failed: 0x0003DEAD Tried to output to a closed log channel (%d). See logging.h.", channel);
		return;
	}

	char* date_buffer[32];
	char log_string_buffer[512 + 32];

	memset(&date_buffer, 0x00, sizeof date_buffer);
	memset(&log_string_buffer, 0x00, sizeof(log_string_buffer));

	Util_DateGetCurrentString(&date_buffer);

	if (strlen(text) > 512)
	{
		printf(u8"Log failed: 0x0004DEAD cannot log string above 512 bytes!");
		return;
	}

	// lop off the last character so it doesn't have a new line

	char* date_buffer_formatted = *date_buffer;
	date_buffer_formatted[strlen(date_buffer_formatted) - 1] = '\0';

	const char* prefix = "[";
	const char* date_suffix = "]: ";

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

	SDL_assert(date_buffer[0] != NULL);

	strcat_s(log_string_buffer, sizeof(log_string_buffer), prefix);
	strcat_s(log_string_buffer, sizeof(log_string_buffer), date_buffer_formatted);
	strcat_s(log_string_buffer, sizeof(log_string_buffer), date_suffix);
	strcat_s(log_string_buffer, sizeof(log_string_buffer), text);

	strcat_s(log_string_buffer, sizeof(log_string_buffer), suffix);

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

		vprintf(log_string_buffer, args);

		Util_ConsoleResetForegroundColor();
	}

	if (Util_EnumHasFlag(sys_logger->settings->source, LogSource_File))
	{
		vfprintf(sys_logger->handle, log_string_buffer, args);
	}

	va_end(args);
}

void Logging_Shutdown()
{
	Logger_destroy(sys_logger);
}