
#ifndef SWAPENGINEH
#define SWAPENGINEH

#include "vtlib/core/Terrain.h"
#include "vtlib/core/Engine.h"

class SwapEngine : public vtEngine
{
public:
	SwapEngine();

	void Eval();
	void TriggerHit(vtTerrain* terrain, int jump_index);
	void FadeOut();
	void FadeIn();

	vtMaterial* m_mat;
	vtTerrain* m_pterrain;
	int m_destination_index;
	bool m_fadingOut;
	bool m_fadingIn;
	bool m_switch;
	int m_framecount;
	float m_time;
};

#endif
