//
// vtSOG.h
//
// vtlib implementation of SOG file IO.
//

#include "vtlib/vtlib.h"
#include "vtSOG.h"


//////////////////////////////////////////////////////////
// Local functions

void OutputSOG::Write(FILE *fp, FileToken ft, RGBf &rgb)
{
	short s = (short) ft;
	fwrite(&s, 2, 1, fp);
	s = sizeof(RGBf);
	fwrite(&s, 2, 1, fp);
	fwrite(&rgb, s, 1, fp);
}

void OutputSOG::Write(FILE *fp, FileToken ft, RGBAf &rgba)
{
	short s = (short) ft;
	fwrite(&s, 2, 1, fp);
	s = sizeof(RGBAf);
	fwrite(&s, 2, 1, fp);
	fwrite(&rgba, s, 1, fp);
}

void OutputSOG::Write(FILE *fp, FileToken ft, bool &b)
{
	short s = (short) ft;
	fwrite(&s, 2, 1, fp);
	s = sizeof(char);
	fwrite(&s, 2, 1, fp);
	char c = (char) b;
	fwrite(&c, s, 1, fp);
}

void OutputSOG::Write(FILE *fp, FileToken ft, vtString &str)
{
	short s = (short) ft;
	fwrite(&s, 2, 1, fp);

	const char *cstr = str;
	s = strlen(cstr)+1;
	fwrite(&s, 2, 1, fp);
	fwrite(cstr, s, 1, fp);
}

void OutputSOG::Write(FILE *fp, FileToken ft, short &s1)
{
	short s = (short) ft;
	fwrite(&s, 2, 1, fp);

	s = sizeof(short);
	fwrite(&s, 2, 1, fp);
	fwrite(&s1, s, 1, fp);
}

void OutputSOG::Write(FILE *fp, FileToken ft, FPoint3 &p)
{
	short s = (short) ft;
	fwrite(&s, 2, 1, fp);
	s = sizeof(FPoint3);
	fwrite(&s, 2, 1, fp);
	fwrite(&p, s, 1, fp);
}

void OutputSOG::Write(FILE *fp, FileToken ft, FPoint2 &p)
{
	short s = (short) ft;
	fwrite(&s, 2, 1, fp);
	s = sizeof(FPoint2);
	fwrite(&s, 2, 1, fp);
	fwrite(&p, s, 1, fp);
}

void OutputSOG::WriteToken(FILE *fp, FileToken ft, short length)
{
	short s = (short) ft;
	fwrite(&s, 2, 1, fp);
	fwrite(&length, 2, 1, fp);
}

void OutputSOG::WriteHeader(FILE *fp)
{
	// write file type identifier
	fwrite(SOG_HEADER, 4, 1, fp);

	short version = SOG_VERSION;
	Write(fp, FT_VERSION, version);
}

//
// Write the materials
//
void OutputSOG::WriteMaterials(FILE *fp, const vtMaterialArray *pMats)
{
	int i;
	short num_mat = (short) pMats->size();
	Write(fp, FT_NUM_MATERIALS, num_mat);

	for (i = 0; i < num_mat; i++)
	{
		vtMaterial *pMat = pMats->at(i).get();
		WriteMaterial(fp, pMat);
	}
}

void OutputSOG::WriteSingleGeometry(FILE *fp, const vtGeode *pGeode)
{
	const vtMaterialArray	*pMats = pGeode->GetMaterials();
	WriteMaterials(fp, pMats);

	short num_geom = 1;
	Write(fp, FT_NUM_GEOMETRIES, num_geom);
	WriteGeometry(fp, pGeode, 0);
}

void OutputSOG::WriteMultiGeometry(FILE *fp, const vtGroup *pParent)
{
	int i;
	short num_geom = pParent->getNumChildren();

	Write(fp, FT_NUM_GEOMETRIES, num_geom);
	for (i = 0; i < num_geom; i++)
	{
		const osg::Node *pChild = pParent->getChild(i);
		const vtGeode *pGeode = dynamic_cast<const vtGeode*>(pChild);
		if (i == 0)
		{
			// assume that they share the same materials
			const vtMaterialArray	*pMats = pGeode->GetMaterials();
			WriteMaterials(fp, pMats);
		}
		WriteGeometry(fp, pGeode, i);
	}
}

