//
// Light.h
//

#define TERRAIN_AMBIENT		0.3f
#define TERRAIN_DIFFUSE		0.6f
#define TERRAIN_EMISSIVE	0.1f

#define TREE_AMBIENT		0.8f

#if VTLIB_OPENSG
// A workaround hack for OpenSG: "in order to enable correct lighting for
//  the plants, one needs to set the diffuse component to a value >0"
#define TREE_DIFFUSE		0.8f
#else
#define TREE_DIFFUSE		0.0f
#endif

#define TREE_EMISSIVE		0.2f

