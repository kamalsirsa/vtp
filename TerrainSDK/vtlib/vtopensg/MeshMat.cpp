//
// MeshMat.cpp - Meshes and Materials for vtlib-OpenSG
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"

#include <OpenSG/OSGSimpleAttachments.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGDynamicVolume.h>
#include <OpenSG/OSGPolygonChunk.h>
#include <OpenSG/OSGLineChunk.h>
#include <OpenSG/OSGFresnelMaterial.h>
#include <OpenSG/OSGPhongMaterial.h>
#include <OpenSG/OSGPathHandler.h>
#include <OpenSG/OSGFontStyleFactory.h>
#include <OpenSG/OSGFontStyle.h>
#include <OpenSG/OSGSharedFontStyle.h>

#include <string>

// Static members
bool vtMaterial::s_bTextureCompression = false;


///////////////////////////////////

vtMaterial::vtMaterial() : vtMaterialBase()
{
	m_pMaterial = osg::SimpleTexturedMaterial::create();
	
	//if i turn this on buildings are correct, if out, the streets are correct..
	//theres an alpha channel in the road/tree textures
	beginEditCP(m_pMaterial);
	m_pMaterial->setEnvMode			(GL_MODULATE); 
	m_pMaterial->setColorMaterial	(GL_AMBIENT_AND_DIFFUSE);
	m_pMaterial->setEnvMap			(false);
	endEditCP(m_pMaterial);

	SetDiffuse(1,1,1);

}

vtMaterial::~vtMaterial()
{
	osg::subRefCP(m_pMaterial);
}

/**
 * Set the diffuse color of this material.
 *
 * \param r,g,b	The rgb value (0.0 to 1.0) of this material
 * \param a		For a material with transparency enabled, the alpha component
 * of the diffuse color determines the overall transparency of the material.
 * This value ranges from 0 (totally transparent) to 1 (totally opaque.)
 *
 */
void vtMaterial::SetDiffuse(float r, float g, float b, float a)
{
/*#if EXCEPT
	m_pMaterial->setDiffuse(FAB, Vec4(r, g, b, a));

	if (a < 1.0f)
		m_pStateSet->setMode(GL_BLEND, SA_ON);
#endif*/
	

	beginEditCP(m_pMaterial);
	m_pMaterial->setDiffuse(osg::Color3f(r,g,b));
	if (a>0.01f) m_pMaterial->setTransparency(1.f-a);
	endEditCP(m_pMaterial);
}
/**
 * Get the diffuse color of this material.
 */
RGBAf vtMaterial::GetDiffuse() const
{
	osg::Real32 a = m_pMaterial->getTransparency();
	osg::Color3f rgb = m_pMaterial->getDiffuse();
	return RGBAf(rgb.red(), rgb.green(), rgb.blue(), 1-a);
}

/**
 * Set the specular color of this material.
 */
void vtMaterial::SetSpecular(float r, float g, float b)
{
	beginEditCP(m_pMaterial);
	m_pMaterial->setSpecular(osg::Color3f(r,g,b));
	endEditCP(m_pMaterial);
}
/**
 * Get the specular color of this material.
 */
RGBf vtMaterial::GetSpecular() const
{
	osg::Color3f rgb = m_pMaterial->getSpecular();
	return RGBf(rgb.red(), rgb.green(), rgb.blue());
}

/**
 * Set the ambient color of this material.
 */
void vtMaterial::SetAmbient(float r, float g, float b)
{
	beginEditCP(m_pMaterial);
	m_pMaterial->setAmbient(osg::Color3f(r,g,b));
	endEditCP(m_pMaterial);
}
/**
 * Get the ambient color of this material.
 */
RGBf vtMaterial::GetAmbient() const
{
	osg::Color3f rgb = m_pMaterial->getAmbient();
	return RGBf(rgb.red(), rgb.green(), rgb.blue());
}

/**
 * Set the emissive color of this material.
 */
void vtMaterial::SetEmission(float r, float g, float b)
{
	beginEditCP(m_pMaterial);
	m_pMaterial->setEmission(osg::Color3f(r,g,b));
	endEditCP(m_pMaterial);
}
/**
 * Get the emissive color of this material.
 */
RGBf vtMaterial::GetEmission() const
{
	osg::Color3f rgb = m_pMaterial->getEmission();
	return RGBf(rgb.red(), rgb.green(), rgb.blue());
}

/**
 * Set the backface culling property of this material.
 */
void vtMaterial::SetCulling(bool bCulling)
{
	int slot(0);
	osg::StateChunkPtr statechunk = m_pMaterial->find( osg::PolygonChunk::getClassType(), slot);
	osg::PolygonChunkPtr pchunk = osg::PolygonChunkPtr::dcast( statechunk );

	if( !pchunk ) {
		pchunk = osg::PolygonChunk::create();
		beginEditCP(m_pMaterial);
		m_pMaterial->addChunk(pchunk, slot);
		endEditCP(m_pMaterial);
		return;
	}

	beginEditCP(pchunk);
	pchunk->setCullFace(bCulling ? GL_CULL_FACE : GL_FRONT_AND_BACK);
	endEditCP(pchunk);
}
/**
 * Get the backface culling property of this material.
 */
bool vtMaterial::GetCulling() const
{
	int slot(0);
	osg::StateChunkPtr statechunk = m_pMaterial->find( osg::PolygonChunk::getClassType(), slot);
	osg::PolygonChunkPtr pchunk = osg::PolygonChunkPtr::dcast( statechunk );

	if( pchunk ) {
		return pchunk->getSFCullFace()->getValue() == GL_CULL_FACE;
	}

	return false;
}

/**
 * Set the lighting property of this material.
 */
void vtMaterial::SetLighting(bool bLighting)
{
	beginEditCP(m_pMaterial);
	m_pMaterial->setLit(bLighting);
	endEditCP(m_pMaterial);
}
/**
 * Get the lighting property of this material.
 */
