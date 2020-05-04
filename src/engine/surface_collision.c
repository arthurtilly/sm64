#include <ultra64.h>

#include "sm64.h"
#include "game/level_update.h"
#include "game/debug.h"
#include "game/camera.h"
#include "game/mario.h"
#include "behavior_script.h"
#include "surface_collision.h"
#include "surface_load.h"
#include "game/object_list_processor.h"
#include "engine/math_util.h"

/**************************************************
 *                      WALLS                      *
 **************************************************/

/**
 * Iterate through the list of walls until all walls are checked and
 * have given their wall push.
 */

static s32 find_wall_collisions_from_list(struct SurfaceNode *surfaceNode,
                                          struct WallCollisionData *data, UNUSED u32 dynamic) {
    register f32 offset;
    register f32 radius = data->radius;
    register struct Surface *surf;
    register f32 x = data->x;
    register f32 y = data->y + data->offsetY;
    register f32 z = data->z;
    Vec3s v1,v2,v3;
    Vec3f n;
    s32 numCols = 0;

    // Max collision radius = 200
    if (radius > 200.0f) {
        radius = 200.0f;
    }

    // Stay in this loop until out of walls.
    while (surfaceNode != NULL) {
        surf = surfaceNode->surface;
        surfaceNode = surfaceNode->next;
        
        vec3f_copy(n,&surf->normal.x);
        vec3s_copy(v1,surf->vertex1);
        vec3s_copy(v2,surf->vertex2);
        vec3s_copy(v3,surf->vertex3);

        // Exclude a large number of walls immediately to optimize.
        if (y < (min_3(v1[1],v2[1],v3[1]) - 5) || y > (max_3(v1[1],v2[1],v3[1]) + 5)) {
            continue;
        }

        offset = n[0] * (x - v1[0]) + n[1] * (y - v1[1]) + n[2] * (z - v1[2]);

        if (offset < -radius || offset > radius) {
            continue;
        }

        //! (Quantum Tunneling) Due to issues with the vertices walls choose and
        //  the fact they are floating point, certain floating point positions
        //  along the seam of two walls may collide with neither wall or both walls.
        if (n[0] < -0.707f || n[0] > 0.707f) {
            if (n[0] > 0.0f) {
                if ((v1[1] - y) * (v1[2] - v2[2]) - (z - v1[2]) * (v2[1] - v1[1]) > 0.0f) {
                    continue;
                }
                if ((v2[1] - y) * (v2[2] - v3[2]) - (z - v2[2]) * (v3[1] - v2[1]) > 0.0f) {
                    continue;
                }
                if ((v3[1] - y) * (v3[2] - v1[2]) - (z - v3[2]) * (v1[1] - v3[1]) > 0.0f) {
                    continue;
                }
            } else {
                if ((v1[1] - y) * (v1[2] - v2[2]) - (z - v1[2]) * (v2[1] - v1[1]) < 0.0f) {
                    continue;
                }
                if ((v2[1] - y) * (v2[2] - v3[2]) - (z - v2[2]) * (v3[1] - v2[1]) < 0.0f) {
                    continue;
                }
                if ((v3[1] - y) * (v3[2] - v1[2]) - (z - v3[2]) * (v1[1] - v3[1]) < 0.0f) {
                    continue;
                }
            }
        } else {
            if (n[2] > 0.0f) {
                if ((v1[1] - y) * (v2[0] - v1[0]) - (v1[0] - x) * (v2[1] - v1[1]) > 0.0f) {
                    continue;
                }
                if ((v2[1] - y) * (v3[0] - v2[0]) - (v2[0] - x) * (v3[1] - v2[1]) > 0.0f) {
                    continue;
                }
                if ((v3[1] - y) * (v1[0] - v3[0]) - (v3[0] - x) * (v1[1] - v3[1]) > 0.0f) {
                    continue;
                }
            } else {
                if ((v1[1] - y) * (v2[0] - v1[0]) - (v1[0] - x) * (v2[1] - v1[1]) < 0.0f) {
                    continue;
                }
                if ((v2[1] - y) * (v3[0] - v2[0]) - (v2[0] - x) * (v3[1] - v2[1]) < 0.0f) {
                    continue;
                }
                if ((v3[1] - y) * (v1[0] - v3[0]) - (v3[0] - x) * (v1[1] - v3[1]) < 0.0f) {
                    continue;
                }
            }
        }

        // Determine if checking for the camera or not.
        if (gCheckingSurfaceCollisionsForCamera) {
            if (surf->flags & SURFACE_FLAG_NO_CAM_COLLISION) {
                continue;
            }
        } else {
            // Ignore camera only surfaces.
            if (surf->type == SURFACE_CAMERA_BOUNDARY) {
                continue;
            }

            // If an object can pass through a vanish cap wall, pass through.
            if (surf->type == SURFACE_VANISH_CAP_WALLS) {
                // If an object can pass through a vanish cap wall, pass through.
                if (gCurrentObject != NULL
                    && (gCurrentObject->activeFlags & ACTIVE_FLAG_MOVE_THROUGH_GRATE)) {
                    continue;
                }

                // If Mario has a vanish cap, pass through the vanish cap wall.
                if (gCurrentObject != NULL && gCurrentObject == gMarioObject
                    && (gMarioState->flags & MARIO_VANISH_CAP)) {
                    continue;
                }
            }
        }

        data->x += n[0] * (radius - offset);
        x += n[0] * (radius - offset);
        data->z += n[2] * (radius - offset);
        z += n[2] * (radius - offset);

        //! (Unreferenced Walls) Since this only returns the first four walls,
        //  this can lead to wall interaction being missed. Typically unreferenced walls
        //  come from only using one wall, however.
        if (data->numWalls < 4) {
            data->walls[data->numWalls++] = surf;
        }

        numCols++;
    }

    return numCols;
}

