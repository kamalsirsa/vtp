//
// Name:     app.cpp
// Purpose:  Example GLUT/vtlib application.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __DARWIN_OSX__
 #import <glut.h>
#else
 #include <GL/glut.h>
#endif

#ifdef __FreeBSD__
#  include <ieeefp.h>
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtdata/vtLog.h"

void redraw();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

/*
  The GLUT window reshape event
*/
void reshape(int w, int h )
{
	glViewport(0, 0, w, h);
	vtGetScene()->SetWindowSize(w, h);
}

/*
  The GLUT keyboard event
*/
void keyboard(unsigned char key, int x, int y)
{
	if ( key == 27 /* ESC */ || key == 'q' || key == 'Q' )
		exit(0);

	vtGetScene()->OnKey(key, 0);
}

/* Mouse buttons. */
#define GLUT_LEFT_BUTTON		0
#define GLUT_MIDDLE_BUTTON		1
#define GLUT_RIGHT_BUTTON		2

/* Mouse button  state. */
#define GLUT_DOWN			0
#define GLUT_UP				1

int last_button_state;

/*
  The GLUT mouse event
*/
void mouse(int button, int state, int x, int y)
{
	// turn GLUT mouse event into a VT mouse event
	vtMouseEvent event;
	event.type = (state == GLUT_DOWN) ? VT_DOWN : VT_UP;

	if (button == GLUT_LEFT_BUTTON)
		event.button = VT_LEFT;
	else if (button == GLUT_MIDDLE_BUTTON)
		event.button = VT_MIDDLE;
	else if (button == GLUT_RIGHT_BUTTON)
		event.button = VT_RIGHT;

	event.flags = 0;
	event.pos.Set(x, y);

	vtGetScene()->OnMouse(event);
}

void motion(int x, int y)
{
	vtMouseEvent event;
	event.type = VT_MOVE;
	event.button = VT_NONE;
	event.flags = 0;
	event.pos.Set(x, y);

	vtGetScene()->OnMouse(event);
}


/*
  The GLUT redraw event
*/
void redraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vtGetScene()->DoUpdate();

	glutPostRedisplay();
	glutSwapBuffers();
}


void InitGLUT()
{
	int   fake_argc = 1;
	char *fake_argv[3];
	fake_argv[0] = "glutSimple";
	fake_argv[1] = "Example Program";
	fake_argv[2] = NULL;

	/*
	Initialise GLUT
	*/
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(800, 600);
	glutInit          (&fake_argc, fake_argv);
	glutInitDisplayMode (GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow  (fake_argv[1]);
	glutDisplayFunc   (redraw);
	glutReshapeFunc   (reshape);
	glutKeyboardFunc  (keyboard);
	glutMouseFunc     (mouse);
	glutMotionFunc    (motion);
}


//
// Create the 3d scene
//
bool CreateScene()
{
	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();
	pScene->Init();

	// Log messages to make troubleshooting easier
	g_Log._StartLog("debug.txt");
	VTLOG("glutSimple\n");

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
	vtGroup *pTopGroup = ts->BeginTerrainScene(false);

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	// Create a new vtTerrain, read its parameters from a file
	vtTerrain *pTerr = new vtTerrain();
	pTerr->SetParamFile("Data/Simple.ini");

	// Add the terrain to the scene, and contruct it
	ts->AppendTerrain(pTerr);
	int iError;
	if (!pTerr->CreateScene(false, iError))
	{
		printf("Terrain creation failed, error %d.\n", iError);
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


/*
  The works.
*/
int main(int, char ** )
{
#ifdef __FreeBSD__
	/*  FreeBSD is more stringent with FP ops by default, and OSG is doing  */
	/*    silly things sqrt(Inf) (computing lengths of MAXFLOAT and NaN     */
	/*    Vec3's).   This turns off FP bug core dumps, ignoring the error   */
	/*    like most platforms do by default.                                */
	fpsetmask(0);
#endif

	printf("Initializing GLUT..\n");
	InitGLUT();

	printf("Creating the terrain..\n");
	CreateScene();

	printf("Running..\n");
	glutMainLoop();

	return 0;
}

