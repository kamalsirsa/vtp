//
// HeightField.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "HeightField.h"
#include "vtDIB.h"

vtHeightField::vtHeightField()
{
	m_EarthExtents.SetRect(0, 0, 0, 0);
}

void vtHeightField::Initialize(const DRECT &earthcover, float fMinHeight,
							   float fMaxHeight)
{
	m_EarthExtents = earthcover;
	m_fMinHeight = fMinHeight;
	m_fMaxHeight = fMaxHeight;
}

/** Gets the minimum and maximum height values.  The values are placed in the
 * arguments by reference.  You must have first called ComputeHeightExtents.
 */
void vtHeightField::GetHeightExtents(float &fMinHeight, float &fMaxHeight) const
{
	fMinHeight = m_fMinHeight;
	fMaxHeight = m_fMaxHeight;
}


/**
 * \return 0 if below terrain, 1 if above terrain, -1 if off terrain.
 */
int vtHeightField3d::PointIsAboveTerrain(const FPoint3 &p) const
{
	float alt;
	if (!FindAltitudeAtPoint(p, alt))
		return -1;
	if (alt < p.y)
		return 1;
	else
		return 0;
}


/**
 * Converts a earth coordinate (project or geographic) to a world coordinate
 * on the surface of the heightfield.
 */
bool vtHeightField3d::ConvertEarthToSurfacePoint(double x, double y, FPoint3 &p3)
{
	// convert earth -> XZ
	m_Conversion.ConvertFromEarth(DPoint2(x, y), p3.x, p3.z);

	// look up altitude
	return FindAltitudeAtPoint(p3, p3.y);
}

/**
 * Tests whether a given point is within the current terrain
 */
bool vtHeightField3d::ContainsWorldPoint(float x, float z)
{
	const FRECT &we = m_WorldExtents;
	return (x > we.left && x < we.right && z < we.bottom && z > we.top);
}


void vtHeightField3d::GetCenter(FPoint3 &center)
{
	FPoint2 c;
	m_WorldExtents.Center(c);
	center.x = c.x;
	center.z = c.y;
	center.y = 0.0f;
}


/////////////////////////////////////////////////////////////////////////////

void vtHeightField3d::Initialize(const LinearUnits units,
	const DRECT &earthextents, float fMinHeight, float fMaxHeight)
{
	vtHeightField::Initialize(earthextents, fMinHeight, fMaxHeight);

	m_Conversion.Setup(units,
		DPoint2(m_EarthExtents.left, m_EarthExtents.bottom));

	m_Conversion.convert_earth_to_local_xz(
		m_EarthExtents.left, m_EarthExtents.bottom,
		m_WorldExtents.left, m_WorldExtents.bottom);

	m_Conversion.convert_earth_to_local_xz(
		m_EarthExtents.right, m_EarthExtents.top,
		m_WorldExtents.right, m_WorldExtents.top);

	FPoint2 hypo(m_WorldExtents.Width(), m_WorldExtents.Height());
	m_fDiagonalLength = hypo.Length();
}


/////////////////////////////////////////////////////////////////////////////
// vtHeightFieldGrid3d

vtHeightFieldGrid3d::vtHeightFieldGrid3d()
{
	m_iColumns = 0;
	m_iRows = 0;
	m_fXStep = 0.0f;
	m_fZStep = 0.0f;
}

void vtHeightFieldGrid3d::Initialize(const LinearUnits units,
	const DRECT& earthextents, float fMinHeight, float fMaxHeight,
	int cols, int rows)
{
	// first initialize parent
	vtHeightField3d::Initialize(units, earthextents, fMinHeight, fMaxHeight);

	m_iColumns = cols;
	m_iRows = rows;

	m_fXStep = m_WorldExtents.Width() / (m_iColumns-1);
	m_fZStep = -m_WorldExtents.Height() / (m_iRows-1);
}


/** Get the grid spacing, the width of each column and row.
 */
DPoint2 vtHeightFieldGrid3d::GetSpacing() const
{
	return DPoint2(m_EarthExtents.Width() / (m_iColumns - 1),
		m_EarthExtents.Height() / (m_iRows - 1));
}

FPoint2 vtHeightFieldGrid3d::GetWorldSpacing() const
{
	return FPoint2(m_fXStep, m_fZStep);
}

/** Get the grid size of the grid.
 * The values are placed into the arguments by reference.
 * \param nColumns The number of columns (east-west)
 * \param nRows THe number of rows (north-south)
 */