/**
 * Formats the position and wall search for find_wall_collisions.
 */
s32 f32_find_wall_collision(f32 *xPtr, f32 *yPtr, f32 *zPtr, f32 offsetY, f32 radius) {
    struct WallCollisionData collision;
    s32 numCollisions = 0;

    collision.offsetY = offsetY;
    collision.radius = radius;

    collision.x = *xPtr;
    collision.y = *yPtr;
    collision.z = *zPtr;

    collision.numWalls = 0;

    numCollisions = find_wall_collisions(&collision);

    *xPtr = collision.x;
    *yPtr = collision.y;
    *zPtr = collision.z;

    return numCollisions;
}

/**
 * Find wall collisions and receive their push.
 */
s32 find_wall_collisions(struct WallCollisionData *colData) {
    struct SurfaceNode *node;
    s32 numCollisions = 0;
    s16 x = colData->x;
    s16 z = colData->z;

    colData->numWalls = 0;

    if (x <= -LEVEL_BOUNDARY_MAX || x >= LEVEL_BOUNDARY_MAX) {
        return numCollisions;
    }
    if (z <= -LEVEL_BOUNDARY_MAX || z >= LEVEL_BOUNDARY_MAX) {
        return numCollisions;
    }

    node = gDynamicSurfaces.next;
    numCollisions += find_wall_collisions_from_list(node, colData, TRUE);

    node = (((gCurrentObject == gMarioObject) && (gMarioObject != NULL)) ? gStaticSurfacePartition[SURF_WALL].next : gStaticSurfaces.next);
    numCollisions += find_wall_collisions_from_list(node, colData, FALSE);

    // Increment the debug tracker.
    gNumCalls.wall += 1;

    return numCollisions;
}

/**************************************************
 *                     CEILINGS                    *
 **************************************************/

/**
 * Iterate through the list of ceilings and find the first ceiling over a given point.
 */
