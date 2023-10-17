#pragma once
#include "core.h"

//
//	Util.h: Implements "general" util functions.
//

// Date utilities
void Util_DateGetCurrentString(char **finalArray);

// Enum utilities
bool Util_EnumHasFlag(Sint32 enumValue, Sint32 flag);

// String utilities
char* Util_StringFromInt(Sint32 num);
bool Util_StringFromBoolean(char* string);