void vtHeightFieldGrid3d::GetDimensions(int &nColumns, int &nRows) const
{
	nColumns = m_iColumns;
	nRows = m_iRows;
}

#define PICK_DISTANCE	1000

/**
 * Tests a ray against a heightfield grid.
 *
 * Note: This algorithm is not guaranteed to give absolutely correct results,
 * but it is reasonably fast and efficient.  It checks a series of PICK_DISTANCE
 * points along the ray against the terrain.  When a pair of points (segment)
 * is found to straddle the terrain, it refines the segment in a binary fashion.
 *
 * Since the length of the test is proportional to a single grid element,
 * there is a very small chance that it will give results that are off by
 * a small distance (less than 1 grid element)
 *
 * \return true if hit terrain.
 */
bool vtHeightFieldGrid3d::CastRayToSurface(const FPoint3 &point,
										   const FPoint3 &dir, FPoint3 &result) const
{
	// cast a series of line segment along the ray
	int i, above;
	FPoint3 p0 = point, p1, p2 = point;
	FPoint3 delta = dir * (m_fDiagonalLength / (m_iColumns * 1.41f));
	bool found_above = false;
	for (i = 0; i < PICK_DISTANCE; i++)
	{
		above = PointIsAboveTerrain(p2);
		if (above == 0)	// below
		{
			p1 = p2;
			break;
		}
		if (above == 1)	// above
		{
			found_above = true;
			p0 = p2;
		}
		p2 += delta;
	}
	if (i == PICK_DISTANCE || !found_above)
		return false;
	// now, do a binary search to refine the result
	for (i = 0; i < 10; i++)
	{
		p2 = (p0 + p1) / 2.0f;
		above = PointIsAboveTerrain(p2);
		if (above == 1)	// above
			p0 = p2;
		else if (above == 0)	// below
			p1 = p2;
	}
	p2 = (p0 + p1) / 2.0f;
	// make sure it's precisely on the ground
	FindAltitudeAtPoint(p2, p2.y);
	result = p2;
	return true;
}

/**
 * Use the height data in the grid to fill a bitmap with a shaded color image.
 *
 * \param pBM			The bitmap to color.
 * \param color_ocean	The color to use for areas at sea level.
 * \param bZeroIsOcean	True if allow areas with elevation=0 are to be
 *							considered ocean.
 * \param progress_callback If supplied, this function will be called back
 *				with a value of 0 to 100 as the operation progresses.
 */
void vtHeightFieldGrid3d::ColorDibFromElevation(vtBitmapBase *pBM, RGBi color_ocean,
	bool bZeroIsOcean, void progress_callback(int))
{
	int w = pBM->GetWidth();
	int h = pBM->GetHeight();

	int gw, gh;
	GetDimensions(gw, gh);

	float fMin, fMax;
	GetHeightExtents(fMin, fMax);

	Array<RGBi> colors;
	colors.Append(RGBi(75, 155, 75));
	colors.Append(RGBi(180, 160, 120));
	colors.Append(RGBi(128, 128, 128));
	int bracket, num = colors.GetSize();
	float bracket_size = (fMax - fMin) / (num - 1);

	int i, j;
	int x, y;
	RGBi color;

	// iterate over the texels
	for (i = 0; i < w; i++)
	{
		if (progress_callback != NULL)
		{
			if ((i&7) == 0)
				progress_callback(i * 100 / w);
		}
		x = i * gw / w;			// find corresponding location in terrain

		for (j = 0; j < h; j++)
		{
			y = j * gh / h;

			float m = GetElevation(x, y);	// local units
			float elev = m - fMin;

			color.r = color.g = color.b = 0;
			if (bZeroIsOcean && m == 0.0f)
			{
				color = color_ocean;
			}
			else if (bracket_size != 0.0f)
			{
				bracket = (int) (elev / bracket_size);
				if (bracket < 0)
					color = colors[0];
				else if (bracket < num-1)
				{
					float fraction = (elev / bracket_size) - bracket;
					RGBi diff = (colors[bracket+1] - colors[bracket]);
					color = colors[bracket] + (diff * fraction);
				}
				else
					color = colors[num-1];
			}
			else
			{
				color.Set(20, 230, 20);	// flat green
			}
			pBM->SetPixel24(i, h-1-j, color);
		}
	}
}


