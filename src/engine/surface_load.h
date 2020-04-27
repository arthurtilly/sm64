#ifndef _SURFACE_LOAD_H
#define _SURFACE_LOAD_H

#include "types.h"

struct SurfaceNode
{
    struct SurfaceNode *next;
    struct Surface *surface;
};

enum SurfaceTypes
{
    SURF_FLOOR,
    SURF_CEILING,
    SURF_WALL
};

// Needed for bs bss reordering memes.
extern s32 unused8038BE90;

extern struct SurfaceNode gStaticSurfaces;
extern struct SurfaceNode gDynamicSurfaces;

extern struct SurfaceNode *sSurfaceNodePool;
extern struct Surface *sSurfacePool;
extern s16 sSurfacePoolSize;

void alloc_surface_pools(void);
void load_area_terrain(s16 index, s16 *data, s8 *surfaceRooms, s16 *macroObjects);
void clear_dynamic_surfaces(void);
void load_object_collision_model(void);

#endif
