//
// Structure.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include "Building.h"
#include "DataPath.h"
#include "Fence.h"
#include "FilePath.h"
#include "Structure.h"
#include "StructArray.h"
#include "vtLog.h"

#include "xmlhelper/easyxml.hpp"

vtStructInstance::vtStructInstance() : vtStructure()
{
	SetType(ST_INSTANCE);

	m_p.Set(0, 0);
	m_fRotation = 0.0f;
	m_fScale = 1.0f;

	m_pItem = NULL;
}

/**
 * Asignment operator, which makes an explicit copy of the structure's
 *  description tags.
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
	for (uint i = 0; i < NumTags(); i++)
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
				pDescriptor->SetMatType(atoi(attval));
			attval = atts.getValue("Source");
			if (attval)
				pDescriptor->SetTextureFilename(attval);
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

			attval = atts.getValue("TwoSided");
			if (attval && !strcmp(attval, "true"))
				pDescriptor->SetTwoSided(true);

			attval = atts.getValue("Blending");
			if (attval && !strcmp(attval, "true"))
				pDescriptor->SetBlending(true);

			m_pMDA->push_back(pDescriptor);
		}
	}
}


///////////////////////////////////////////////////////////////////////
// Methods for vtMaterialDescriptor
//
vtMaterialDescriptor::vtMaterialDescriptor()
{
	m_Type = 0;
	m_Colorable = VT_MATERIAL_SELFCOLOURED_TEXTURE;
	m_UVScale.Set(1,1);
	m_bTwoSided = false;
	m_bAmbient = false;
	m_bBlending = false;
	m_RGB.Set(0,0,0);
}

/**
 * Create a high-level description of a material.
 *
 * \param Name Name of the new material.
 * \param TextureFilename Filename of the source imagery, for a textured material,
 *		 or "" otherwise.
 * \param Colorable Enumeration, one of:
	- VT_MATERIAL_COLOURABLE A plain surface that can be any color.
	- VT_MATERIAL_SELFCOLOURED_TEXTURE A textured surface.
	- VT_MATERIAL_COLOURABLE_TEXTURE A textured surface which can be made any color.
 * \param fUVScaleX, fUVScaleY For textured materials, this is the size of the
 *		provided texture, in meters.  For example, if the texture is of a row
 *		of bricks that is 50x50 cm, then the scale factors are (.5, .5).
 *		If a texture should not be tiled (for example, a door or window which
 *		always exactly fits its surface) then scale factor is -1.
 * \param bTwoSided True for surfaces which should be visible from both sides.
 * \param bAmbient True for surfaces which are only affected by ambient light.
 * \param bBlending True for textures with transparency.
 * \param Color For VT_MATERIAL_SELFCOLOURED_TEXTURE, the color intrinsic to
 *		the texture, e.g. for a texture of orange bricks, the color is orange.
 */
vtMaterialDescriptor::vtMaterialDescriptor(const char *Name,
	const vtString &TextureFilename, const vtMaterialColorEnum Colorable,
	const float fUVScaleX, const float fUVScaleY, const bool bTwoSided,
	const bool bAmbient, const bool bBlending, const RGBi &Color)
{
	m_Name = Name;
	m_Type = 0;
	m_TextureFilename = TextureFilename;
	m_Colorable = Colorable;
	m_UVScale.Set(fUVScaleX, fUVScaleY);
	m_bTwoSided = bTwoSided;
	m_bAmbient = bAmbient;
	m_bBlending = bBlending;
	m_RGB = Color;
}

vtMaterialDescriptor::~vtMaterialDescriptor()
{
}

