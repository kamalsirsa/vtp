
#ifndef TERRAINERRH
#define TERRAINERRH

// Possible errors returned from vtTerrain::CreateScene
#define TERRAIN_ERROR_NOTFOUND		1	// file not found
#define TERRAIN_ERROR_NOREGULAR		2	// counldn't create regular terrain
#define TERRAIN_ERROR_NOTPOWER2		3	// size wasn't legal for CLOD
#define TERRAIN_ERROR_NOMEM			4	// not enough memory
#define TERRAIN_ERROR_LODFAILED		5	// couldn't create LOD terrain

#endif
