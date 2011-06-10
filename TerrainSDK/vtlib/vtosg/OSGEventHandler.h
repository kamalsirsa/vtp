//
// OSGEventHandler.h
//
// Copyright (c) 2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

/** Turn OSG events into vtlib events */
class vtOSGEventHandler : public osgGA::GUIEventHandler 
{
public: 
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
	void handleKeyEvent(const osgGA::GUIEventAdapter& ea);
	void handleMouseEvent(const osgGA::GUIEventAdapter& ea);
};

