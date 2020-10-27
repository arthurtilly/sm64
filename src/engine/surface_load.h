#ifndef _SURFACE_LOAD_H
#define _SURFACE_LOAD_H

#include "types.h"

struct SurfaceNode
{
    struct SurfaceNode *next;
    struct Surface *surface;
};

enum
{
    SPATIAL_PARTITION_FLOORS,
    SPATIAL_PARTITION_CEILS,
    SPATIAL_PARTITION_WALLS
};

typedef struct SurfaceNode SpatialPartitionCell[3];

// Needed for bs bss reordering memes.
extern s32 unused8038BE90;

extern struct SurfaceNode gStaticSurfaces;
extern struct SurfaceNode gDynamicSurfaces;

extern SpatialPartitionCell gStaticSurfacePartition;

extern struct SurfaceNode *sSurfaceNodePool;
extern struct Surface *sSurfacePool;
extern s16 sSurfacePoolSize;

void alloc_surface_pools(void);
void load_area_terrain(s16 index, s16 *data, s8 *surfaceRooms, s16 *macroObjects);
void clear_dynamic_and_transformed_surfaces(void);
void create_transformed_surfaces(Vec3f);
void load_object_collision_model(void);

#endif
