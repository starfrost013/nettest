#pragma once

//
// Mathematical Utilities for 3D Rendering
//

typedef float vec2_t[2];		// x,y...		for 2D stuff
typedef float vec3_t[3];		// x,y,z...		for 3D stuff
typedef float vec4_t[4];		// x,y,z,w...	for 3D rendering
typedef float mat22_t[2][2];	// 2x2 matrix	for 3D rendering
typedef float mat33_t[3][3];	// 3x3 matrix	for 3D rendering
typedef float mat44_t[4][4];	// 4x4 matrix	for 3D rendering

// Dot product
float Vector2_Dot(vec2_t a, vec2_t b);
float Vector3_Dot(vec3_t a, vec3_t b);
float Vector4_Dot(vec4_t a, vec4_t b);

// Cross product
vec2_t* Vector2_Cross(vec2_t a, vec2_t b, vec2_t* result);
vec3_t* Vector3_Cross(vec3_t a, vec3_t b, vec3_t* result);
vec4_t* Vector4_Cross(vec4_t a, vec4_t b, vec4_t* result);