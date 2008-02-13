//
// Vehicles.cpp
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/TerrainScene.h"	// for vtGetContent
#include "vtlib/core/GeomUtil.h"		// for CreateBoundSphereGeom
#include "vtdata/vtLog.h"
#include "CarEngine.h"
#include "Vehicles.h"


///////////////////////////////
// helpers

#if VTLIB_OSG
// Walk an OSG scenegraph looking for geodes with statesets, change the color
//  of any materials found.
class RemapDiffuseVisitor : public osg::NodeVisitor
{
public:
	RemapDiffuseVisitor() : NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
	virtual void apply(osg::Geode& geode)
	{
		for (unsigned i=0; i<geode.getNumDrawables(); ++i)
		{
			osg::Geometry *geo = dynamic_cast<osg::Geometry *>(geode.getDrawable(i));
			if (!geo) continue;

			osg::StateSet *stateset = geo->getStateSet();
			if (!stateset) continue;

			osg::StateAttribute *state = stateset->getAttribute(osg::StateAttribute::MATERIAL);
			if (!state) continue;

			osg::Material *mat = dynamic_cast<osg::Material *>(state);
			if (!mat) continue;

			const osg::Vec4 v4 = mat->getDiffuse(FAB);
			if (v4.r() == 1.0f && v4.g() == 0.0f && v4.b() == 1.0f)
			{
				//VTLOG("oldmat rc %d, ", mat->referenceCount());

				osg::Material *newmat = (osg::Material *)mat->clone(osg::CopyOp::DEEP_COPY_ALL);
				newmat->setDiffuse(FAB, osg::Vec4(c.r*2/3,c.g*2/3,c.b*2/3,1));
				newmat->setAmbient(FAB, osg::Vec4(c.r*1/3,c.g*1/3,c.b*1/3,1));

				//VTLOG("newmat rc %d\n", newmat->referenceCount());

				stateset->setAttribute(newmat);
				//VTLOG(" -> %d %d\n", mat->referenceCount(), newmat->referenceCount());
			}
		}
		osg::NodeVisitor::apply(geode);
	}
	RGBf c;
};
#endif

void ConvertPurpleToColor(vtNode *pModel, RGBf replace)
{
#if VTLIB_OSG
	RemapDiffuseVisitor viz;
	viz.c = replace;
	pModel->GetOsgNode()->accept(viz);
#endif
}

/////////////////////////////////////////

VehicleManager::VehicleManager()
{
}

VehicleManager::~VehicleManager()
{
}

Vehicle *VehicleManager::CreateVehicleFromNode(vtNode *node, const RGBf &cColor)
{
	Vehicle *pNewVehicle = new Vehicle;

	//VTLOG1("-----------------\n");
	//vtLogGraph(node);
	//VTLOG1("-----------------\n");
	//vtLogNativeGraph(node->GetOsgNode());

	vtGroup *pNewModel = (vtGroup *)node->Clone(true);	// Deep copy

	//VTLOG1("-----------------\n");
	//vtLogNativeGraph(pNewModel->GetOsgNode());

	pNewVehicle->AddChild(pNewModel);

	vtNode *pFrontLeft = pNewModel->FindNativeNode("front_left");
	vtNode *pFrontRight = pNewModel->FindNativeNode("front_right");
	vtNode *pRearLeft = pNewModel->FindNativeNode("rear_left");
	vtNode *pRearRight = pNewModel->FindNativeNode("rear_right");

	if (!pFrontLeft || !pFrontRight || !pRearLeft || !pRearRight)
	{
		// Didn't find them.
		return NULL;
	}
	if (dynamic_cast<vtTransform*>(pFrontLeft))
	{
		// They are already transforms, no need to insert any
		pFrontLeft->SetName2("front_left_xform");
		pFrontRight->SetName2("front_right_xform");
		pRearLeft->SetName2("rear_left_xform");
		pRearRight->SetName2("rear_right_xform");
	}
	else
	{
		// Stick transform above them (this seems like a bad way to go)
		vtTransform *pTransform;

		pTransform = new vtTransform;
		pTransform->SetName2("front_left_xform");
		pTransform->AddChild(pFrontLeft);
		pFrontLeft->GetParent()->RemoveChild(pFrontLeft);
		pNewModel->AddChild(pTransform);

		pTransform = new vtTransform;
		pTransform->SetName2("front_right_xform");
		pTransform->AddChild(pFrontRight);
		pFrontRight->GetParent()->RemoveChild(pFrontRight);
		pNewModel->AddChild(pTransform);

		pTransform = new vtTransform;
		pTransform->SetName2("rear_left_xform");
		pTransform->AddChild(pRearLeft);
		pRearLeft->GetParent()->RemoveChild(pRearLeft);
		pNewModel->AddChild(pTransform);

		pTransform = new vtTransform;
		pTransform->SetName2("rear_right_xform");
		pTransform->AddChild(pRearRight);
		pRearRight->GetParent()->RemoveChild(pRearRight);
		pNewModel->AddChild(pTransform);

		//VTLOG1("-----------------\n");
		//vtLogNativeGraph(pNewModel->GetOsgNode());
	}
	ConvertPurpleToColor(pNewVehicle, cColor);

	return pNewVehicle;
}