bool vtMaterial::GetLighting() const
{
	return m_pMaterial->getLit();
}

/**
 * Set the transparent property of this material.
 *
 * \param bOn True to turn on transparency (blending).
 * \param bAdd True for additive blending.
 */
void vtMaterial::SetTransparent(bool bOn, bool bAdd)
{
	
	//all this seems to affect only the sun ?!
	if ( !m_pMaterial->getLit() ) {
		beginEditCP(m_pMaterial);
		int slot(0);
		osg::StateChunkPtr statechunk = m_pMaterial->find( osg::BlendChunk::getClassType(), slot);
		osg::BlendChunkPtr bchunk = osg::BlendChunkPtr::dcast( statechunk );
		if (!bchunk && bOn ) {
				bchunk = osg::BlendChunk::create();
				m_pMaterial->addChunk(bchunk, slot);
		}

		if( bOn) {
			beginEditCP(bchunk);
			bchunk->setAlphaFunc(GL_GEQUAL);
			//TODO bchunk-> how to set the reference value ??
			endEditCP(bchunk);
		} else {
			//simply remove blend chunk ...
			m_pMaterial->subChunk(bchunk, slot);
		}

		if( bAdd ) {
			beginEditCP(bchunk);
			bchunk->setSrcFactor(GL_ONE);
			bchunk->setDestFactor(GL_ONE_MINUS_SRC_COLOR);
			endEditCP(bchunk);
		}
		endEditCP(m_pMaterial);
	}
}

/**
 * Get the transparent property of this material.
 */
bool vtMaterial::GetTransparent() const
{
	return m_pMaterial->getTransparency() > .01f;
}

osg::StatePtr vtMaterial::GetState() const
{
	return m_pMaterial->getState();
}

/**
 * Set the wireframe property of this material.
 *
 * \param bOn True to turn on wireframe.
 */
void vtMaterial::SetWireframe(bool bOn)
{
/*#if EXCEPT
	if (bOn)
	{
		PolygonMode *pm = new PolygonMode();
		pm->setMode(PolygonMode::FRONT_AND_BACK, PolygonMode::LINE);
		m_pStateSet->setAttributeAndModes(pm, StateAttribute::OVERRIDE | SA_ON);
	}
	else
	{
		// turn wireframe off
		m_pStateSet->setMode(GL_POLYGON_MODE, StateAttribute::OFF);
	}
#endif*/

}
/**
 * Get the wireframe property of this material.
 */
bool vtMaterial::GetWireframe() const
{
#if EXCEPT
	// OSG 0.9.0
	StateAttribute::GLModeValue m;
	m = m_pStateSet->getMode(StateAttribute::POLYGONMODE);
	return(m == SA_ON);
#endif //EXCEPT
	return false;
}

/**
 * Set the texture for this material.
 */
void vtMaterial::SetTexture(vtImage *pImage)
{

	beginEditCP(m_pMaterial);
	m_pMaterial->setImage(pImage->GetImage());
	endEditCP(m_pMaterial);

	m_pImage = pImage;


	/** "Note, If the mode is set USE_IMAGE_DATA_FORMAT, USE_ARB_COMPRESSION,
	 * USE_S3TC_COMPRESSION the internalFormat is automatically selected, and
	 * will overwrite the previous _internalFormat. */
//	m_pTexture->setInternalFormatMode(osg::Texture::USE_S3TC_DXT1_COMPRESSION);
	//if( s_bTextureCompression )
		//m_pTexture->setInternalFormatMode(osg::Texture::USE_ARB_COMPRESSION);

	// From the OSG list: "Why doesn't the OSG deallocate image buffer right
	// *after* a glTextImage2D?
	// By default the OSG doesn't do it bacause the user may actually want to
	// do things with the image after its been bound.  You can make the
	// osg::Texture classes unref their images automatically by doing:
	// texture->setUnRefImageDataAfterApply(true);


}

/**
 * Loads and sets the texture for a material.
 */
bool vtMaterial::SetTexture2(const char *szFilename)
{
	vtImage *image = vtImageRead(szFilename);
	if( image ) {
		SetTexture(image);
		image->Release();	// don't hold on to it; pass ownership to the material
	}
	return(image != NULL);
}


/**
 * Returns the texture (image) associated with a material.
 */
vtImage *vtMaterial::GetTexture() const
{
	// It is valid to return a non-const pointer to the image, since the image
	//  can be modified entirely independently of the material.
	return const_cast<vtImage*>(m_pImage);
}

/**
 * Call this method to tell vtlib that you have modified the contents of a
 *  texture so it needs to be sent again to the graphics card.
 */
void vtMaterial::ModifiedTexture()
{
	if( m_pMaterial->getImage() == osg::NullFC ) return;

	int slot(0);
	osg::StateChunkPtr statechunk = m_pMaterial->find( osg::TextureChunk::getClassType(), slot);
	osg::TextureChunkPtr tex = osg::TextureChunkPtr::dcast( statechunk );

	if( tex ) tex->imageContentChanged();
}


/**
 * Set the texture clamping property for this material.
 */
void vtMaterial::SetClamp(bool bClamp)
{
	if( m_pMaterial->getImage() == osg::NullFC ) {
		return;
	}

	int slot(0);
	osg::StateChunkPtr statechunk = m_pMaterial->find( osg::TextureChunk::getClassType(), slot);
	osg::TextureChunkPtr tex = osg::TextureChunkPtr::dcast( statechunk );

	//there is no use setting clamp if there is no texture..
	if( !tex ) {
		//tex = osg::TextureChunk::create();
		//beginEditCP(m_pMaterial);
		//m_pMaterial->addChunk(tex,osg::State::AutoSlotReplace);
		//endEditCP(m_pMaterial);
	} else {
		beginEditCP(tex);
		if( bClamp ) {
			tex->setWrapS(GL_CLAMP);
			tex->setWrapT(GL_CLAMP);
		} else {
			tex->setWrapS(GL_REPEAT);
			tex->setWrapT(GL_REPEAT);
		}
		endEditCP(tex);
	}
}