void vtHeightFieldGrid3d::ShadeDibFromElevation(vtBitmapBase *pBM, const FPoint3 &light_dir,
	float light_factor, void progress_callback(int))
{
	// consider upward-pointing, rather than downward-pointing, normal
	FPoint3 light_direction = -light_dir;

	int w = pBM->GetWidth();
	int h = pBM->GetHeight();
	int gw = m_iColumns, gh = m_iRows;

	float xFactor = (float)gw/(float)w;
	float yFactor = (float)gh/(float)h;

	bool b8bit = (pBM->GetDepth() == 8);
	FPoint3 p1, p2, p3;
	FPoint3 v1, v2, v3;
	int i, j;
	int x, y;
	RGBi rgb;

	// iterate over the texels
	for (j = 0; j < h-1; j++)
	{
		if (progress_callback != NULL)
		{
			if ((j&7) == 0)
				progress_callback(j * 100 / h);
		}
		// find corresponding location in terrain
		y = (int) (j * yFactor);
		for (i = 0; i < w-1; i++)
		{
			x = (int) (i * xFactor);

			float shade;
			if (x < gw-1)
			{
				// compute surface normal
				GetWorldLocation(x, y, p1);
				GetWorldLocation(x+1, y, p2);
				GetWorldLocation(x, y+1, p3);
				v1 = p2 - p1;
				v2 = p3 - p1;
				v3 = v1.Cross(v2);
				v3.Normalize();

				shade = v3.Dot(light_direction); // shading 0 (dark) to 1 (light)
				shade /= .7071f;
				shade = 1.0f + ((shade - 1.0f) * 2.0f);
				if (shade < 0.3f)	// clip - don't shade down below ambient level
					shade = 0.3f;
				else if (shade > 1.2f)
					shade = 1.2f;
			}
			else
				shade = 1.0f;

			// Push the value of 'shade' toward 1.0 by the light_factor factor.
			// This means that light_factor=0 means no lighting, 1 means full lighting.
			float diff = 1 - shade;
			diff = diff * (1 - light_factor);
			shade += diff;

			// combine color and shading
			if (b8bit)
				pBM->ScalePixel8(i, h-1-j, shade);
			else
				pBM->ScalePixel24(i, h-1-j, shade);
		}
	}
}

class LightMap
{
public:
	LightMap(int w, int h)
	{
		m_w = w;
		m_h = h;
		m_data = new unsigned char*[m_w];
		int rows, cols;
		for (cols = 0; cols < m_w; cols++)
			m_data[cols] = new unsigned char[m_h];
		for (cols = 0; cols < m_w; cols++)
			for (rows = 0; rows < m_h ; rows++)
				m_data[cols][rows] = 0;
	}
	~LightMap()
	{
		// Dispose with the temporary arrays
		for (int cols = 0 ; cols < m_w ; ++cols)
			delete [] m_data[cols];
		delete [] m_data;
	}
	void Set(int x, int y, unsigned char val) { m_data[x][y] = val; }
	unsigned char Get(int x, int y) { return m_data[x][y]; }

	unsigned char **m_data;
	int m_w, m_h;
};


/**
 * ShadowCastDib - method to create shadows over the terrain based on the
 * angle of the sun.
 *
 * \param pBM	An interface to the bitmap to be shaded.
 * \param light_dir	The direction of the light, in world coordinates, coming
 *		down toward the terrain.  For example, (-1,-1,0) is pointing down
 *		where the sun would be at 9AM on the equator.  Despite this example,
 *		light_dir should be normalized to unit length.
 * \param light_factor	Amount of shading, from 0 to 1.  A value of 0 means
 *		no lighting, 1 means full lighting.
 * \param progress_callback	Optional callback for progress notification.
 */
/* Core code contributed by Kevin Behilo, 2/20/04.
 *
 * Possible TODO: add code to soften and blend shadow edges
 *  (see aliasing comments in source).
 *
 * Definite TODO: the whole thing can be sped up by precalculating the
 *  surface normals once.  In fact that should be placed in a separate Shading
 *  Context, so that it could be re-used for quickly re-shading multiple times.
 */