static struct Surface *find_ceil_from_list(struct SurfaceNode *surfaceNode, s32 x, s32 y, s32 z,
                                           f32 *pheight) {
    register struct Surface *surf;
    struct Surface *ceil = NULL;
    Vec3s v1, v2, v3;
    Vec3f n;

    ceil = NULL;

    // Stay in this loop until out of ceilings.
    while (surfaceNode != NULL) {
        surf = surfaceNode->surface;
        surfaceNode = surfaceNode->next;

        vec3f_copy(n,&surf->normal.x);
        vec3s_copy(v1,surf->vertex1);
        vec3s_copy(v2,surf->vertex2);
        vec3s_copy(v3,surf->vertex3);

        // Checking if point is in bounds of the triangle laterally.
        if ((v1[2] - z) * (v2[0] - v1[0]) - (v1[0] - x) * (v2[2] - v1[2]) > 0) {
            continue;
        }
        if ((v2[2] - z) * (v3[0] - v2[0]) - (v2[0] - x) * (v3[2] - v2[2]) > 0) {
            continue;
        }
        if ((v3[2] - z) * (v1[0] - v3[0]) - (v3[0] - x) * (v1[2] - v3[2]) > 0) {
            continue;
        }

        // Determine if checking for the camera or not.
        if (gCheckingSurfaceCollisionsForCamera != 0) {
            if (surf->flags & SURFACE_FLAG_NO_CAM_COLLISION) {
                continue;
            }
        }
        // Ignore camera only surfaces.
        else if (surf->type == SURFACE_CAMERA_BOUNDARY) {
            continue;
        }

        {
            f32 height;

            // Find the ceil height at the specific point.
            height = -(n[0] * (x - v1[0]) + n[2] * (z - v1[2]) - n[1] * v1[1]) / n[1];

            // Checks for ceiling interaction with a 78 unit buffer.
            //! (Exposed Ceilings) Because any point above a ceiling counts
            //  as interacting with a ceiling, ceilings far below can cause
            // "invisible walls" that are really just exposed ceilings.
            if (y - (height - -78.0f) > 0.0f) {
                continue;
            }
            
            if (height < *pheight) {
                *pheight = height;
                ceil = surf;
            }
        }
    }
    
    return ceil;
}

/**
 * Find the lowest ceiling above a given position and return the height.
 */
f32 find_ceil(f32 posX, f32 posY, f32 posZ, struct Surface **pceil) {
    struct Surface *ceil, *dynamicCeil;
    struct SurfaceNode *surfaceList;
    f32 height = 20000.0f;
    f32 dynamicHeight = 20000.0f;
    s16 x, y, z;

    //! (Parallel Universes) Because position is casted to an s16, reaching higher
    // float locations  can return ceilings despite them not existing there.
    //(Dynamic ceilings will unload due to the range.)
    x = (s16) posX;
    y = (s16) posY;
    z = (s16) posZ;
    *pceil = NULL;

    if (x <= -LEVEL_BOUNDARY_MAX || x >= LEVEL_BOUNDARY_MAX) {
        return height;
    }
    if (z <= -LEVEL_BOUNDARY_MAX || z >= LEVEL_BOUNDARY_MAX) {
        return height;
    }

    // Check for surfaces belonging to objects.
    surfaceList = gDynamicSurfaces.next;
    dynamicCeil = find_ceil_from_list(surfaceList, x, y, z, &dynamicHeight);

    // Check for surfaces that are a part of level geometry.
    surfaceList = (((gCurrentObject == gMarioObject) && (gMarioObject != NULL)) ? gStaticSurfacePartition[SURF_CEILING].next : gStaticSurfaces.next);
    ceil = find_ceil_from_list(surfaceList, x, y, z, &height);

    if (dynamicHeight < height) {
        ceil = dynamicCeil;
        height = dynamicHeight;
    }

    *pceil = ceil;

    // Increment the debug tracker.
    gNumCalls.ceil += 1;

    return height;
}

/**************************************************
 *                     FLOORS                      *
 **************************************************/

