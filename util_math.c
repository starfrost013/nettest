#pragma once
#include "util_math.h"

float Vector2_Dot(vec2_t a, vec2_t b)
{
	return (a[0] * b[0]) + (a[1] * b[1]);
}

float Vector3_Dot(vec3_t a, vec3_t b)
{
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

float Vector4_Dot(vec4_t a, vec4_t b)
{
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]) + (a[3] * b[3]);
}