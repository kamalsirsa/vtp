#include <OpenSG/OSGConfig.h>

OSG_BEGIN_NAMESPACE


//! access the type of the class
inline
OSG::FieldContainerType &vtOsgDynMeshBase::getClassType(void)
{
    return _type; 
} 

//! access the numerical type of the class
inline
OSG::UInt32 vtOsgDynMeshBase::getClassTypeId(void) 
{
    return _type.getId(); 
} 

//! create a new instance of the class
inline
vtOsgDynMeshPtr vtOsgDynMeshBase::create(void) 
{
    vtOsgDynMeshPtr fc; 

    if(getClassType().getPrototype() != OSG::NullFC) 
    {
        fc = vtOsgDynMeshPtr::dcast(
            getClassType().getPrototype()-> shallowCopy()); 
    }
    
    return fc; 
}

//! create an empty new instance of the class, do not copy the prototype
inline
vtOsgDynMeshPtr vtOsgDynMeshBase::createEmpty(void) 
{ 
    vtOsgDynMeshPtr returnValue; 
    
    newPtr(returnValue); 

    return returnValue; 
}


/*------------------------------ get -----------------------------------*/

//! Get the Cubes::_mfPosition field.
inline
MFPnt3f *vtOsgDynMeshBase::getMFPosition(void)
{
    return &_mfPosition;
}

//! Get the Cubes::_mfLength field.
inline
MFReal32 *vtOsgDynMeshBase::getMFLength(void)
{
    return &_mfLength;
}

//! Get the Cubes::_mfColor field.
inline
MFColor3f *vtOsgDynMeshBase::getMFColor(void)
{
    return &_mfColor;
}



//! Get the value of the \a index element the Cubes::_mfPosition field.
inline
Pnt3f &vtOsgDynMeshBase::getPosition(const UInt32 index)
{
    return _mfPosition[index];
}

//! Get the Cubes::_mfPosition field.
inline
MFPnt3f &vtOsgDynMeshBase::getPosition(void)
{
    return _mfPosition;
}

//! Get the Cubes::_mfPosition field.
inline
const MFPnt3f &vtOsgDynMeshBase::getPosition(void) const
{
    return _mfPosition;
}

//! Get the value of the \a index element the Cubes::_mfLength field.
inline
Real32 &vtOsgDynMeshBase::getLength(const UInt32 index)
{
    return _mfLength[index];
}

//! Get the Cubes::_mfLength field.
inline
MFReal32 &vtOsgDynMeshBase::getLength(void)
{
    return _mfLength;
}

//! Get the Cubes::_mfLength field.
inline
const MFReal32 &vtOsgDynMeshBase::getLength(void) const
{
    return _mfLength;
}

//! Get the value of the \a index element the Cubes::_mfColor field.
inline
Color3f &vtOsgDynMeshBase::getColor(const UInt32 index)
{
    return _mfColor[index];
}

//! Get the Cubes::_mfColor field.
inline
MFColor3f &vtOsgDynMeshBase::getColor(void)
{
    return _mfColor;
}

//! Get the Cubes::_mfColor field.
inline
const MFColor3f &vtOsgDynMeshBase::getColor(void) const
{
    return _mfColor;
}

OSG_END_NAMESPACE

#define OSGCUBESBASE_INLINE_CVSID "@(#)$Id$"