void vtMaterialDescriptor::WriteToFile(FILE *fp)
{
	fprintf(fp, "\t<MaterialDescriptor Name=\"");
	fprintf(fp, "%s", (pcchar) m_Name);
	fprintf(fp, "\"");

	if (m_Type != 0)
		fprintf(fp, " Type=\"%d\"", m_Type);

	fprintf(fp, " Colorable=\"%s\"", m_Colorable == VT_MATERIAL_COLOURABLE_TEXTURE ? "true" : "false");

	fprintf(fp, " Source=\"");
	fprintf(fp, "%s", (pcchar) m_TextureFilename);
	fprintf(fp, "\"");

	fprintf(fp, " Scale=\"%f, %f\"", m_UVScale.x, m_UVScale.y);
//	fprintf(fp, " RGB=\"" << m_RGB.r << " " << m_RGB.g << " " << m_RGB.b << "\""

	if (m_bTwoSided)
		fprintf(fp, " TwoSided=\"true\"");

	if (m_bBlending)
		fprintf(fp, " Blending=\"true\"");

	fprintf(fp, "/>\n");
}
	//friend std::ostream &operator << (std::ostream & Output, const vtMaterialDescriptor &Input)
	//{
	//	const RGBi &rgb = Input.m_RGB;
	//	Output << "\t<MaterialDescriptor Name=\""<< (pcchar)*Input.m_pName << "\""
	//		<< " Colorable=\"" << (Input.m_Colorable == VT_MATERIAL_COLOURABLE_TEXTURE) << "\""
	//		<< " Source=\"" << (pcchar)Input.m_TextureFilename << "\""
	//		<< " Scale=\"" << Input.m_UVScale.x << ", " << Input.m_UVScale.y << "\""
	//		<< " RGB=\"" << rgb.r << " " << rgb.g << " " << rgb.b << "\""
	//		<< "/>" << std::endl;
	//	return Output;
	//}


///////////////////////////////////////////////////////////////////////
// Methods for vtMaterialDescriptorArray
//

void vtMaterialDescriptorArray::CreatePlain()
{
	// First provide plain material, which does need to be serialized
	push_back(new vtMaterialDescriptor(BMAT_NAME_PLAIN, "", VT_MATERIAL_COLOURABLE));
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
	VTLOG(" successfully loaded %d material descriptions.\n", size());
	return true;
}

bool vtMaterialDescriptorArray::Save(const char *szFileName)
{
	VTLOG("Saving materials to %s\n", szFileName);

	FILE *fp = vtFileOpen(szFileName, "wb");
	if (!fp)
		return false;

	fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	fprintf(fp, "<MaterialDescriptorArray>\n");
	uint iSize = size();
	for (uint i = 0; i < iSize; i++)
		at(i)->WriteToFile(fp);
	fprintf(fp, "</MaterialDescriptorArray>\n");
	fclose(fp);

	return true;
}

	//friend std::ostream &operator << (std::ostream & Output, vtMaterialDescriptorArray &Input)
	//{
	//	int iSize = Input.size();
	//	Output << "<?xml version=\"1.0\"?>" << std::endl;
	//	Output << "<MaterialDescriptorArray>" << std::endl;
	//	for (int i = 0; i < iSize; i++)
	//		Output << *Input.at(i);
	//	Output << "</MaterialDescriptorArray>" << std::endl;
	//	return Output;
	//}

const vtString *vtMaterialDescriptorArray::FindName(const char *name) const
{
	int iIndex;
	int iSize = size();

	for (iIndex = 0; iIndex < iSize; iIndex++)
	{
		const vtString *pFoundName = &(at(iIndex)->GetName());
		if (pFoundName && (*pFoundName).CompareNoCase(name) == 0)
			return pFoundName;
	}
	return NULL;
}

bool vtMaterialDescriptorArray::LoadExternalMaterials()
{
	VTLOG1("vtMaterialDescriptorArray::LoadExternalMaterials:\n");

	// we always need at least 1 internal material
	CreatePlain();

	const char *fname = "Culture/materials.xml";
	vtString matfile = FindFileOnPaths(vtGetDataPath(), fname);
	if (matfile == "")
	{
		VTLOG("Couldn't find '%s' on Data paths, building materials will "\
			"not be available.\n", fname);
		return false;
	}
	VTLOG1("   found, loading.\n");
	return Load(matfile);
}

//
// Global material descriptors
//
static vtMaterialDescriptorArray *g_pMaterials = NULL;

bool LoadGlobalMaterials()
{
	GetGlobalMaterials();
	return g_pMaterials->LoadExternalMaterials();
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

