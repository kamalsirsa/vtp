//
// LightSpacePerspectiveShadowTechnique.cpp
//
// Implement an OSG shadow technique for vtosg.
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "LightSpacePerspectiveShadowTechnique.h"
#include <osg/Texture3D>

#if OSG_VERSION_MAJOR == 2 && OSG_VERSION_MINOR >= 7

CLightSpacePerspectiveShadowTechnique::CLightSpacePerspectiveShadowTechnique() 
{
	// Override shaders here
	_mainFragmentShader = new osg::Shader( osg::Shader::FRAGMENT,
		" // following expressions are auto modified - do not change them:       \n"
		" // gl_TexCoord[0]  0 - can be subsituted with other index              \n"
		"                                                                        \n"
		"float DynamicShadow( );                                                 \n"
		"                                                                        \n"
		"varying vec4 colorAmbientEmissive;                                      \n"
		"                                                                        \n"
		"uniform sampler2D baseTexture;                                          \n"
		"                                                                        \n"
		"void main(void)                                                         \n"
		"{                                                                       \n"
		"  // A dummy white texture has been applied by default to all geometry  \n"
		"  // so the call to texture2d will always return a valid value          \n"
		"  vec4 color = texture2D( baseTexture, gl_TexCoord[0].xy );             \n"
		"  color *= mix( colorAmbientEmissive, gl_Color, DynamicShadow() );      \n"
		"  gl_FragColor = color;                                                 \n"
		"} \n" ); 

	_mainVertexShader = new osg::Shader( osg::Shader::VERTEX,
		" // following expressions are auto modified - do not change them:      \n"
		" // gl_TexCoord[0]      0 - can be subsituted with other index         \n"
		" // gl_TextureMatrix[0] 0 - can be subsituted with other index         \n"
		" // gl_MultiTexCoord0   0 - can be subsituted with other index         \n"
		"                                                                       \n"    
		"const int NumEnabledLights = 1;                                        \n"
		"                                                                       \n"
		"void DynamicShadow( in vec4 ecPosition );                              \n"
		"                                                                        \n"
		"varying vec4 colorAmbientEmissive;                                     \n"
		"                                                                       \n"
		"uniform int  renderingVTPBaseTexture;                                  \n"
		"                                                                       \n"
		"void SpotLight(in int i,                                               \n" 
		"               in vec3 eye,                                            \n"
		"               in vec3 ecPosition3,                                    \n"
		"               in vec3 normal,                                            \n"
		"               inout vec4 ambient,                                     \n"
		"               inout vec4 diffuse,                                        \n"
		"               inout vec4 specular)                                    \n"
		"{                                                                        \n"
		"    float nDotVP;          // normal . light direction                 \n"
		"    float nDotHV;          // normal . light half vector                \n"
		"    float pf;              // power factor                                \n"
		"    float spotDot;         // cosine of angle between spotlight        \n"
		"    float spotAttenuation; // spotlight attenuation factor             \n"
		"    float attenuation;     // computed attenuation factor                \n"
		"    float d;               // distance from surface to light source    \n"
		"    vec3 VP;               // direction from surface to light position \n"
		"    vec3 halfVector;       // direction of maximum highlights          \n"
		"                                                                        \n"
		"    // Compute vector from surface to light position                    \n"
		"    VP = vec3(gl_LightSource[i].position) - ecPosition3;                \n"
		"                                                                       \n"
		"    // Compute distance between surface and light position                \n"
		"    d = length(VP);                                                    \n"
		"                                                                        \n"
		"    // Normalize the vector from surface to light position             \n"
		"    VP = normalize(VP);                                                \n"
		"                                                                        \n"
		"    // Compute attenuation                                                \n"
		"    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +       \n" 
		"                         gl_LightSource[i].linearAttenuation * d +        \n"
		"                         gl_LightSource[i].quadraticAttenuation *d*d); \n"
		"                                                                        \n"
		"    // See if point on surface is inside cone of illumination          \n"
		"    spotDot = dot(-VP, normalize(gl_LightSource[i].spotDirection));    \n"
		"                                                                        \n"
		"    if (spotDot < gl_LightSource[i].spotCosCutoff)                        \n"
		"        spotAttenuation = 0.0; // light adds no contribution            \n"
		"    else                                                                \n"
		"        spotAttenuation = pow(spotDot, gl_LightSource[i].spotExponent);\n"
		"                                                                        \n"
		"    // Combine the spotlight and distance attenuation.                    \n"
		"    attenuation *= spotAttenuation;                                    \n"
		"                                                                        \n"
		"    halfVector = normalize(VP + eye);                                    \n"
		"                                                                        \n"
		"    nDotVP = max(0.0, dot(normal, VP));                                \n"
		"    nDotHV = max(0.0, dot(normal, halfVector));                        \n"
		"                                                                        \n"
		"    if (nDotVP == 0.0)                                                 \n"
		"        pf = 0.0;                                                        \n"
		"    else                                                                \n"
		"        pf = pow(nDotHV, gl_FrontMaterial.shininess);                    \n"
		"                                                                        \n"
		"    ambient  += gl_LightSource[i].ambient * attenuation;                \n"
		"    diffuse  += gl_LightSource[i].diffuse * nDotVP * attenuation;        \n"
		"    specular += gl_LightSource[i].specular * pf * attenuation;            \n"
		"}                                                                        \n"
		"                                                                        \n"
		"void PointLight(in int i,                                                \n"
		"                in vec3 eye,                                            \n"
		"                in vec3 ecPosition3,                                    \n"
		"                in vec3 normal,                                        \n"
		"                inout vec4 ambient,                                    \n"
		"                inout vec4 diffuse,                                    \n"
		"                inout vec4 specular)                                   \n" 
		"{                                                                        \n"
		"    float nDotVP;      // normal . light direction                        \n"
		"    float nDotHV;      // normal . light half vector                    \n"
		"    float pf;          // power factor                                    \n"
		"    float attenuation; // computed attenuation factor                    \n"
		"    float d;           // distance from surface to light source        \n"
		"    vec3  VP;          // direction from surface to light position        \n"
		"    vec3  halfVector;  // direction of maximum highlights                \n"
		"                                                                        \n"
		"    // Compute vector from surface to light position                    \n"
		"    VP = vec3(gl_LightSource[i].position) - ecPosition3;                \n"
		"                                                                        \n"
		"    // Compute distance between surface and light position                \n"
		"    d = length(VP);                                                    \n"
		"                                                                        \n"
		"    // Normalize the vector from surface to light position             \n" 
		"    VP = normalize(VP);                                                \n"
		"                                                                        \n"
		"    // Compute attenuation                                                \n"
		"    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +        \n"
		"                         gl_LightSource[i].linearAttenuation * d +        \n"
		"                         gl_LightSource[i].quadraticAttenuation * d*d);\n"
		"                                                                        \n"
		"    halfVector = normalize(VP + eye);                                    \n"
		"                                                                        \n"
		"    nDotVP = max(0.0, dot(normal, VP));                                \n"
		"    nDotHV = max(0.0, dot(normal, halfVector));                        \n"
		"                                                                        \n"
		"    if (nDotVP == 0.0)                                                    \n"
		"        pf = 0.0;                                                        \n"
		"    else                                                                \n"
		"        pf = pow(nDotHV, gl_FrontMaterial.shininess);                  \n"
		"                                                                        \n"
		"    ambient += gl_LightSource[i].ambient * attenuation;                \n"
		"    diffuse += gl_LightSource[i].diffuse * nDotVP * attenuation;        \n"
		"    specular += gl_LightSource[i].specular * pf * attenuation;            \n"
		"}                                                                        \n"
		"                                                                        \n"
		"void DirectionalLight(in int i,                                        \n"
		"                      in vec3 normal,                                    \n"
		"                      inout vec4 ambient,                                \n"
		"                      inout vec4 diffuse,                                \n"
		"                      inout vec4 specular)                                \n"
		"{                                                                        \n"
		"     float nDotVP;         // normal . light direction                    \n"
		"     float nDotHV;         // normal . light half vector                \n"
		"     float pf;             // power factor                                \n"
		"                                                                        \n"
		"     nDotVP = max(0.0, dot(normal,                                        \n"
		"                normalize(vec3(gl_LightSource[i].position))));            \n"
		"     nDotHV = max(0.0, dot(normal,                                        \n"
		"                      vec3(gl_LightSource[i].halfVector)));            \n"
		"                                                                        \n"
		"     if (nDotVP == 0.0)                                                \n"
		"         pf = 0.0;                                                        \n"
		"     else                                                                \n"
		"         pf = pow(nDotHV, gl_FrontMaterial.shininess);                    \n"
		"                                                                        \n"
		"     ambient  += gl_LightSource[i].ambient;                            \n"
		"     diffuse  += gl_LightSource[i].diffuse * nDotVP;                    \n"
		"     specular += gl_LightSource[i].specular * pf;                        \n"
		"}                                                                        \n"
		"                                                                        \n"
		"void main( )                                                            \n"
		"{                                                                        \n"
		"    // Transform vertex to clip space                                    \n"
		"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;            \n"
		"    vec3 normal = normalize( gl_NormalMatrix * gl_Normal );            \n"
		"                                                                        \n"
		"    vec4  ecPos  = gl_ModelViewMatrix * gl_Vertex;                        \n"
		"    float ecLen  = length( ecPos );                                    \n"
		"    vec3  ecPosition3 = ecPos.xyz / ecPos.w;                           \n"
		"                                                                       \n" 
		"    vec3  eye = vec3( 0.0, 0.0, 1.0 );                                 \n"
		"    //vec3  eye = -normalize(ecPosition3);                             \n"
		"                                                                       \n"
		"    DynamicShadow( ecPos );                                            \n"
		"                                                                        \n"
		"    if (renderingVTPBaseTexture) \n"
		"    { \n"
		"        gl_TexCoord[0].s = dot(gl_Vertex, gl_ObjectPlaneS[0]); \n" 
		"        gl_TexCoord[0].t = dot(gl_Vertex, gl_ObjectPlaneT[0]); \n" 
		"    } \n"
		"    else \n"
		"        gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;       \n"
		"                                                                        \n"
		"    // Front Face lighting                                                \n"
		"                                                                        \n"
		"    // Clear the light intensity accumulators                            \n"
		"    vec4 amb  = vec4(0.0);                                                \n"
		"    vec4 diff = vec4(0.0);                                                \n"
		"    vec4 spec = vec4(0.0);                                                \n"
		"                                                                        \n"
		"    // Loop through enabled lights, compute contribution from each        \n"
		"    for (int i = 0; i < NumEnabledLights; i++)                          \n"
		"    {                                                                   \n" 
		"        if (gl_LightSource[i].position.w == 0.0)                        \n"
		"            DirectionalLight(i, normal, amb, diff, spec);               \n" 
		"        else if (gl_LightSource[i].spotCutoff == 180.0)                 \n"
		"            PointLight(i, eye, ecPosition3, normal, amb, diff, spec);   \n"   
		"        else                                                            \n"  
		"             SpotLight(i, eye, ecPosition3, normal, amb, diff, spec);    \n" 
		"    }                                                                  \n"
		"                                                                       \n"
		"    // colorAmbientEmissive is used to modulate shaded fragments       \n"
		"    // gl_FrontColor is used to modulate unshaded fragments            \n"
		"    colorAmbientEmissive = gl_FrontLightModelProduct.sceneColor +        \n"
		"                           amb * gl_FrontMaterial.ambient;             \n"       
		"    gl_FrontColor = colorAmbientEmissive +                             \n"
		"                    diff * gl_FrontMaterial.diffuse;                   \n"
		"    if (renderingVTPBaseTexture) \n"
		"    { \n"
		"        // Lighting is off \n"
		"        gl_FrontColor = vec4(1.0, 1.0, 1.0, 1.0); \n"
		"        colorAmbientEmissive = vec4(0.2, 0.2, 0.2, 1.0); \n"
		"    } \n"
		"    gl_FrontSecondaryColor = vec4(spec*gl_FrontMaterial.specular);     \n"
		"                                                                        \n"
		"    gl_BackColor = gl_FrontColor;                                        \n"
		"    gl_BackSecondaryColor = gl_FrontSecondaryColor;                    \n"
		"                                                                        \n"
		"    gl_FogFragCoord = ecLen;                                            \n"
		"} \n" );                                                                
}