/**
 * Get the texture clamping property of this material.
 */
bool vtMaterial::GetClamp() const
{
	if( m_pMaterial->getImage() == osg::NullFC ) {
		return false;
	}

	int slot(0);
	osg::StateChunkPtr statechunk = m_pMaterial->find( osg::TextureChunk::getClassType(), slot);
	osg::TextureChunkPtr tex = osg::TextureChunkPtr::dcast( statechunk );
	if( tex ) {
		return tex->getWrapS() == GL_CLAMP; 
	}
	return false;

}

/**
 * Set the texture mipmapping property for this material.
 */
void vtMaterial::SetMipMap(bool bMipMap)
{
	if( m_pMaterial->getImage() == osg::NullFC )
		return;

	if( bMipMap ) {
		beginEditCP(m_pMaterial);
		m_pMaterial->setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
		endEditCP(m_pMaterial);
	} else {
		beginEditCP(m_pMaterial);
		m_pMaterial->setMinFilter(GL_LINEAR);
		endEditCP(m_pMaterial);
	}
}

/**
 * Get the texture mipmapping property of this material.
 */
bool vtMaterial::GetMipMap() const
{
	if( m_pMaterial->getImage() == osg::NullFC ) {
		return false;
	}
	return m_pMaterial->getMinFilter() == GL_LINEAR_MIPMAP_LINEAR;
}


/////////////////////////////////////////////////////////////////////////////
// vtMaterialArray
//

vtMaterialArray::vtMaterialArray()
{
#if EXCEPT
	ref();		// artficially set refcount to 1
#endif
}

vtMaterialArray::~vtMaterialArray()
{
}

void vtMaterialArray::Release()
{
#if EXCEPT
	unref();	// trigger self-deletion if no more references
#endif
}

/**
 * Adds a material to this material array.
 *
 * \return The index of the material that was added.
 */
int vtMaterialArray::AppendMaterial(vtMaterial *pMat)
{
	// nothing special to do
	return Append(pMat);
}


/////////////////////////////////////////////////////////////////////////////
// vtMesh
//

/**
 * Construct a Mesh.
 * A Mesh is a container for a set of vertices and primitives.
 *
 * \param ePrimType The type of primitive this mesh will contain.  Allowed
 *		values are:
 *		- GL_POINTS
 *		- GL_LINES
 *		- GL_LINE_STRIP
 *		- GL_TRIANGLES
 *		- GL_TRIANGLE_STRIP
 *		- GL_TRIANGLE_FAN
 *		- GL_QUADS
 *		- GL_POLYGON
 *
 * \param VertType Flags which indicate what type of information is stored
 *		with each vertex.  This can be any combination of the following bit
 *		flags:
 *		- VT_Normals - a normal per vertex.
 *		- VT_Colors - a color per vertex.
 *		- VT_TexCoords - a texture coordinate (UV) per vertex.
 *
 * \param NumVertices The expected number of vertices that the mesh will
 *		contain.  If more than this number of vertices are added, the mesh
 *		will automatically grow to contain them.  However it is more
 *		efficient if you know the number at creation time and pass it in
 *		this parameter.
 */
