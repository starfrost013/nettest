#pragma once
#include "render.h"

renderer_t*			sys_renderer;							// SDL renderer struct
SDL_Texture*		sys_canvas;								// Canvas for 2D/Soft3D Renderer

bool Render_Init()
{
	// Initialise Complicated Directmoron Layer
	sys_renderer = (renderer_t*)malloc(sizeof(renderer_t));

	SDL_assert(sys_renderer != NULL);
	if (sys_renderer == NULL) return false; // shutup compiler

	// ONLY SOFTWARE 3D IS IMPLEMENTED!
	memset(sys_renderer, 0x00, sizeof(sys_renderer));
	
	// hardcode for now
	sys_renderer->type = renderer_3d_soft;
	sys_renderer->window_width = 1024;
	sys_renderer->window_height = 768;
	sys_renderer->title = "Complicated DirectMoron Layer Window";

	Logging_LogChannel("Creating SDL window and renderer device", LogChannel_Message);

	SDL_CreateWindowAndRenderer(sys_renderer->window_width, sys_renderer->window_height, 0, &sys_renderer->window_ptr, &sys_renderer->renderer_ptr);

	//todo: handle flags

	if (!sys_renderer->window_ptr
		|| !sys_renderer->renderer_ptr)
	{
		Logging_LogChannel("Failed to initialise SDL3 window: %s", LogChannel_Fatal, SDL_GetError());
		return false;
	}

	SDL_SetWindowTitle(sys_renderer->window_ptr, sys_renderer->title);

	// todo: create sys_canvas
	if (sys_renderer->type == renderer_2d 
		|| sys_renderer->type == renderer_3d_soft)
	{
		Logging_LogChannel("Soft3D: Creating SDL canvas (size: %dx%d)", LogChannel_Message, sys_renderer->window_width, sys_renderer->window_height);
		// ARGB or rgba?
		sys_canvas = SDL_CreateTexture(sys_renderer->renderer_ptr, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, sys_renderer->window_width, sys_renderer->window_height);
		
		if (sys_canvas == NULL)
		{
			Logging_LogChannel("Failed to create software 3D renderer canvas: %s", LogChannel_Fatal, SDL_GetError());
			return false;
		}

		return true; 
	}
	else
	{
		Logging_LogChannel("The current renderer is not implemented!", LogChannel_Warning);
	}

	// just return true in the case of non-implemented renderer
	return true; 
}

void Render_Shutdown()
{
	SDL_DestroyRenderer(sys_renderer->renderer_ptr);
}