void vtHeightFieldGrid3d::ShadowCastDib(vtBitmapBase *pBM, const FPoint3 &light_dir,
	float light_factor, void progress_callback(int))
{
	int w = pBM->GetWidth();
	int h = pBM->GetHeight();

	int gw, gh;
	GetDimensions(gw, gh);

	float xFactor = (float)gw/(float)w;
	float yFactor = (float)gh/(float)h;

	bool b8bit = (pBM->GetDepth() == 8);

	float shade, f, HScale;;
	FPoint3 p1, p2, p3;
	FPoint3 v1, v2, v3;
	int i, j;
	int x, y, z;
	int i_init, i_final, i_incr;
	int j_init, j_final, j_incr;

	// These values are hardcoded here but could be exposed in the GUI
	float sun =  0.7f;
	float amb =  0.45f;

	// If we have light that's pointing UP, rather than down at the terrain,
	//  then it's only going to take a really long time to produce a
	//  completely dark terrain.  We can catch this case up front.
	if (light_dir.y > 0)
	{
		for (i = 0; i < w; i++)
		{
			for (j = 0; j < h; j++)
			{
				if (b8bit)
					pBM->ScalePixel8(i, j, amb);
				else
					pBM->ScalePixel24(i, j, amb);
			}
		}
		return;
	}

	// Create array to hold flags 
	LightMap lightmap(w, h);

	// This factor is used when applying shading to non-shadowed areas to
	// try and keep the "contrast" down to a min. (still get "patches" of
	// dark/light spots though).
	// It is initialized to 1.0, because in case there are no shadows at all
	//  (such as at noon) we still need a reasonable value.
	float darkest_shadow = 1.0;

	// For the vector used to cast shadows, we need it in grid coordinates,
	//  which are (Column,Row) where Row is north.  But the direction passed
	//  in uses OpenGL coordinates where Z is south.  So flip Z.
	FPoint3 grid_light_dir = light_dir;
	grid_light_dir.z = -grid_light_dir.z;

	// Scale the light vector such that the X or Z component (whichever is
	//  larger) is 1.  This is will serve as our direction vector in grid
	//  coordinates, when drawing a line across the grid to cast the shadow.
	//
	// Code adapted from aaron_torpy:
	// http://www.geocities.com/aaron_torpy/algorithms.htm
	//
	if ( fabs(grid_light_dir.x) > fabs(grid_light_dir.z) )
	{
		HScale = m_fXStep;
		f = fabs(light_dir.x);
	}
	else
	{
		HScale = m_fZStep;
		f = fabs(light_dir.z);
	}
	grid_light_dir /= f;
		
	if (grid_light_dir.x > 0)
	{
		i_init=0;
		i_final=w-1;
		i_incr=1;
	}
	else
	{
		i_init=w-1;
		i_final=-1;
		i_incr=-1;
	}
	if (grid_light_dir.z > 0)
	{
		j_init=0;
		j_final=h-1;
		j_incr=1;
	}
	else
	{
		j_init=h-1;
		j_final=-1;
		j_incr=-1;
	}

	// First pass: find each point that it is in shadow.
    for (j = j_init; j != j_final; j += j_incr) 
	{
   		if (progress_callback != NULL)
		{
			if ((j&7) == 0)
				progress_callback(abs(j-j_init) * 100 / h);
		}
		for (i = i_init; i != i_final; i += i_incr) 
		{
			float shadowheight = GetElevation(i,j);

			if (shadowheight == INVALID_ELEVATION)
			{
				// set a flag so we won't visit this one again
				lightmap.Set(i, j, 1);
				continue;
			}

			bool Under_Out;
			int k;
			for (k=1, Under_Out=false; Under_Out == false; k++) 
			{
				x = (int) (i + grid_light_dir.x*k + 0.5f);
				z = (int) (j + grid_light_dir.z*k + 0.5f);
				shadowheight += grid_light_dir.y * HScale;

				if ((x<0) || (x>=w-1) || (z<0) || (z>=h-1)) 
				{
					Under_Out = true; //Out of the grid
					break;
				}

				float elevation = GetElevation(x,z);

				// skip holes in the grid
				if (elevation == INVALID_ELEVATION)
					continue;

				if (elevation - shadowheight > 0) 
				{ 	
					if (k>1)
						Under_Out = true; //Under the terrain 
					break;
				} 

				// Combine color and shading.
				// Only do shadow if we have not shaded this i,j before.
				if (lightmap.Get(x,z) < 1)
				{
					int xx = (int) (x * xFactor);
					int yy = (int) (z * yFactor);	
					GetWorldLocation(xx, yy, p1);
					GetWorldLocation(xx+1, yy, p2);
					GetWorldLocation(xx, yy+1, p3);

					v1 = p2 - p1;
					v2 = p3 - p1;
					v3 = v1.Cross(v2);
					v3.Normalize();

					//*****************************************
					//*****************************************
					//The Amb .5f value was arbitrarily chosen
					//Need to experiment more to determine the best value
					//perhaps calculating Sun(r, g, b) and Amb(r, g, b)
					// for a given time of day (e.g. warmer colors close to sunset)
					//or give control to user since textures will differ

					// Here the Sun(r, g, b) = 0 because we are in the shade
					// therefore I(r, g, b) = Amb(r, g, b) * (0.5*N[z] + 0.5)

				//	shade =  sun*v3.Dot(-light_direction) + amb * (0.5f*v3.y + 0.5f);
					shade =  amb * (0.5f*v3.y + 0.5f);
					//*****************************************
					//*****************************************
					if (darkest_shadow > shade)
						darkest_shadow = shade;

					//Rather than doing the shading at this point we may want to 
					//simply save the value into the LightMap array. Then apply 
					//some anti-aliasing or edge softening algorithm to the LightMap.
					//Once that's done, apply the whole LightMap to the DIB.
					if (b8bit)
						pBM->ScalePixel8(x, h-1-z, shade);
					else
						pBM->ScalePixel24(x, h-1-z, shade);

					//set a flag to show that this texel has been shaded.
					// (or set to value of the shading - see comment above)
					lightmap.Set(x, z, lightmap.Get(x, z)+1);
				}
			}
		} //for i
	} //for j

	// For dot-product lighting, we use the normal 3D vector, only inverted
	//  so that we can compare it to the upward-pointing ground normals.
	FPoint3 inv_light_dir = -light_dir;

	// Second pass.  Now we are going to loop through the LightMap and apply
	//  the full lighting formula to each texel that has not been shaded yet.
	for (j = 0; j < h-1; j++)
	{
   		if (progress_callback != NULL)
		{
			if ((j&7) == 0)
				progress_callback(j * 100 / h);
		}
		for (i = 0; i < w-1; i++)
		{
			if (lightmap.Get(i, j) > 0)
				continue;

			if (GetElevation(i,j) == INVALID_ELEVATION)
				continue;

			x = (int) (i * xFactor);
			y = (int) (j * yFactor);	
			GetWorldLocation(x, y, p1);
			GetWorldLocation(x+1, y, p2);
			GetWorldLocation(x, y+1, p3);

			v1 = p2 - p1;
			v2 = p3 - p1;
			v3 = v1.Cross(v2);
			FPoint3 vn=v3;
			v3.Normalize();
			
			//*****************************************
			//*****************************************
			//shade formula based on:
			//http://www.geocities.com/aaron_torpy/algorithms.htm#calc_intensity

			// The Amb value was arbitrarily chosen
			// Need to experiment more to determine the best value
			// Perhaps calculating Sun(r, g, b) and Amb(r, g, b) for a
			//  given time of day (e.g. warmer colors close to sunset)
			// or give control to user since textures will differ

			// I(r, g, b) = Sun(r, g, b) * scalarprod(N, v) + Amb(r, g, b) * (0.5*N[z] + 0.5)
			shade = sun * v3.Dot(inv_light_dir);

			// It's a reasonable assuption that an angle of 45 degrees is
			//  sufficient to fully illuminate the ground.
			shade /= .7071f;

			// Now add ambient component
			shade += amb * (0.5f*v3.y + 0.5f);

			// Maybe clipping values can be exposed to the user as well.
			// Clip - don't shade down below lowest ambient level
			if (shade < darkest_shadow)
				shade = darkest_shadow;
			else if (shade > 1.2f)
				shade = 1.2f;

			// Push the value of 'shade' toward 1.0 by the light_factor factor.
			// This means that light_factor=0 means no lighting, 1 means full lighting.
			float diff = 1 - shade;
			diff = diff * (1 - light_factor);
			shade += diff;

			// Rather than doing the shading at this point we may want to 
			// simply save the value into the LightMap array. Then apply 
			// some anti-aliasing or edge softening algorithm to the LightMap.
			// Once that's done, apply the whole LightMap to the DIB.
			// LightMap[I][J]= shade; // set to value of the shading - see comment above)
			if (b8bit)
				pBM->ScalePixel8(i, h-1-j, shade);
			else
				pBM->ScalePixel24(i, h-1-j, shade);
		}
	}

	// Possible TODO: Apply edge softening algorithm (?)

}


