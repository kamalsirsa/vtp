//
// Enumerations for the Enviro application.
//

#ifndef ENVIROENUMH
#define ENVIROENUMH

// States in which the application can be in:
enum AppState {
	AS_Initializing,	// still setting up
	AS_Orbit,			// full view of the earth
	AS_MovingIn,		// moving in toward a specific terrain
	AS_Terrain,			// inside a terrain
	AS_MovingOut,		// moving out to orbit
	AS_Error
};

enum NavType {
	NT_Normal,
	NT_Velo,
	NT_Grab,
	NT_Quake,
	NT_Pano
};

enum MouseMode
{
	MM_NAVIGATE,
	MM_SELECT,
	MM_FENCES,
	MM_ROUTES,
	MM_PLANTS,
	MM_INSTANCES,
	MM_MOVE,
	MM_LINEAR,
	MM_FLYROUTE,

	MM_NONE
};

#endif	// ENVIROENUMH