//
//
//
void OutputSOG::WriteGeometry(FILE *fp, const vtGeode *pGeode, short id)
{
	short i;

	// 4 components: name, id, parentid, meshes
	short components = 4;
	Write(fp, FT_GEOMETRY, components);

	vtString gname(pGeode->getName().c_str());
	Write(fp, FT_GEOMNAME, gname);

	Write(fp, FT_GEOMID, id);
	short parentid = -1;
	Write(fp, FT_GEOMPARENTID, parentid);

	//
	// Write the Meshes
	//
	short num_mesh = pGeode->GetNumMeshes();
	Write(fp, FT_NUM_MESHES, num_mesh);

	for (i = 0; i < num_mesh; i++)
	{
		vtMesh *pMesh = pGeode->GetMesh(i);
		if (pMesh)
			WriteMesh(fp, pMesh);
	}
}

void OutputSOG::WriteMaterial(FILE *fp, const vtMaterial *pMat)
{
	RGBAf rgba;
	RGBf rgb;
	bool b;
	vtImage *pImage;
	short components = 7;

//	pImage = pMat->GetTexture();
	pImage = NULL;
	if (pImage)
		components += 2;

	Write(fp, FT_MATERIAL, components);

	rgba = pMat->GetDiffuse();
	Write(fp, FT_DIFFUSE, rgba);

	rgb = pMat->GetSpecular();
	Write(fp, FT_SPECULAR, rgb);

	rgb = pMat->GetAmbient();
	Write(fp, FT_AMBIENT, rgb);

	rgb = pMat->GetEmission();
	Write(fp, FT_EMISSION, rgb);

	b = pMat->GetCulling();
	Write(fp, FT_CULLING, b);

	b = pMat->GetLighting();
	Write(fp, FT_LIGHTING, b);

	b = pMat->GetTransparent();
	Write(fp, FT_TRANSPARENT, b);

	if (pImage)
	{
		b = pMat->GetClamp();
		Write(fp, FT_CLAMP, b);

		vtString fname = pImage->GetFilename().c_str();
		Write(fp, FT_IMAGEFNAME, fname);
	}
}

void OutputSOG::WriteMesh(FILE *fp, const vtMesh *pMesh)
{
	int i;
	FPoint3 p;
	RGBf rgb;
	FPoint2 uv;

	bool need_prim_lengths = false;
	switch (pMesh->getPrimType())
	{
	case osg::PrimitiveSet::POINTS:
	case osg::PrimitiveSet::LINES:
	case osg::PrimitiveSet::TRIANGLES:
	case osg::PrimitiveSet::QUADS:
		need_prim_lengths = false;
		break;

	case osg::PrimitiveSet::LINE_STRIP:
	case osg::PrimitiveSet::TRIANGLE_STRIP:
	case osg::PrimitiveSet::TRIANGLE_FAN:
	case osg::PrimitiveSet::QUAD_STRIP:
	case osg::PrimitiveSet::POLYGON:
		need_prim_lengths = true;
		break;
	}

	short components = 9;
	if (need_prim_lengths)
		components++;
	Write(fp, FT_MESH, components);

	// write vertex flags (normals, colors, texcoords)
	short vtype = 0;
	if (pMesh->hasVertexNormals()) vtype |= VT_Normals;
	if (pMesh->hasVertexColors()) vtype |= VT_Colors;
	if (pMesh->hasVertexTexCoords()) vtype |= VT_TexCoords;
	Write(fp, FT_VTX_FLAGS, vtype);

	// write primitive type (line, triangles, strip..)
	short ptype = pMesh->getPrimType();
	Write(fp, FT_PRIM_TYPE, ptype);

	// write material index
	short matidx = pMesh->GetMatIndex();
	Write(fp, FT_MAT_INDEX, matidx);

	// write number of vertices
	short verts = pMesh->GetNumVertices();
	Write(fp, FT_NUM_VERTICES, verts);

	// write number of primitives
	short idxs = pMesh->GetNumIndices();
	Write(fp, FT_NUM_INDICES, idxs);

	// write number of primitives
	short prims = pMesh->GetNumPrims();
	Write(fp, FT_NUM_PRIMITIVES, prims);

	// write number of components per vertex
	short vcomponents = 1;
	if (vtype & VT_Normals) vcomponents++;
	if (vtype & VT_Colors) vcomponents++;
	if (vtype & VT_TexCoords) vcomponents++;
	Write(fp, FT_VTX_COMPONENTS, vcomponents);

	WriteToken(fp, FT_VTX_ARRAY, verts);
	for (i = 0; i < verts; i++)
	{
		p = pMesh->GetVtxPos(i);
		Write(fp, FT_VTX_POS, p);

		if (vtype & VT_Normals)
		{
			p = pMesh->GetVtxNormal(i);
			Write(fp, FT_VTX_NORMAL, p);
		}
		if (vtype & VT_Colors)
		{
			rgb = pMesh->GetVtxColor(i);
			Write(fp, FT_VTX_COLOR, rgb);
		}
		if (vtype & VT_TexCoords)
		{
			uv = pMesh->GetVtxTexCoord(i);
			Write(fp, FT_VTX_COORD1, uv);
		}
	}

	// write indices
	WriteToken(fp, FT_INDEX_ARRAY, idxs*2);
	short idx;
	for (i = 0; i < idxs; i++)
	{
		idx = pMesh->GetIndex(i);
		fwrite(&idx, 2, 1, fp);
	}

	if (need_prim_lengths)
	{
		// write primitives
		WriteToken(fp, FT_PRIM_LEN_ARRAY, prims*2);

		// write primitive lengths
		short len;
		for (i = 0; i < prims; i++)
		{
			len = pMesh->GetPrimLen(i);
			fwrite(&len, 2, 1, fp);
		}
	}
}