vtMesh::vtMesh(enum PrimType ePrimType, int VertType, int NumVertices) :
vtMeshBase(ePrimType, VertType, NumVertices)
{
	m_pGeometryNode = osg::makeCoredNode<osg::Geometry>(&m_pGeometryCore);
	osg::setName(m_pGeometryNode,"vtMesh-Geometry");

	//TODO wrap this in a SetUserData(osg::NodePtr, vtNode*) method
	//add this vtmesh as user data to the osg node
	osg::VoidPAttachmentPtr a = osg::VoidPAttachment::create();
	beginEditCP(a);
	a->getField().setValue(this);
	endEditCP(a);
	beginEditCP(m_pGeometryNode, osg::Node::AttachmentsFieldMask);
	m_pGeometryNode->addAttachment(a);
	endEditCP(m_pGeometryNode, osg::Node::AttachmentsFieldMask);

	//handle only primitive types, the lengths of the vertices go into the lengths field
	m_pPrimSet = osg::GeoPTypesUI8::create();

	beginEditCP( m_pPrimSet, osg::GeoPTypesUI8::GeoPropDataFieldMask );
	switch( ePrimType ) {
		case POINTS:
			m_pPrimSet->addValue(GL_POINTS); 
			break;
		case LINES:
			m_pPrimSet->addValue(GL_LINES);
			break;
		case TRIANGLES:
			m_pPrimSet->addValue(GL_TRIANGLES);
			break;
		case QUADS:
			m_pPrimSet->addValue(GL_QUADS);
			break;
		case LINE_STRIP:
			m_pPrimSet->addValue(GL_LINE_STRIP);
			break;
		case TRIANGLE_STRIP:
			m_pPrimSet->addValue(GL_TRIANGLE_STRIP);
			break;
		case TRIANGLE_FAN:
			m_pPrimSet->addValue(GL_TRIANGLE_FAN);
			break;
		case POLYGON:
			m_pPrimSet->addValue(GL_POLYGON);
			break;
	}
	endEditCP( m_pPrimSet, osg::GeoPTypesUI8::GeoPropDataFieldMask );

	//make room for the expected # of vertices
	m_Vert = osg::GeoPositions3f::create();	//the vertices are directly stored in the scenegraph
	beginEditCP(m_Vert, osg::GeoPositions3f::GeoPropDataFieldMask);

	// We would like to pre-allocate room for NumVertices vertices.  However,
	//  there is no 'reserve' method available in OpenSG!
	//m_Vert->reserve( NumVertices );
	endEditCP(m_Vert, osg::GeoPositions3f::GeoPropDataFieldMask); 

	//no, this duplicates the first primitive, for time being leave it as it is. first prim. has zero length
	m_Length = osg::GeoPLengthsUI32::create();
	beginEditCP(m_Length, osg::GeoPLengthsUI32::GeoPropDataFieldMask);
	m_Length->addValue(0); 
	endEditCP(m_Length, osg::GeoPLengthsUI32::GeoPropDataFieldMask);

	m_Index = osg::GeoIndicesUI32::create();
	//we don't have any indices yet
	//beginEditCP(m_Length, osg::GeoIndicesUI32::GeoPropDataFieldMask);
	//endEditCP(m_Length, osg::GeoIndicesUI32::GeoPropDataFieldMask);

	beginEditCP(m_pGeometryCore, osg::Geometry::TypesFieldMask     |
				osg::Geometry::LengthsFieldMask   |
				osg::Geometry::PositionsFieldMask |
				osg::Geometry::MaterialFieldMask  |
				osg::Geometry::IndicesFieldMask );
	{
		m_pGeometryCore->setTypes    (m_pPrimSet);
		m_pGeometryCore->setLengths  (m_Length);
		m_pGeometryCore->setPositions(m_Vert);
		
		//do index not for polys..
		if (ePrimType!=POLYGON) m_pGeometryCore->setIndices (m_Index);

		/*if (!(VertType & VT_Colors)) {
			m_pGeometryCore->setMaterial( osg::SimpleTexturedMaterial::create() );
		}*/
	}
    endEditCP  (m_pGeometryCore, osg::Geometry::TypesFieldMask     |
				osg::Geometry::LengthsFieldMask   |
				osg::Geometry::PositionsFieldMask |
				osg::Geometry::MaterialFieldMask  |
				osg::Geometry::IndicesFieldMask  );


	//TODO colors, texcoords, normals, see above
	if( VertType & VT_Normals ) {
		//TODO for the time being, single indexing is used, this defaults to a PER_VERTEX mapping
		m_Norm = osg::GeoNormals3f::create();
		//m_Norm->reserve(NumVertices); //make room for expected Normals
		beginEditCP(m_pGeometryCore);
		m_pGeometryCore->setNormals(m_Norm);
		endEditCP(m_pGeometryCore);
	}

	if( VertType & VT_Colors )
	//TODO for the time being, single indexing is used, this defaults to a PER_VERTEX mapping
	{
		m_Color = osg::GeoColors3f::create();
		//m_Color->reserve(NumVertices);
		beginEditCP(m_pGeometryCore);
		m_pGeometryCore->setColors(m_Color);
		endEditCP(m_pGeometryCore);
	}

	if( VertType & VT_TexCoords ) {
		m_Tex = osg::GeoTexCoords2f::create();
		//m_Tex->reserve(NumVertices);
		beginEditCP(m_pGeometryCore);
		m_pGeometryCore->setTexCoords(m_Tex);
		endEditCP(m_pGeometryCore);
	}
}

void vtMesh::Release()
{ //TODO release vtMesh
#if EXCEPT
	unref();	// trigger self-deletion if no more references
	if( _refCount == 1 ) // no more references except from m_pGeometry
		// explicit dereference. if Release is not called, this dereference should
		//  also occur implicitly in the destructor
		m_pGeometry = NULL;
#endif
}


// Override with ability to get OSG bounding box
void vtMesh::GetBoundBox(FBox3 &box) const
{
	osg::DynamicVolume vol;
	vol = m_pGeometryNode->getVolume(true); //no update
	vol.morphToType(osg::DynamicVolume::BOX_VOLUME);
	osg::Pnt3f min, max,cen;
	vol.getBounds(min, max);
//	vol.getCenter( cen );

	box.max = s2v ( max.subZero() );
	box.min = s2v ( min.subZero() ); 
}


/**
 * Add a triangle.
 *  p0, p1, p2 are the indices of the vertices of the triangle.
 */
void vtMesh::AddTri(int p0, int p1, int p2)
{
/*
#if EXCEPT
	DrawArrays *pDrawArrays = dynamic_cast<DrawArrays*>(m_pPrimSet.get());
	m_Index->push_back(p0);
	m_Index->push_back(p1);
	m_Index->push_back(p2);
	pDrawArrays->setCount(m_Index->size());
#endif */

	//add indices
	beginEditCP( m_Index/*TODO osg::GeoIndices::GeoPropDataFieldMask*/ );
	m_Index->push_back(p0);
	m_Index->push_back(p1);
	m_Index->push_back(p2);
	endEditCP( m_Index/*, osg::GeoIndices::GeoPropDataFieldMask*/ );
	//adjust length
	osg::UInt32 l = m_Index->getSize();
	beginEditCP( m_Length );
	m_Length->setValue(l,0); 
	endEditCP(m_Length);
}

/**
 * Add a triangle fan with up to 6 points (center + 5 points).  The first 3
 * arguments are required, the rest are optional.  A fan will be created
 * with as many point indices as you pass.
 */