/**
 * Find the height of the highest floor below an object.
 */
f32 unused_obj_find_floor_height(struct Object *obj) {
    struct Surface *floor;
    f32 floorHeight = find_floor(obj->oPosX, obj->oPosY, obj->oPosZ, &floor);
    return floorHeight;
}

/**
 * Basically a local variable that passes through floor geo info.
 */
struct FloorGeometry sFloorGeo;

static u8 unused8038BE50[0x40];

/**
 * Return the floor height underneath (xPos, yPos, zPos) and populate `floorGeo`
 * with data about the floor's normal vector and origin offset. Also update
 * sFloorGeo.
 */
f32 find_floor_height_and_data(f32 xPos, f32 yPos, f32 zPos, struct FloorGeometry **floorGeo) {
    struct Surface *floor;
    f32 floorHeight = find_floor(xPos, yPos, zPos, &floor);

    *floorGeo = NULL;

    if (floor != NULL) {
        sFloorGeo.normalX = floor->normal.x;
        sFloorGeo.normalY = floor->normal.y;
        sFloorGeo.normalZ = floor->normal.z;
        sFloorGeo.originOffset = floor->originOffset;

        *floorGeo = &sFloorGeo;
    }
    return floorHeight;
}

/**
 * Iterate through the list of floors and find the first floor under a given point.
 */
static struct Surface *find_floor_from_list(struct SurfaceNode *surfaceNode, s32 x, s32 y, s32 z,
                                            f32 *pheight) {
    register struct Surface *surf;
    Vec3s v1,v2,v3;
    Vec3f n;
    f32 height;
    struct Surface *floor = NULL;

    // Iterate through the list of floors until there are no more floors.
    while (surfaceNode != NULL) {
        surf = surfaceNode->surface;
        surfaceNode = surfaceNode->next;
        
        vec3f_copy(n,&surf->normal.x);
        vec3s_copy(v1,surf->vertex1);
        vec3s_copy(v2,surf->vertex2);
        vec3s_copy(v3,surf->vertex3);

        if ((v1[2] - z) * (v2[0] - v1[0]) - (v1[0] - x) * (v2[2] - v1[2]) < 0) {
            continue;
        }
        if ((v2[2] - z) * (v3[0] - v2[0]) - (v2[0] - x) * (v3[2] - v2[2]) < 0) {
            continue;
        }
        if ((v3[2] - z) * (v1[0] - v3[0]) - (v3[0] - x) * (v1[2] - v3[2]) < 0) {
            continue;
        }

        // Determine if we are checking for the camera or not.
        if (gCheckingSurfaceCollisionsForCamera != 0) {
            if (surf->flags & SURFACE_FLAG_NO_CAM_COLLISION) {
                continue;
            }
        }
        // If we are not checking for the camera, ignore camera only floors.
        else if (surf->type == SURFACE_CAMERA_BOUNDARY) {
            continue;
        }

        // Find the height of the floor at a given location.
        height = -(n[0] * (x - v1[0]) + n[2] * (z - v1[2]) - n[1] * v1[1]) / n[1];
        
        // Checks for floor interaction with a 78 unit buffer.
        if (y - (height + -78.0f) < 0.0f) {
            continue;
        }

        if (height > *pheight) {
            *pheight = height;
            floor = surf;
        }
    }

    //! (Surface Cucking) Since only the first floor is returned and not the highest,
    //  higher floors can be "cucked" by lower floors.
    return floor;
}

/**
 * Find the height of the highest floor below a point.
 */
f32 find_floor_height(f32 x, f32 y, f32 z) {
    struct Surface *floor;

    f32 floorHeight = find_floor(x, y, z, &floor);

    return floorHeight;
}

/**
 * Find the highest floor under a given position and return the height.
 */
