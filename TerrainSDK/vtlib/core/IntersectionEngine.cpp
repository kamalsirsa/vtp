//
// IntersectionEngine.cpp
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "IntersectionEngine.h"

//
// Intersection Engine.  changes traffic lights at nodes.
//

vtIntersectionEngine::vtIntersectionEngine(NodeGeom *node) {
	//uses node's initial light state to determine which signal lights are linked.
	//lights of one color change together.
	m_pNode = node;
	m_bFirstTime = true;

	m_iTotalStates = 4;
	m_Lights = new LightStatus[m_pNode->m_iLinks * m_iTotalStates];

	for (int i = 0; i < m_pNode->m_iLinks; i++) {
		m_Lights[i] = m_pNode->GetLightStatus(i);
		if (m_Lights[i] == LT_RED) {
			m_Lights[i+m_pNode->m_iLinks] = LT_RED;
			m_Lights[i+2*m_pNode->m_iLinks] = LT_GREEN;
			m_Lights[i+3*m_pNode->m_iLinks] = LT_YELLOW;
		} else if (m_Lights[i] == LT_GREEN) {
			m_Lights[i+m_pNode->m_iLinks] = LT_YELLOW;
			m_Lights[i+2*m_pNode->m_iLinks] = LT_RED;
			m_Lights[i+3*m_pNode->m_iLinks] = LT_RED;
		} else {
			//error.
			//assert(false);
		}
	}
}

vtIntersectionEngine::~vtIntersectionEngine()
{
	delete m_Lights;
}

//changes the light colors at a node.
void vtIntersectionEngine::Eval() {
	float t = vtGetTime();
	if (!m_bFirstTime) {
		if (t > m_fNextChange) {
			m_iCurState++;
			m_iCurState = m_iCurState % m_iTotalStates;
			//assign new light states.
			for (int i = 0; i < m_pNode->m_iLinks; i++) {
				LightStatus l = m_Lights[i+m_pNode->m_iLinks*m_iCurState];
				m_pNode->SetLightStatus(i, l);
			}
			if (m_iCurState & 0x1) {
				//if curret state is odd:
				//there is a yellow light.  use shorter time
				m_fNextChange = t + LT_SWITCH_TIME2;
			} else {
				//lights are green and red.  using longer time
				m_fNextChange = t + LT_SWITCH_TIME;
			}
		}
	} else {
		m_bFirstTime = false;
		//generate current status randomly.
		m_iCurState = (rand() % m_iTotalStates);

		//generate offset time randomly.
		int offset;
		if (m_iCurState & 0x1) {
			offset = rand() % LT_SWITCH_TIME2;
		} else {
			offset = rand() % LT_SWITCH_TIME;
		}

		//assign new light states.
		for (int i = 0; i < m_pNode->m_iLinks; i++) {
			m_pNode->SetLightStatus(i, m_Lights[i+m_pNode->m_iLinks*m_iCurState]);
		}
		m_fNextChange = t + offset;
	}
}