Vehicle *VehicleManager::CreateVehicle(const char *szType, const RGBf &cColor)
{
	vtContentManager3d &con = vtGetContent();
	vtNode *node = con.CreateNodeFromItemname(szType);
	if (!node)
		return NULL;
	Vehicle *v = CreateVehicleFromNode(node, cColor);
	if (v)
		v->SetName2(vtString("Vehicle-") + szType);
	return v;
}


///////////////////////////////////////////////////////////////////////

Vehicle::Vehicle()
{
	m_pHighlight = NULL;
}

void Vehicle::ShowBounds(bool bShow)
{
	if (bShow)
	{
		if (!m_pHighlight)
		{
			// the highlight geometry doesn't exist, so create it
			// get bounding sphere
			vtNode *contents = GetChild(0);
			if (contents)
			{
				FSphere sphere;
				GetBoundSphere(sphere);
				sphere.center.Set(0,0,0);

				m_pHighlight = CreateBoundSphereGeom(sphere);
				AddChild(m_pHighlight);
			}
		}
		m_pHighlight->SetEnabled(true);
	}
	else
	{
		if (m_pHighlight)
			m_pHighlight->SetEnabled(false);
	}
}


///////////////////////////////////////////////////////////////////////

VehicleSet::VehicleSet()
{
	m_iSelected = -1;
}

void VehicleSet::AddEngine(CarEngine *e)
{
	m_Engines.push_back(e);
}

int VehicleSet::FindClosestVehicle(const FPoint3 &point, float &closest)
{
	closest = 1E9;
	int vehicle = -1;
	for (unsigned int i = 0; i < m_Engines.size(); i++)
	{
		FPoint3 vepos = m_Engines[i]->GetCurPos();
		float dist = (point - vepos).Length();
		if (dist < closest)
		{
			closest = dist;
			vehicle = i;
		}
	}
	return vehicle;
}

void VehicleSet::VisualSelect(int vehicle)
{
	// Stop vehicle simulation while it is selected
	CarEngine *eng = m_Engines[vehicle];
	eng->SetEnabled(false);

	Vehicle *car = dynamic_cast<Vehicle*> (eng->GetTarget());
	if (!car)
		return;
	car->ShowBounds(true);
	m_iSelected = vehicle;
}

void VehicleSet::VisualDeselectAll()
{
	unsigned int size = m_Engines.size();
	for (unsigned int i = 0; i < size; i++)
	{
		// Resume vehicle simulation while it is deselected
		CarEngine *eng = m_Engines[i];
		eng->SetEnabled(true);
		eng->IgnoreElapsedTime();

		Vehicle *car = dynamic_cast<Vehicle*> (eng->GetTarget());
		if (car)
			car->ShowBounds(false);
	}
	m_iSelected = -1;
}

void VehicleSet::SetVehicleSpeed(int vehicle, float fMetersPerSec)
{
	CarEngine *eng = m_Engines[vehicle];
	eng->SetTargetSpeed(fMetersPerSec);
}

CarEngine *VehicleSet::GetSelectedCarEngine()
{
	if (m_iSelected != -1)
		return m_Engines[m_iSelected];
	else
		return NULL;
}

