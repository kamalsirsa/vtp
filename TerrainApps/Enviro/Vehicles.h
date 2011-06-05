//
// Vehicles.h
//
// Classes for describing and operating on vehicles.
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VEHICLEH
#define VEHICLEH

class CarEngine;

class Vehicle : public vtTransform
{
public:
	Vehicle();
	~Vehicle();

	void ShowBounds(bool bShow);

	vtGeode	*m_pHighlight;	// The wireframe highlight

	vtNode *m_pFrontLeft;
	vtNode *m_pFrontRight;
	vtNode *m_pRearLeft;
	vtNode *m_pRearRight;
};

class VehicleManager
{
public:
	VehicleManager();
	~VehicleManager();

	Vehicle *CreateVehicle(const char *szType, const RGBf &cColor);

protected:
	Vehicle *CreateVehicleFromNode(osg::Node *node, const RGBf &cColor);
};

class VehicleSet
{
public:
	VehicleSet();

	void AddEngine(CarEngine *e);
	int FindClosestVehicle(const FPoint3 &point, float &closest);
	void VisualSelect(int vehicle);
	void VisualDeselectAll();
	int GetSelected() { return m_iSelected; }
	void SetVehicleSpeed(int vehicle, float fMetersPerSec);

	CarEngine *GetSelectedCarEngine();
	void SetSelectedRotation(float fRot);

	std::vector<CarEngine*> m_Engines;
	int m_iSelected;
};

#endif	// VEHICLEH

