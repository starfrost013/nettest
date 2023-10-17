#pragma once
#include "core.h"
#include "Util.h"

char gStringFromIntPtr[10];

void Util_DateGetCurrentString(char **finalArray)
{
	time_t currentTime;
	struct tm currentTimeInfoPtr;

	SDL_assert(finalArray != NULL);

	memset(&currentTimeInfoPtr, 0x00, sizeof(currentTimeInfoPtr));
	memset(&currentTime, 0x00, sizeof(currentTime));

	time(&currentTime);

	localtime_s(&currentTimeInfoPtr, &currentTime);

	*finalArray = asctime(&currentTimeInfoPtr);
}

bool Util_EnumHasFlag(Sint32 enumValue, Sint32 flag)
{
	return ((enumValue & flag) == flag);
}

bool Util_StringFromBoolean(char* string)
{
	SDL_assert(string != NULL);

	if (SDL_strcasecmp(string, "true"))
	{
		return true;
	}
	else if (SDL_strcasecmp(string, "false"))
	{
		return false;
	}

	// everything else is false
	return false;
}

char* Util_StringFromInt(Sint32 num)
{
	// 10 digit number as uint32_t max is 2147483647
	memset(&gStringFromIntPtr, 0x00, sizeof(char) * 10);
	sprintf_s(&gStringFromIntPtr, sizeof(char) * 10, "%d", num);
	
	return &gStringFromIntPtr;
}