void vtMesh::AddFan(int p0, int p1, int p2, int p3, int p4, int p5)
{
/*
#if EXCEPT
	DrawArrayLengths *pDrawArrayLengths = dynamic_cast<DrawArrayLengths*>(m_pPrimSet.get());
	int len = 2;

	m_Index->push_back(p0);
	m_Index->push_back(p1);

	if (p2 != -1) { m_Index->push_back(p2); len = 3; }
	if (p3 != -1) { m_Index->push_back(p3); len = 4; }
	if (p4 != -1) { m_Index->push_back(p4); len = 5; }
	if (p5 != -1) { m_Index->push_back(p5); len = 6; }

	pDrawArrayLengths->push_back(len);
#endif */

	int len = 2;

	beginEditCP( m_Index );
	m_Index->push_back(p0);
	m_Index->push_back(p1);
	endEditCP( m_Index );

	if( p2 != -1 ) {
		beginEditCP( m_Index );
		m_Index->push_back(p2); 
		endEditCP( m_Index );
		len = 3; 
	}
	if( p3 != -1 ) {
		beginEditCP( m_Index );
		m_Index->push_back(p3); 
		endEditCP( m_Index );
		len = 4; 
	}
	if( p4 != -1 ) {
		beginEditCP( m_Index );
		m_Index->push_back(p4); 
		endEditCP( m_Index );
		len = 5; 
	}
	if( p5 != -1 ) {
		beginEditCP( m_Index );
		m_Index->push_back(p5);
		endEditCP( m_Index );
		len = 6; 
	}

	//create new primitive
	beginEditCP(m_Length);
	m_Length->push_back(len);
	endEditCP(m_Length);

	beginEditCP(m_pPrimSet);
	m_pPrimSet->push_back(m_pPrimSet->getValue(0));
	endEditCP(m_pPrimSet);
}

/**
 * Add a triangle fan with any number of points.
 *	\param idx An array of vertex indices for the fan.
 *	\param iNVerts the number of vertices in the fan.
 */
void vtMesh::AddFan(int *idx, int iNVerts)
{
/*
#if EXCEPT
	DrawArrayLengths *pDrawArrayLengths = dynamic_cast<DrawArrayLengths*>(m_pPrimSet.get());
	if (!pDrawArrayLengths)
	{
		VTLOG("Error, calling AddFan when primtype is %d\n", m_ePrimType);
		return;
	}
	for (int i = 0; i < iNVerts; i++)
		m_Index->push_back(idx[i]);

	pDrawArrayLengths->push_back(iNVerts);
#endif */

	if( m_Length == osg::NullFC ) {
		VTLOG("Error, calling AddFan when primtype is %d\n", m_ePrimType);
		return;
	}

	//store indices
	for( int i = 0; i < iNVerts; ++i ) {

		beginEditCP( m_Index/*TODO osg::GeoIndices::GeoPropDataFieldMask*/ );
		m_Index->push_back(idx[i]); 
		endEditCP( m_Index/*TODO osg::GeoIndices::GeoPropDataFieldMask*/ );
	}

	
	//add new primitive length
	beginEditCP(m_Length);
	m_Length->addValue(iNVerts);
	endEditCP(m_Length);

	//create new primitive
	beginEditCP(m_pPrimSet);
	m_pPrimSet->addValue( m_pPrimSet->getValue(0));
	endEditCP(m_pPrimSet);
}

/**
 * Adds an indexed triangle strip to the mesh.
 *
 * \param iNVerts The number of vertices in the strip.
 * \param pIndices An array of the indices of the vertices in the strip.
 */
void vtMesh::AddStrip(int iNVerts, unsigned short *pIndices)
{
/*
#if EXCEPT
	DrawArrayLengths *pDrawArrayLengths = dynamic_cast<DrawArrayLengths*>(m_pPrimSet.get());
	if (!pDrawArrayLengths)
	{
		VTLOG("Error, calling AddStrip when primtype is %d\n", m_ePrimType);
		return;
	}
	for (int i = 0; i < iNVerts; i++)
		m_Index->push_back(pIndices[i]);

	pDrawArrayLengths->push_back(iNVerts);
#endif */

	//store indices
	for( int i = 0; i < iNVerts; ++i ) {

		beginEditCP( m_Index/*TODO osg::GeoIndices::GeoPropDataFieldMask*/ );
		m_Index->push_back( pIndices[i]);   
		endEditCP( m_Index/*TODO osg::GeoIndices::GeoPropDataFieldMask*/ );
	}
	//the indices are in, but we need to create a new primitive..
	//set the length of the primitive
	beginEditCP(m_Length);
	m_Length->addValue(iNVerts);
	endEditCP(m_Length);
	//since multiple types of primitives could be stored, set also the type of the prim 
	//it is ok to simply copy the type of the first, since all are of the same type in VTP
	beginEditCP(m_pPrimSet);
	m_pPrimSet->addValue(m_pPrimSet->getValue(0));
	endEditCP(m_pPrimSet);
}

/**
 * Add a single line primitive to a mesh.
 *	\param p0, p1	The indices of the two vertices of the line.
 */
void vtMesh::AddLine(int p0, int p1)
{
/*
#if EXCEPT
	m_Index->push_back(p0);
	m_Index->push_back(p1);

	if (m_ePrimType == GL_LINES)
	{
		DrawArrays *pDrawArrays = dynamic_cast<DrawArrays*>(m_pPrimSet.get());
		pDrawArrays->setCount(m_Index->size());
	}
	else if (m_ePrimType == GL_LINE_STRIP)
	{
		DrawArrayLengths *pDrawArrayLengths = dynamic_cast<DrawArrayLengths*>(m_pPrimSet.get());
		pDrawArrayLengths->push_back(2);
	}
#endif */

	beginEditCP( m_Index/*TODO osg::GeoIndices::GeoPropDataFieldMask*/ );
	m_Index->push_back(p0);
	m_Index->push_back(p1);
	endEditCP( m_Index/*, osg::GeoIndices::GeoPropDataFieldMask*/ );

	//TODO check count in addLine
	//adjust index count is ! necessary
	if( m_ePrimType == GL_LINES ) {
		//DrawArrays *pDrawArrays = dynamic_cast<DrawArrays*>(m_pPrimSet.get());
		//pDrawArrays->setCount(m_Index->size());

		//no need to add new primitive, so simply adjust the length
		osg::UInt32 l = m_Length->getValue(0);
		l+=2;
		beginEditCP( m_Length );
		m_Length->setValue(l,0);
		endEditCP(m_Length);
	} else if( m_ePrimType == GL_LINE_STRIP ) {
		//DrawArrayLengths *pDrawArrayLengths = dynamic_cast<DrawArrayLengths*>(m_pPrimSet.get());
		//pDrawArrayLengths->push_back(2);

		//TODO check line strip and line vice versa ?
		beginEditCP(m_Length);
		m_Length->addValue(2);
		endEditCP(m_Length);

		beginEditCP(m_pPrimSet);
		m_pPrimSet->addValue(m_pPrimSet->getValue(0));
		endEditCP(m_pPrimSet);
	}

}

