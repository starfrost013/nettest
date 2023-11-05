#pragma once
#include "core.h"

//
// Entity.h : Defines an entity.
//

typedef struct entity_s
{
	vec4_t		position;		// x,y,z,w (ignore z on 2d renderer)
	vec3_t		lines[];		// x,y,z per line or something. Idk we will load these as verts when we have a model format!

} entity_t;