f32 find_floor(f32 xPos, f32 yPos, f32 zPos, struct Surface **pfloor) {
    struct Surface *floor, *dynamicFloor;
    struct SurfaceNode *surfaceList;

    f32 height = -11000.0f;
    f32 dynamicHeight = -11000.0f;

    //! (Parallel Universes) Because position is casted to an s16, reaching higher
    // float locations  can return floors despite them not existing there.
    //(Dynamic floors will unload due to the range.)
    s16 x = (s16) xPos;
    s16 y = (s16) yPos;
    s16 z = (s16) zPos;

    *pfloor = NULL;

    if (x <= -LEVEL_BOUNDARY_MAX || x >= LEVEL_BOUNDARY_MAX) {
        return height;
    }
    if (z <= -LEVEL_BOUNDARY_MAX || z >= LEVEL_BOUNDARY_MAX) {
        return height;
    }

    // Check for surfaces belonging to objects.
    surfaceList = gDynamicSurfaces.next;
    dynamicFloor = find_floor_from_list(surfaceList, x, y, z, &dynamicHeight);

    // Check for surfaces that are a part of level geometry.
    surfaceList = (((gCurrentObject == gMarioObject) && (gMarioObject != NULL)) ? gStaticSurfacePartition[SURF_FLOOR].next : gStaticSurfaces.next);
    floor = find_floor_from_list(surfaceList, x, y, z, &height);

    // To prevent the Merry-Go-Round room from loading when Mario passes above the hole that leads
    // there, SURFACE_INTANGIBLE is used. This prevent the wrong room from loading, but can also allow
    // Mario to pass through.
    if (!gFindFloorIncludeSurfaceIntangible) {
        //! (BBH Crash) Most NULL checking is done by checking the height of the floor returned
        //  instead of checking directly for a NULL floor. If this check returns a NULL floor
        //  (happens when there is no floor under the SURFACE_INTANGIBLE floor) but returns the height
        //  of the SURFACE_INTANGIBLE floor instead of the typical -11000 returned for a NULL floor.
        if (floor != NULL && floor->type == SURFACE_INTANGIBLE) {
            floor = find_floor_from_list(surfaceList, x, (s32)(height - 200.0f), z, &height);
        }
    } else {
        // To prevent accidentally leaving the floor tangible, stop checking for it.
        gFindFloorIncludeSurfaceIntangible = FALSE;
    }

    // If a floor was missed, increment the debug counter.
    if (floor == NULL) {
        gNumFindFloorMisses += 1;
    }

    if (dynamicHeight > height) {
        floor = dynamicFloor;
        height = dynamicHeight;
    }

    *pfloor = floor;

    // Increment the debug tracker.
    gNumCalls.floor += 1;

    return height;
}

/**************************************************
 *               ENVIRONMENTAL BOXES               *
 **************************************************/

/**
 * Finds the height of water at a given location.
 */
f32 find_water_level(f32 x, f32 z) {
    s32 i;
    s32 numRegions;
    s16 val;
    f32 loX, hiX, loZ, hiZ;
    f32 waterLevel = -11000.0f;
    s16 *p = gEnvironmentRegions;

    if (p != NULL) {
        numRegions = *p++;

        for (i = 0; i < numRegions; i++) {
            val = *p++;
            loX = *p++;
            loZ = *p++;
            hiX = *p++;
            hiZ = *p++;

            // If the location is within a water box and it is a water box.
            // Water is less than 50 val only, while above is gas and such.
            if (loX < x && x < hiX && loZ < z && z < hiZ && val < 50) {
                // Set the water height. Since this breaks, only return the first height.
                waterLevel = *p;
                break;
            }
            p++;
        }
    }

    return waterLevel;
}

/**
 * Finds the height of the poison gas (used only in HMC) at a given location.
 */
