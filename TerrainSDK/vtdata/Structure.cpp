//
// Structure.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include "Structure.h"
#include "StructArray.h"
#include "Building.h"
#include "Fence.h"
#include "xmlhelper/easyxml.hpp"
#include "vtLog.h"
#include "FilePath.h"

vtStructInstance::vtStructInstance() : vtStructure()
{
	SetType(ST_INSTANCE);

	m_p.Set(0, 0);
	m_fRotation = 0.0f;
	m_fScale = 1.0f;

	m_pItem = NULL;
}

/**
 * Asignment operator, which makes an explicit copy the entire building
 * including each level.
 */
vtStructInstance &vtStructInstance::operator=(const vtStructInstance &v)
{
	// copy parent data
	vtStructure::CopyFrom(v);

	// copy class data
	m_p = v.m_p;
	m_fRotation = v.m_fRotation;
	m_fScale = v.m_fScale;
	m_pItem = v.m_pItem;
	return *this;
}

void vtStructInstance::WriteXML(GZOutput &out, bool bDegrees) const
{
	const char *coord_format = "%.9lg";	// up to 9 significant digits

	gfprintf(out, "\t<Imported");
	if (m_fElevationOffset != 0.0f)
		gfprintf(out, " ElevationOffset=\"%.2f\"", m_fElevationOffset);
	if (m_bAbsolute)
		gfprintf(out, " Absolute=\"true\"");
	gfprintf(out, ">\n");

	// first write the placement
	gfprintf(out, "\t\t<Location>\n");
	gfprintf(out, "\t\t\t<gml:coordinates>");
	gfprintf(out, coord_format, m_p.x);
	gfprintf(out, ",");
	gfprintf(out, coord_format, m_p.y);
	gfprintf(out, "</gml:coordinates>\n");
	gfprintf(out, "\t\t</Location>\n");

	if (m_fRotation != 0.0f)
	{
		gfprintf(out, "\t\t<Rotation>%g</Rotation>\n", m_fRotation);
	}
	if (m_fScale != 1.0f)
	{
		gfprintf(out, "\t\t<Scale>%g</Scale>\n", m_fScale);
	}
	WriteTags(out);
	gfprintf(out, "\t</Imported>\n");
}

bool vtStructInstance::GetExtents(DRECT &rect) const
{
	// we have no way (yet) of knowing the extents of an external
	// reference, so just give a placeholder of a single point.
	rect.SetRect(m_p.x, m_p.y, m_p.x, m_p.y);
	return true;
}

bool vtStructInstance::IsContainedBy(const DRECT &rect) const
{
	return rect.ContainsPoint(m_p);
}

double vtStructInstance::DistanceToPoint(const DPoint2 &p, float fMaxRadius) const
{
	// simple distance from the origin of this instance to the given point
	return (m_p - p).Length();
}


///////////////////////////////////////////////////////////////////////

vtStructure::vtStructure()
{
	m_type = ST_NONE;
	m_fElevationOffset = 0.0f;
	m_bAbsolute = false;
#ifdef VIAVTDATA
	m_bIsVIAContributor = false;
	m_bIsVIATarget = false;
#endif
}

vtStructure::~vtStructure()
{
	m_type = ST_NONE;
}

void vtStructure::CopyFrom(const vtStructure &v)
{
	// copy parent members
	CopyTagsFrom(v);

	// copy structure members
	m_type = v.m_type;
	m_fElevationOffset = v.m_fElevationOffset;
	m_bAbsolute = v.m_bAbsolute;
}

void vtStructure::WriteTags(GZOutput &out) const
{
	// now write all extra tags (attributes) for this structure
	for (unsigned int i = 0; i < NumTags(); i++)
	{
		const vtTag *tag = GetTag(i);
		gfprintf(out, "\t\t<%s>%s</%s>\n", (const char *)tag->name,
			(const char *)tag->value, (const char *)tag->name);
	}
}

////////////////////////////////////////////////////////////////////////


// XML parser for MaterialDescriptorArray

class MaterialDescriptorArrayVisitor : public XMLVisitor
{
public:
	MaterialDescriptorArrayVisitor(vtMaterialDescriptorArray *MDA) : m_state(0), m_pMDA(MDA) {}
	void startXML() { m_state = 0; }
	void endXML() { m_state = 0; }
	void startElement(const char *name, const XMLAttributes &atts);

private:
	int m_state;

	vtMaterialDescriptorArray *m_pMDA;
};

void MaterialDescriptorArrayVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	const char *attval;

	if (m_state == 0 && !strcmp(name, "MaterialDescriptorArray"))
		m_state = 1;
	else if (m_state == 1)
	{
		if (!strcmp(name, "MaterialDescriptor"))
		{
			vtMaterialDescriptor *pDescriptor = new vtMaterialDescriptor;
			attval = atts.getValue("Name");
			if (attval)
				pDescriptor->SetName(*(new vtString(attval)));

			pDescriptor->SetColorable(VT_MATERIAL_SELFCOLOURED_TEXTURE);
			attval = atts.getValue("Colorable");
			if (attval && !strcmp(attval, "true"))
				pDescriptor->SetColorable(VT_MATERIAL_COLOURABLE_TEXTURE);

			attval = atts.getValue("Type");
			if (attval)
				pDescriptor->SetType(atoi(attval));
			attval = atts.getValue("Source");
			if (attval)
				pDescriptor->SetSourceName(attval);
			attval = atts.getValue("Scale");
			if (attval)
			{
				float x, y;
				int terms = sscanf(attval, "%f, %f", &x, &y);
				if (terms == 1)
					pDescriptor->SetUVScale(x, x);
				if (terms == 2)
					pDescriptor->SetUVScale(x, y);
			}
			attval = atts.getValue("RGB");
			if (attval)
			{
				short r, g, b;
				sscanf(attval, "%2hx %2hx %2hx", &r, &g, &b);
				pDescriptor->SetRGB(RGBi(r, g, b));
			}
			m_pMDA->Append(pDescriptor);
		}
	}
}


///////////////////////////////////////////////////////////////////////
// Methods for vtMaterialDescriptor
//
vtMaterialDescriptor::vtMaterialDescriptor()
{
	m_pName = NULL;
	m_Type = 0;
	m_Colorable = VT_MATERIAL_SELFCOLOURED_TEXTURE;
	m_UVScale.Set(1,1);
	m_bTwoSided = false;
	m_bAmbient = false;
	m_bBlending = false;
	m_RGB.Set(0,0,0);
	m_iMaterialIndex = -1;
}

vtMaterialDescriptor::vtMaterialDescriptor(const char *name,
	const vtString &SourceName, const vtMaterialColorEnum Colorable,
	const float fUVScaleX, const float fUVScaleY, const bool bTwoSided,
	const bool bAmbient, const bool bBlending, const RGBi &Color)
{
	m_pName = new vtString(name);	//GetGlobalMaterials()->FindName(name);
	m_Type = 0;
	m_SourceName = SourceName;
	m_Colorable = Colorable;
	m_UVScale.Set(fUVScaleX, fUVScaleY);
	m_bTwoSided = bTwoSided;
	m_bAmbient = bAmbient;
	m_bBlending = bBlending;
	m_RGB = Color;
	m_iMaterialIndex = -1;
}

vtMaterialDescriptor::~vtMaterialDescriptor()
{
	if (m_pName)
		delete m_pName;
}


///////////////////////////////////////////////////////////////////////
// Methods for vtMaterialDescriptorArray
//

void vtMaterialDescriptorArray::CreatePlain()
{
	// First provide plain material, which does need to be serialized
	Append(new vtMaterialDescriptor(BMAT_NAME_PLAIN, "", VT_MATERIAL_COLOURED));
}

bool vtMaterialDescriptorArray::Load(const char *szFileName)
{
	VTLOG("Loading materials from %s\n", szFileName);

	MaterialDescriptorArrayVisitor Visitor(this);
	try
	{
		readXML(szFileName, Visitor);
	}
	catch (xh_exception &e)
	{
		// TODO: would be good to pass back the error message.
		VTLOG("vtMaterialDescriptorArray::Load xml error %s\n", e.getMessage().c_str());
		return false;
	}
	return true;
}

const vtString *vtMaterialDescriptorArray::FindName(const char *name)
{
	int iIndex;
	int iSize = GetSize();

	for (iIndex = 0; iIndex < iSize; iIndex++)
	{
		const vtString *pFoundName = &GetAt(iIndex)->GetName();
		if (pFoundName && *pFoundName == name)
			return pFoundName;
	}
	return NULL;
}

bool vtMaterialDescriptorArray::LoadExternalMaterials(const vtStringArray &paths)
{
	// we always need at least 1 internal material
	CreatePlain();

	const char *fname = "Culture/materials.xml";
	vtString matfile = FindFileOnPaths(paths, fname);
	if (matfile == "")
	{
		VTLOG("Couldn't find '%s' on Data paths, building materials will "\
			"not be available.\n", fname);
		return false;
	}
	return Load(matfile);
}

//
// Global material descriptors
//
static vtMaterialDescriptorArray *g_pMaterials = NULL;

bool LoadGlobalMaterials(const vtStringArray &paths)
{
	GetGlobalMaterials();
	return g_pMaterials->LoadExternalMaterials(paths);
}

void SetGlobalMaterials(vtMaterialDescriptorArray *mats)
{
	g_pMaterials = mats;
}

vtMaterialDescriptorArray *GetGlobalMaterials()
{
	if (!g_pMaterials)
		g_pMaterials = new vtMaterialDescriptorArray;
	return g_pMaterials;
}

void FreeGlobalMaterials()
{
	delete g_pMaterials;
	g_pMaterials = NULL;
}