//////////////////////////////////////////////////////////////////////////

bool InputSOG::Read(FILE *fp, short &token, short &len)
{
	if (fread(&token, 2, 1, fp) == 0) return false;
	if (fread(&len, 2, 1, fp) == 0) return false;
	return true;
}

bool InputSOG::ReadBool(FILE *fp)
{
	char c;
	int quiet = fread(&c, 1, 1, fp);
	return (c == 1);
}

vtMaterial *InputSOG::ReadMaterial(FILE *fp)
{
	short token, len;
	short components;
	RGBAf rgba;
	RGBf rgb;
	bool b;
	int i;
	int quiet;

	Read(fp, token, len);

	assert(token == FT_MATERIAL);
	quiet = fread(&components, 2, 1, fp);
	vtMaterial *pMat = new vtMaterial;

	for (i = 0; i < components; i++)
	{
		if (!Read(fp, token, len))
			return NULL;

		switch (token)
		{
		case FT_DIFFUSE:
			quiet = fread(&rgba, sizeof(RGBAf), 1, fp);
			pMat->SetDiffuse1(rgba);
			break;
		case FT_SPECULAR:
			quiet = fread(&rgb, sizeof(RGBf), 1, fp);
			pMat->SetSpecular1(rgb);
			break;
		case FT_AMBIENT:
			quiet = fread(&rgb, sizeof(RGBf), 1, fp);
			pMat->SetAmbient1(rgb);
			break;
		case FT_EMISSION:
			quiet = fread(&rgb, sizeof(RGBf), 1, fp);
			pMat->SetEmission1(rgb);
			break;
		case FT_CULLING:
			b = ReadBool(fp);
			pMat->SetCulling(b);
			break;
		case FT_LIGHTING:
			b = ReadBool(fp);
			pMat->SetLighting(b);
			break;
		case FT_TRANSPARENT:
			b = ReadBool(fp);
			pMat->SetTransparent(b);
			break;
		default:
			// unknown chunk, skip it
			fseek(fp, len, SEEK_CUR);
			break;
		}
	}
	return pMat;
}