f32 find_poison_gas_level(f32 x, f32 z) {
    s32 i;
    s32 numRegions;
    UNUSED s32 unused;
    s16 val;
    f32 loX, hiX, loZ, hiZ;
    f32 gasLevel = -11000.0f;
    s16 *p = gEnvironmentRegions;

    if (p != NULL) {
        numRegions = *p++;

        for (i = 0; i < numRegions; i++) {
            val = *p;

            if (val >= 50) {
                loX = *(p + 1);
                loZ = *(p + 2);
                hiX = *(p + 3);
                hiZ = *(p + 4);

                // If the location is within a gas's box and it is a gas box.
                // Gas has a value of 50, 60, etc.
                if (loX < x && x < hiX && loZ < z && z < hiZ && val % 10 == 0) {
                    // Set the gas height. Since this breaks, only return the first height.
                    gasLevel = *(p + 5);
                    break;
                }
            }

            p += 6;
        }
    }

    return gasLevel;
}

/**************************************************
 *                      DEBUG                      *
 **************************************************/

/**
 * Finds the length of a surface list for debug purposes.
 */
static s32 surface_list_length(struct SurfaceNode *list) {
    s32 count = 0;

    while (list != NULL) {
        list = list->next;
        count++;
    }

    return count;
}

/**
 * Print the area,number of walls, how many times they were called,
 * and some allocation information.
 */
void debug_surface_list_info(f32 xPos, f32 zPos) {
    s32 numFloors = 0;
    s32 numWalls = 0;
    s32 numCeils = 0;

    s32 cellX = (xPos + LEVEL_BOUNDARY_MAX) / CELL_SIZE;
    s32 cellZ = (zPos + LEVEL_BOUNDARY_MAX) / CELL_SIZE;


    print_debug_top_down_mapinfo("area   %x", cellZ * 16 + cellX);

    // Names represent ground, walls, and roofs as found in SMS.
    print_debug_top_down_mapinfo("dg %d", numFloors);
    print_debug_top_down_mapinfo("dw %d", numWalls);
    print_debug_top_down_mapinfo("dr %d", numCeils);

    set_text_array_x_y(80, -3);

    print_debug_top_down_mapinfo("%d", gNumCalls.floor);
    print_debug_top_down_mapinfo("%d", gNumCalls.wall);
    print_debug_top_down_mapinfo("%d", gNumCalls.ceil);

    set_text_array_x_y(-80, 0);

    // listal- List Allocated?, statbg- Static Background?, movebg- Moving Background?
    print_debug_top_down_mapinfo("listal %d", gSurfaceNodesAllocated);
    print_debug_top_down_mapinfo("statbg %d", gNumStaticSurfaces);
    print_debug_top_down_mapinfo("movebg %d", gSurfacesAllocated - gNumStaticSurfaces);

    gNumCalls.floor = 0;
    gNumCalls.ceil = 0;
    gNumCalls.wall = 0;
}

/**
 * An unused function that finds and interacts with any type of surface.
 * Perhaps an original implementation of surfaces before they were more specialized.
 */
s32 unused_resolve_floor_or_ceil_collisions(s32 checkCeil, f32 *px, f32 *py, f32 *pz, f32 radius,
                                                   struct Surface **psurface, f32 *surfaceHeight) {
    f32 nx, ny, nz, oo;
    f32 x = *px;
    f32 y = *py;
    f32 z = *pz;
    f32 offset, distance;

    *psurface = NULL;

    if (checkCeil) {
        *surfaceHeight = find_ceil(x, y, z, psurface);
    } else {
        *surfaceHeight = find_floor(x, y, z, psurface);
    }

    if (*psurface == NULL) {
        return -1;
    }

    nx = (*psurface)->normal.x;
    ny = (*psurface)->normal.y;
    nz = (*psurface)->normal.z;
    oo = (*psurface)->originOffset;

    offset = nx * x + ny * y + nz * z + oo;
    distance = offset >= 0 ? offset : -offset;

    // Interesting surface interaction that should be surf type independent.
    if (distance < radius) {
        *px += nx * (radius - offset);
        *py += ny * (radius - offset);
        *pz += nz * (radius - offset);

        return 1;
    }

    return 0;
}