/**
 * Add a single line primitive to a mesh.
 *	\param pos1, pos2	The positions of the two vertices of the line.
 *  \return The index of the first vertex added.
 */
int vtMesh::AddLine(const FPoint3 &pos1, const FPoint3 &pos2)
{
	int p0 = AddVertex(pos1);
	int p1 = AddVertex(pos2);
	AddLine(p0, p1);
	return p0;
}

/**
 * Add a triangle.
 *  p0, p1, p2 are the indices of the vertices of the triangle.
 */
void vtMesh::AddQuad(int p0, int p1, int p2, int p3)
{
/*
#if EXCEPT
	DrawArrays *pDrawArrays = dynamic_cast<DrawArrays*>(m_pPrimSet.get());
	if (!pDrawArrays)
	{
		VTLOG("Error, calling AddQuad when primtype is %d\n", m_ePrimType);
		return;
	}
	m_Index->push_back(p0);
	m_Index->push_back(p1);
	m_Index->push_back(p2);
	m_Index->push_back(p3);
	pDrawArrays->setCount(m_Index->size());
#endif */

	if( m_Length == osg::NullFC ) {
		VTLOG("Error, calling AddQuad when primtype is %d\n", m_ePrimType);
		return;
	}

	beginEditCP( m_Index/*TODO osg::GeoIndices::GeoPropDataFieldMask*/ );
	m_Index->push_back(p0);
	m_Index->push_back(p1);
	m_Index->push_back(p2);
	m_Index->push_back(p3);
	endEditCP( m_Index/*, osg::GeoIndices::GeoPropDataFieldMask*/ );

	//adjust primitive length
	beginEditCP(m_Length);
	m_Length->addValue(4);
	endEditCP(m_Length);

	//create a new quad primitive
	beginEditCP(m_pPrimSet);
	m_pPrimSet->addValue(m_pPrimSet->getValue(0));
	endEditCP(m_pPrimSet);

}

unsigned int vtMesh::GetNumVertices() const
{
	return m_Vert->size();
}

/**
 * Set the position of a vertex.
 *	\param i	Index of the vertex.
 *	\param p	The position.
 */
void vtMesh::SetVtxPos(unsigned int i, const FPoint3 &p)
{
	osg::Vec3f s;
	v2s(p, s);

	beginEditCP(m_Vert, osg::GeoPositions3f::GeoPropDataFieldMask);
	if( i >= m_Vert->size() ) {
		m_Vert->resize(i+1);
	}
	m_Vert->setValue(s,i);
	endEditCP(m_Vert, osg::GeoPositions3f::GeoPropDataFieldMask);

	if( m_ePrimType == GL_POINTS ) {
		beginEditCP( m_Index/*TODO osg::GeoIndices::GeoPropDataFieldMask*/ );
		if( i >= m_Index->size() ) {
			m_Index->resize(i+1);
		}
		m_Index->setValue(i,i);     
		endEditCP( m_Index/*TODO osg::GeoIndices::GeoPropDataFieldMask*/ );

		//# of vertices eq # of indices
		osg::UInt32 l = m_Vert->size();
		beginEditCP( m_Length/*TODO osg::GeoIndices::GeoPropDataFieldMask*/ );
		m_Length->setValue(l,0);
		endEditCP(m_Length);
	}
}

/**
 * Get the position of a vertex.
 */
FPoint3 vtMesh::GetVtxPos(unsigned int i) const
{
	FPoint3 p;
	s2v( m_Vert->getValue(i), p);
	return p;
}

/**
 * Set the normal of a vertex.  This is used for lighting, if the mesh
 *	is used with a material with lighting enabled.  Generally you will
 *	want to use a vector of unit length.
 *
 *	\param i	Index of the vertex.
 *	\param norm	The normal vector.
 */
void vtMesh::SetVtxNormal(unsigned int i, const FPoint3 &norm)
{
	osg::Vec3f s;
	v2s(norm, s);

	if( i >= (int)m_Norm->getSize() )
		m_Norm->resize( i+1 );

	beginEditCP(m_Norm);
	m_Norm->setValue(s,i);
	endEditCP(m_Norm);
}

/**
 * Get the normal of a vertex.
 */
FPoint3 vtMesh::GetVtxNormal(unsigned int i) const
{
	FPoint3 p;
	s2v( m_Norm->getValue(i), p);
	return p;
}

/**
 * Set the color of a vertex.  This color multiplies with the color of the
 *	material used with the mesh, so if you want the vertex color to be
 *	dominant, use a white material.
 *
 *	\param i		Index of the vertex.
 *	\param color	The color.
 */
void vtMesh::SetVtxColor(unsigned int i, const RGBAf &color)
{
	if( m_iVtxType & VT_Colors ) {
		osg::Vec4f s;
		v2s(color, s);
		if( i >= (int)m_Color->size() )
			m_Color->resize(i + 1);
		//TODO alpha is not stored in color
		m_Color->setValue( osg::Color3f(s.x(), s.y(), s.z()), i);
	}

}

/**
 * Get the color of a vertex.
 */
RGBAf vtMesh::GetVtxColor(unsigned int i) const
{
	if( m_iVtxType & VT_Colors ) {
		RGBAf p;
		osg::Color3f col;
		col = m_Color->getValue(i);
		return RGBAf(col.red(), col.green(), col.blue(), 0.);
	} else {
		return RGBAf(0,0,0,0.);
	}
}