vtMesh *InputSOG::ReadMesh(FILE *fp)
{
	int i, j;
	short token, len;
	short components;
	short vtype, ptype, matidx, verts, indices, prims;
	short vcomponents;
	FPoint3 p;
	RGBf rgb;
	FPoint2 uv;
	vtArray<short> Index;
	int quiet;

	Read(fp, token, len);
	assert(token == FT_MESH);
	quiet = fread(&components, 2, 1, fp);

	// first 8 components are required
	Read(fp, token, len);
	assert(token == FT_VTX_FLAGS);
	quiet = fread(&vtype, 2, 1, fp);

	Read(fp, token, len);
	assert(token == FT_PRIM_TYPE);
	quiet = fread(&ptype, 2, 1, fp);

	Read(fp, token, len);
	assert(token == FT_MAT_INDEX);
	quiet = fread(&matidx, 2, 1, fp);

	Read(fp, token, len);
	assert(token == FT_NUM_VERTICES);
	quiet = fread(&verts, 2, 1, fp);

	// we now have enough information to start mesh construction
	vtMesh *pMesh = new vtMesh((vtMesh::PrimType)ptype, vtype, verts);
	pMesh->SetMatIndex(matidx);

	Read(fp, token, len);
	assert(token == FT_NUM_INDICES);
	quiet = fread(&indices, 2, 1, fp);
	Index.SetSize(indices);

	Read(fp, token, len);
	assert(token == FT_NUM_PRIMITIVES);
	quiet = fread(&prims, 2, 1, fp);

	Read(fp, token, len);
	assert(token == FT_VTX_COMPONENTS);
	quiet = fread(&vcomponents, 2, 1, fp);

	Read(fp, token, len);
	assert(token == FT_VTX_ARRAY);

	for (i = 0; i < verts; i++)
	{
		for (j = 0; j < vcomponents; j++)
		{
			Read(fp, token, len);
			switch (token)
			{
			case FT_VTX_POS:
				quiet = fread(&p, sizeof(FPoint3), 1, fp);
				pMesh->SetVtxPos(i, p);
				break;
			case FT_VTX_NORMAL:
				quiet = fread(&p, sizeof(FPoint3), 1, fp);
				pMesh->SetVtxNormal(i, p);
				break;
			case FT_VTX_COLOR:
				quiet = fread(&rgb, sizeof(RGBf), 1, fp);
				pMesh->SetVtxColor(i, rgb);
				break;
			case FT_VTX_COORD1:
				quiet = fread(&uv, sizeof(FPoint2), 1, fp);
				pMesh->SetVtxTexCoord(i, uv);
				break;
			default:
				// unknown chunk, skip it
				fseek(fp, len, SEEK_CUR);
				break;
			}
		}
	}

	for (i = 0; i < components - 8; i++)
	{
		Read(fp, token, len);
		switch (token)
		{
		case FT_INDEX_ARRAY:
			quiet = fread(Index.GetData(), 2, indices, fp);
			break;

		case FT_PRIM_LEN_ARRAY:
			unsigned short *indices = (unsigned short *)Index.GetData();
			for (j = 0; j < prims; j++)
			{
				quiet = fread(&len, 2, 1, fp);
				pMesh->AddStrip(len, indices);
				indices += len;
			}
			break;
		}
	}
	return pMesh;
}


bool InputSOG::ReadContents(FILE *fp, vtGroup *Parent)
{
	int j;
	char buf[5];
	short num_mat, num_geom, version;
	int quiet;

	// read file type identifier
	quiet = fread(buf, 4, 1, fp);
	buf[4] = 0;
	if (strcmp(buf, SOG_HEADER))
		return false;

	short token, len;
	Read(fp, token, len);
	if (token != FT_VERSION)
		return false;
	quiet = fread(&version, 2, 1, fp);

	// read materials
	Read(fp, token, len);
	if (token != FT_NUM_MATERIALS)
		return false;
	quiet = fread(&num_mat, 2, 1, fp);

	vtMaterialArrayPtr	pMats = new vtMaterialArray;
	vtMaterial *pMat;

	for (j = 0; j < num_mat; j++)
	{
		pMat = ReadMaterial(fp);
		pMats->AppendMaterial(pMat);
	}

	// read geometries
	Read(fp, token, len);
	if (token != FT_NUM_GEOMETRIES)
		return false;
	quiet = fread(&num_geom, 2, 1, fp);
	for (j = 0; j < num_geom; j++)
	{
		vtGeode *pGeode = ReadGeometry(fp, pMats);
		Parent->addChild(pGeode);
	}

	return true;
}

vtGeode *InputSOG::ReadGeometry(FILE *fp, vtMaterialArray *pMats)
{
	short num_mesh, components;
	short token, len, i, j;
//	bool eof = false;
	int quiet;

	Read(fp, token, len);
	assert(token == FT_GEOMETRY);
	quiet = fread(&components, 2, 1, fp);

	vtGeode *pGeode = new vtGeode;
	vtMesh *pMesh;

	pGeode->SetMaterials(pMats);

	for (i = 0; i < components; i++)
	{
		if (!Read(fp, token, len))
			return NULL;
		switch (token)
		{
		case FT_NUM_MESHES:
			quiet = fread(&num_mesh, 2, 1, fp);
			for (j = 0; j < num_mesh; j++)
			{
				pMesh = ReadMesh(fp);
				pGeode->AddMesh(pMesh, pMesh->GetMatIndex());
			}
			break;
		case FT_GEOMNAME:
		case FT_GEOMID:
		case FT_GEOMPARENTID:
		default:
			// unknown chunk, skip it
			fseek(fp, len, SEEK_CUR);
		}
	}
	return pGeode;
}