void CLightSpacePerspectiveShadowTechnique::ViewData::init(ThisClass * st, osgUtil::CullVisitor * cv)           
{
	BaseClass::ViewData::init( st, cv );
	// Add additional uniforms here
	_stateset->addUniform
		( new osg::Uniform( "renderingVTPBaseTexture", int( 0 ) ) );
}

void CLightSpacePerspectiveShadowTechnique::InitJittering(osg::StateSet *pStateSet)
{
    // create a 3D texture with hw mipmapping
    osg::Texture3D* pJitterTexture = new osg::Texture3D;
    pJitterTexture->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::NEAREST);
    pJitterTexture->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::NEAREST);
    pJitterTexture->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::REPEAT);
    pJitterTexture->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::REPEAT);
    pJitterTexture->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::REPEAT);
    pJitterTexture->setUseHardwareMipMapGeneration(true);

    const unsigned int size = 16;
    const unsigned int gridW =  8;
    const unsigned int gridH =  8;
    unsigned int R = (gridW * gridH / 2);
    pJitterTexture->setTextureSize(size, size, R);

    // then create the 3d image to fill with jittering data
    osg::Image* pJitterImage = new osg::Image;
    unsigned char *pJitterData = new unsigned char[size * size * R * 4];

    for ( unsigned int s = 0; s < size; ++s )
    {
        for ( unsigned int t = 0; t < size; ++t )
        {
            float v[4], d[4];

            for ( unsigned int r = 0; r < R; ++r )
            {
                const int x = r % ( gridW / 2 );
                const int y = ( gridH - 1 ) - ( r / (gridW / 2) );

                // Generate points on a  regular gridW x gridH rectangular
                // grid.   We  multiply  x   by  2  because,  we  treat  2
                // consecutive x  each loop iteration.  Add 0.5f  to be in
                // the center of the pixel. x, y belongs to [ 0.0, 1.0 ].
                v[0] = float( x * 2     + 0.5f ) / gridW;
                v[1] = float( y         + 0.5f ) / gridH;
                v[2] = float( x * 2 + 1 + 0.5f ) / gridW;
                v[3] = v[1];

                // Jitter positions. ( 0.5f / w ) == ( 1.0f / 2*w )
                v[0] += ((float)rand() * 2.f / RAND_MAX - 1.f) * ( 0.5f / gridW );
                v[1] += ((float)rand() * 2.f / RAND_MAX - 1.f) * ( 0.5f / gridH );
                v[2] += ((float)rand() * 2.f / RAND_MAX - 1.f) * ( 0.5f / gridW );
                v[3] += ((float)rand() * 2.f / RAND_MAX - 1.f) * ( 0.5f / gridH );

                // Warp to disk; values in [-1,1]
                d[0] = sqrtf( v[1] ) * cosf( 2.f * 3.1415926f * v[0] );
                d[1] = sqrtf( v[1] ) * sinf( 2.f * 3.1415926f * v[0] );
                d[2] = sqrtf( v[3] ) * cosf( 2.f * 3.1415926f * v[2] );
                d[3] = sqrtf( v[3] ) * sinf( 2.f * 3.1415926f * v[2] );

                // store d into unsigned values [0,255]
                const unsigned int tmp = ( (r * size * size) + (t * size) + s ) * 4;
                pJitterData[ tmp + 0 ] = (unsigned char)( ( 1.f + d[0] ) * 127  );
                pJitterData[ tmp + 1 ] = (unsigned char)( ( 1.f + d[1] ) * 127  );
                pJitterData[ tmp + 2 ] = (unsigned char)( ( 1.f + d[2] ) * 127  );
                pJitterData[ tmp + 3 ] = (unsigned char)( ( 1.f + d[3] ) * 127  );

            }
        }
    }

    // the GPU Gem implementation uses a NV specific internal texture format (GL_SIGNED_RGBA_NV)
    // In order to make it more generic, we use GL_RGBA4 which should be cross platform.
    pJitterImage->setImage(size, size, R, GL_RGBA4, GL_RGBA, GL_UNSIGNED_BYTE, pJitterData, osg::Image::USE_NEW_DELETE);
    pJitterTexture->setImage(pJitterImage);

    pStateSet->setTextureAttributeAndModes(m_JitterTextureUnit, pJitterTexture, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    pStateSet->setTextureMode(m_JitterTextureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
    pStateSet->setTextureMode(m_JitterTextureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
    pStateSet->setTextureMode(m_JitterTextureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
}

#endif