/**
 * Set the texture coordinates of a vertex.  Generally these values are
 *	in the range of 0 to 1, although you can use higher values if you want
 *	repeating tiling.  The components of the texture coordinates are
 *  usually called "u" and "v".
 *
 *	\param i	Index of the vertex.
 *	\param uv	The texture coordinate.
 */
void vtMesh::SetVtxTexCoord(unsigned int i, const FPoint2 &uv)
{

	if( m_iVtxType & VT_TexCoords ) {
		osg::Vec2f s;
		v2s(uv, s);

		// Not sure whether I need this
		if( i >= (int)m_Tex->getSize() )
			m_Tex->resize(i + 1);
		m_Tex->setValue(s,i);
	}
}

/**
 * Get the texture coordinates of a vertex.
 */
FPoint2 vtMesh::GetVtxTexCoord(unsigned int i) const
{
	if( m_iVtxType & VT_TexCoords ) {
		FPoint2 p;
		s2v( (osg::Vec2f)m_Tex->getValue(i), p);
		return p;
	}
	return FPoint2(0,0);
}

int vtMesh::GetNumPrims() const
{
	return m_pPrimSet->getSize();
}

/**
 * Set whether to allow rendering optimization of this mesh.  With OpenGL,
 *	this optimization is called a "display list", which increases the speed
 *	of rendering by creating a special representation of the mesh the first
 *	time it is drawn.  The tradeoff is that subsequent changes to the mesh
 *	are not applied unless you call ReOptimize().
 *
 *	\param bAllow	True to allow optimization.  The default is true.
 */
void vtMesh::AllowOptimize(bool bAllow)
{
	beginEditCP(m_pGeometryCore);
	m_pGeometryCore->setDlistCache(bAllow);
	endEditCP(m_pGeometryCore);
}

/**
 * For a mesh with rendering optimization enabled, forces an update of the
 *	optimized representation.
 */
void vtMesh::ReOptimize()
{
	m_pGeometryCore->changed(osg::Geometry::DlistCacheFieldMask, osg::ChangedOrigin::AbstrCheckValid);
}

void vtMesh::SetLineWidth(float fWidth)
{
	osg::SimpleTexturedMaterialPtr mat = osg::SimpleTexturedMaterialPtr::dcast( m_pGeometryCore->getMaterial() );

	if( !mat ) return;

	int slot(0);

	osg::StateChunkPtr statechunk = mat->find( osg::LineChunk::getClassType(), slot);
	osg::LineChunkPtr lchunk = osg::LineChunkPtr::dcast( statechunk );

	beginEditCP(mat);
	if( !lchunk ) {
		lchunk = osg::LineChunk::create();
		mat->addChunk(lchunk);
	}

	beginEditCP(lchunk);
	lchunk->setWidth( fWidth ); 
	endEditCP(lchunk);

	endEditCP(mat);
}

/**
 * Set the normals of the vertices by combining the normals of the
 * surrounding faces.  This requires going through all the primitives
 * to average their contribution to each vertex.
 */
void vtMesh::SetNormalsFromPrimitives()
{
	beginEditCP(m_Norm);
	osg::GeoNormals3f::StoredFieldType *normField = m_Norm->getFieldPtr();
	osg::GeoNormals3f::StoredFieldType::iterator itr;
	for( itr = normField->begin(); itr != normField->end(); ++itr )
		(*itr) = osg::Vec3f(0, 0, 0);
	endEditCP(m_Norm);

	switch( m_ePrimType ) {
		case GL_POINTS:
		case GL_LINES:
		case GL_LINE_STRIP:
		case GL_TRIANGLES:
			break;
		case GL_TRIANGLE_STRIP:
			_AddStripNormals();
			break;
		case GL_TRIANGLE_FAN:
		case GL_QUADS:
		case GL_POLYGON:
			break;
	}

	beginEditCP(m_Norm);
	for( itr = normField->begin(); itr != normField->end(); ++itr )
		itr->normalize();
	endEditCP(m_Norm);

}

void vtMesh::_AddStripNormals()
{
	int prims = GetNumPrims();
	int i, j, len, idx;
	unsigned short v0 = 0, v1 = 0, v2 = 0;
	osg::Vec3f p0, p1, p2, d0, d1, norm;

	idx = 0;
	for( i = 0; i < prims; i++ ) {
		len = m_Length->getValue(i);
		for( j = 0; j < len; j++ ) {
			v0 = v1; p0 = p1;
			v1 = v2; p1 = p2;
			v2 = m_Index->getValue(idx);
			p2 = m_Vert->getValue(v2);
			if( j >= 2 ) {
				d0 = (p1 - p0);
				d1 = (p2 - p0);
				d0.normalize();
				d1.normalize();

				//norm = d0^d1;
				norm = d0.cross(d1);

				osg::Vec3f v0t, v1t, v2t;
				v0t = m_Norm->getValue(v0);
				v1t = m_Norm->getValue(v1);
				v2t = m_Norm->getValue(v2);

				v0t += norm;
				v1t += norm;
				v2t += norm;

				beginEditCP(m_Norm);
				m_Norm->setValue(v0t, v0);
				m_Norm->setValue(v1t, v1);
				m_Norm->setValue(v2t, v2);
				endEditCP(m_Norm);
			}
			idx++;
		}
	}
}



/////////////////////////////////////////////////////////////////////////////
// Text

vtFont::vtFont()
{
}

vtFont::~vtFont()
{
	// no need to free m_pOsgFont, it is a ref_ptr
}

bool vtFont::LoadFont(const char *filename)
{
	osg::PathHandler paths;
	paths.push_backPath(".");
	//need to have freetype1 for ttf support..
	m_pFontStyle = osg::FontStyleFactory::the().create( paths, filename, 1.f );
	m_pFontStyle->setXRes(16);
	m_pFontStyle->setYRes(16);
	return m_pFontStyle != 0;
}

////

