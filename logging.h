#pragma once
#include "core.h"

typedef enum LogChannel_s
{
	LogChannel_Message = 1,

	LogChannel_Warning = 2,

	LogChannel_Error = 4,

	LogChannel_Fatal = 8,
} LogChannel;

typedef enum LogSource_s
{
	LogSource_Printf = 1,

	LogSource_File = 2,
} LogSource;

typedef struct LogSettings_s
{
	const char* fileName;
	LogChannel channels;
	LogSource source;
	bool keepOldLogs;
} LogSettings;

typedef struct Logger_s
{
	LogSettings* settings;
	FILE* handle;
	bool initialised;
} Logger;

bool Logging_Init();
void Logging_LogChannel(const char* text, LogChannel channel);
void Logging_LogAll(const char* text);
void Logging_Shutdown();

Logger* Logger_new();

void Logger_destroy(Logger* logger);

LogSettings* LogSettings_new(const char* fileName, LogChannel channels, LogSource source, bool keepOldLogs);

void LogSettings_destroy(LogSettings* settings);

Logger* sys_logger;