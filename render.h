#pragma once
#include "core.h"

//
// render.h
// This defines the shared structures used by every renderer.
//

typedef struct color4_s
{
	float r;
	float g;
	float b;
	float a;
} color4_t;

// Renderer type enum.
typedef enum renderer_type_e
{
	renderer_3d_soft = 0,						// Software 3D
	renderer_3d_opengl = 1,						// OpenGL 3D
	renderer_2d = 2,							// 2D
} renderer_type;

// Renderer struct!
typedef struct renderer_s
{
	renderer_type		type;
	Uint32				window_width;
	Uint32				window_height;
	const char*			title;
	SDL_Window*			window_ptr;
	SDL_Renderer*		renderer_ptr;
} renderer_t;

// Functions
bool Render_Init();								// Initialises SDL3 window
void Render_Shutdown();							// Shuts down SDL3 window

// Globals

extern renderer_t*		sys_renderer;			// Main renderer struct shared across all three renderers.

// 2D, Software 3D: Texture that gets written to and shoved into the GPU every frame. Not used in OpenGL 3d  - MUST BE DESTROYED, RECREATED AND RESIZED WHEN THE WINDOW SIZE CHANGES!
extern SDL_Texture*		sys_canvas;				