/**
 * Construct a TextMesh object.
 *
 * \param font The font that will be used to draw the text.
 * \param fSize Size (height) in world units of the text rectangle.
 * \param bCenter If true, the origin of the text rectangle is at
 *			it's bottom center.  Otherwise, bottom left.
 */
vtTextMesh::vtTextMesh(vtFont *font, float fSize, bool bCenter)
{
	//this is not ok, since we can only have an instance of one font at a unique size
	//but creating copies of the fontstyle here would be even worse...
	//for markers though one size should be fine.
	//font resolution is now in vtFont
	font->GetFontStyle()->setSize( fSize );

	m_pSharedFontStyle = osg::SharedFontStyle::create(); 
	m_pSharedFontStyle->setContainedFontStyle( font->GetFontStyle() );

	m_pSharedFontStyleWrapper = osg::SharedFontStyleWrapper::create();
	m_pSharedFontStyleWrapper->setFStyleContainer( m_pSharedFontStyle );

	osg::ChunkMaterialPtr mat = osg::ChunkMaterial::create();
	osg::MaterialChunkPtr pmc = osg::MaterialChunk::create();
	pmc->setDiffuse( osg::Color4f( 1,1,1,0 ) );
	pmc->setAmbient( osg::Color4f( 0,1,0,0 ) );
	pmc->setSpecular( osg::Color4f( 0,0,1,0 ) );
	pmc->setLit( false );
	pmc->setShininess( 20 );

	beginEditCP(mat);
	mat->addChunk( pmc );
	mat->setSortKey(1);	//otherwise interference ...
	endEditCP(mat);


	m_pOsgText = osg::ScreenAlignedText::create();
	beginEditCP(m_pOsgText);
	{
		m_pOsgText->setPosition( osg::Vec3f ( 0, 0, 0 ) );
		m_pOsgText->setFont( m_pSharedFontStyleWrapper );
		m_pOsgText->setVerticalLineDistance( 0.20 );
		//alignment: Left = 0, Centered = 1, Right = 2
		m_pOsgText->setAlignment( bCenter ? 1 : 0 );
		m_pOsgText->setMaterial( mat );

	}
	endEditCP(m_pOsgText);

	//don't forget to decorate osg node with vtTextMesh!
	osg::VoidPAttachmentPtr a = osg::VoidPAttachment::create();
	beginEditCP(a);
	a->getField().setValue( this );
	endEditCP(a);

	osg::NodePtr node = osg::Node::create();
	beginEditCP(node, osg::Node::AttachmentsFieldMask);
	node->setCore(m_pOsgText);
	endEditCP(node, osg::Node::AttachmentsFieldMask);

	m_pOsgTextNode = osg::makeCoredNode<osg::Transform>(&m_Transform);
	beginEditCP(m_pOsgTextNode);
	m_pOsgTextNode->addChild(node);
	m_pOsgTextNode->addAttachment(a);
	endEditCP(m_pOsgTextNode);
}

vtTextMesh::~vtTextMesh()
{
}

void vtTextMesh::Release()
{
#if EXCEPT
	unref();
	if( _refCount == 1 ) // no more references except from m_pGeometry
		// explicit dereference. if Release is not called, this dereference should
		//  also occur implicitly in the destructor
		m_pOsgText = NULL;
#endif
}

// Override with ability to get OSG bounding box
void vtTextMesh::GetBoundBox(FBox3 &box) const
{
	osg::DynamicVolume vol;
	vol = m_pOsgTextNode->getVolume(false);	//no update
	vol.morphToType(osg::DynamicVolume::BOX_VOLUME);
	osg::Pnt3f min, max;
	vol.getBounds(min, max);
	box.max = s2v ( max.subZero() );
	box.min = s2v ( min.subZero() );
}


void vtTextMesh::SetText(const char *text)
{
	beginEditCP(m_pOsgText);
	m_pOsgText->getText().push_back( text );
	endEditCP(m_pOsgText);
}

void vtTextMesh::SetText(const wchar_t *text)
{
	wstring2 wstr(text);
	beginEditCP(m_pOsgText);
	m_pOsgText->getText().push_back( wstr.to_utf8() );
	endEditCP(m_pOsgText);
}

#if SUPPORT_WSTRING
void vtTextMesh::SetText(const std::wstring &text)
{
	wstring2 wstr(text.c_str());
	beginEditCP(m_pOsgText);
	m_pOsgText->getText().push_back( wstr.to_utf8() );
	endEditCP(m_pOsgText);
}
#endif //SUPPORT_WSTRING

void vtTextMesh::SetPosition(const FPoint3 &pos)
{
	osg::Vec3f s; 
	v2s(pos, s);
	beginEditCP(m_pOsgText);
	m_pOsgText->setPosition( s );
	endEditCP(m_pOsgText);
}

void vtTextMesh::SetRotation(const FQuat &rot)
{
	osg::Quaternion q; 
	q.setValueAsQuat ( rot.x, rot.y, rot.z, rot.w );

	osg::Matrix mat;
	mat.setIdentity();
	mat.setRotate( q );

	beginEditCP( m_Transform );
	m_Transform->getMatrix() = mat;
	endEditCP( m_Transform );
}

//TODO
void vtTextMesh::SetAlignment(int align)
{
#if EXCEPT
	osgText::Text::AxisAlignment osga;

	if( align == 0 )
		osga = osgText::Text::XY_PLANE;
	else if( align == 1 )
		osga = osgText::Text::XZ_PLANE;
	else if( align == 2 )
		osga = osgText::Text::YZ_PLANE;
	else return;

	m_pOsgText->setAxisAlignment(osga);
#endif
}

void vtTextMesh::SetColor(const RGBAf &rgba)
{
	beginEditCP(m_pOsgText);
	m_pOsgText->setColor( osg::Color4f( rgba.r, rgba.g, rgba.b, rgba.a ) );
	endEditCP(m_pOsgText);
}
