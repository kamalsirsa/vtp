//
// Name:	 app.cpp
// Purpose:  Example GLUT/vtlib application.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "SDL.h"
#ifdef __FreeBSD__
#  include <ieeefp.h>
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"

class App
{
public:
	bool CreateScene();

	void videosettings(bool same_video_mode, bool fullscreen);
	void display();
	void run();
	int main();
	void process_mouse_button(const SDL_Event &event);
	void process_mouse_motion(const SDL_Event &event);
	void process_event(const SDL_Event &event);
	void process_events();
};


//
// Initialize the SDL display.  This code originated from another project,
// so i am not sure of the details, but it should work well on each
// platform.
//
void App::videosettings(bool same_video_mode, bool fullscreen)
{
	int width, height;

	if ( ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) == -1 ) )
	{
		std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
		exit( -1 );
	}
	atexit( SDL_Quit );

	SDL_VideoInfo const * info = SDL_GetVideoInfo();
	if( !info )
	{
		std::cerr << "Video query failed: " << SDL_GetError( ) << std::endl;
		exit( -1 );
	}

	int num_modes;
	SDL_Rect ** modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
	if ( (modes != NULL) && (modes != (SDL_Rect **)-1) )
	{
		for (num_modes = 0; modes[num_modes]; num_modes++);
		if ( same_video_mode )
		{
			// EEERRR should get the surface and use its parameters
			width = modes[0]->w;
			height = modes[0]->h;
		}
		else
		{
			for ( int i=num_modes-1; i >= 0; i-- )
			{
				width = modes[i]->w;
				height = modes[i]->h;
				if ( (modes[i]->w >= 1024) && (modes[i]->h >= 768) ) {
					break;
				}
			}
		} 
	}
	else
	{
		std::cerr << "Video list modes failed: " << SDL_GetError( ) << std::endl; 
		exit( -1 );
	}

	std::cerr << width << " " << height << std::endl;
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	int flags = SDL_OPENGL | SDL_DOUBLEBUF;

	if (fullscreen)
		flags |= SDL_FULLSCREEN;
	if ( info->hw_available )
		flags |= SDL_HWSURFACE;
	else
		flags |= SDL_SWSURFACE;

	if ( info->blit_hw )
		flags |= SDL_HWACCEL;

	if( SDL_SetVideoMode( width, height, 16, flags ) == 0 )
	{
		std::cerr << "Video mode set failed: " << SDL_GetError( ) << std::endl; 
		exit( -1 );
	}
	// Tell the SDL output size to vtlib
	vtGetScene()->SetWindowSize(width, height);
}

//--------------------------------------------------------------------------

//
// Create the 3d scene: call vtlib to load the terrain and prepare for
// user interaction.
//
bool App::CreateScene()
{
	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();
	pScene->Init();

	// Set the global data path
	vtStringArray paths;
	paths.push_back(vtString("Data/"));
	vtTerrain::SetDataPath(paths);

	// Look up the camera
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetHither(10);
	pCamera->SetYon(100000);

	// Create a new terrain scene.  This will contain all the terrain
	// that are created.
	vtTerrainScene *ts = new vtTerrainScene();
	vtGroup *pTopGroup = ts->BeginTerrainScene();

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	// Create a new vtTerrain, read its paramters from a file
	vtTerrain *pTerr = new vtTerrain();
	pTerr->SetParamFile("Data/Simple.ini");

	// Add the terrain to the scene, and contruct it
	ts->AppendTerrain(pTerr);
	if (!pTerr->CreateScene())
	{
		printf("Terrain creation failed.");
		return false;
	}
	ts->SetTerrain(pTerr);

	// Create a navigation engine to move around on the terrain
	// Flight speed is 400 m/frame
	// Height over terrain is 100 m
	vtTerrainFlyer *pFlyer = new vtTerrainFlyer(400, 100, true);
	pFlyer->SetTarget(pCamera);
	pFlyer->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pFlyer);

	return true;
}

void App::display()
{
#if !VTLIB_PSM
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vtGetScene()->DoUpdate();

	SDL_GL_SwapBuffers();
#endif
}

void App::process_mouse_button(const SDL_Event &sdle)
{
	// turn SDL mouse button event into a VT mouse event
	vtMouseEvent event;
	event.type = (sdle.button.type == SDL_MOUSEBUTTONDOWN) ? VT_DOWN : VT_UP;

	if (sdle.button.button == 1)
		event.button = VT_LEFT;
	else if (sdle.button.button == 2)
		event.button = VT_MIDDLE;
	else if (sdle.button.button == 3)
		event.button = VT_RIGHT;

	event.flags = 0;
	event.pos.Set(sdle.button.x, sdle.button.y);

	vtGetScene()->OnMouse(event);
}

void App::process_mouse_motion(const SDL_Event &sdle)
{
	// turn SDL mouse move event into a VT mouse event
	vtMouseEvent event;
	event.type = VT_MOVE;
	event.button = VT_NONE;
	event.flags = 0;
	event.pos.Set(sdle.motion.x, sdle.motion.y);

	vtGetScene()->OnMouse(event);
}

void App::process_event(const SDL_Event &event)
{
	int key;

	switch( event.type )
	{
		case SDL_QUIT:
			exit(0);
		case SDL_KEYDOWN:
			break;
		case SDL_KEYUP:
			// turn SDL key event into a VT mouse event
			key = event.key.keysym.sym;
			if ( key == 27 /* ESC */ || key == 'q' || key == 'Q' )
				exit(0);
			vtGetScene()->OnKey(key, 0);
			break;
		case SDL_MOUSEMOTION:
			process_mouse_motion(event);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			process_mouse_button(event);
			break;
		case SDL_VIDEORESIZE:
			// Tell vtlib
			vtGetScene()->SetWindowSize(event.resize.w, event.resize.h);
			break;
	}
}

void App::process_events()
{
	SDL_Event event;

	while ( SDL_PollEvent( &event ) )
		process_event(event);
}

void App::run()
{
	while ( true )
	{
		display();			// draw scene
		process_events();	// handle user events
	}
}

/*
  The works.
*/
int App::main()
{
#ifdef __FreeBSD__
	/*  FreeBSD is more stringent with FP ops by default, and OSG is	*/
	/*	doing silly things sqrt(Inf) (computing lengths of MAXFLOAT		*/
	/*	and NaN Vec3's).   This turns off FP bug core dumps, ignoring	*/
	/*	the error like most platforms do by default.					*/
	fpsetmask(0);
#endif
	printf("Initializing SDL..\n");
	videosettings(true, false);

	printf("Creating the terrain..\n");
	if (!CreateScene())
		return 0;

	printf("Running..\n");
	run();

	return 0;
}

int main(int, char ** )
{
	App app;
	return app.main();
}
