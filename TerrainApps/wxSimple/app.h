//
// Name: app.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

class vtTerrainScene;

// Define a new application type
class vtApp: public wxApp
{
public:
	bool OnInit(void);
	bool CreateScene();
	int OnExit();

	class vtFrame *m_frame;
	vtTerrainScene *m_pTerrainScene;
	vtCamera *m_pCamera;
};

