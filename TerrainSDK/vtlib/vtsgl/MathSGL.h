
#ifndef VTSGL_MATHH
#define VTSGL_MATHH

///////////////////////
// math helpers

inline void v2s(const FPoint2 &f, sglVec2f &s) { s[0] = f.x; s[1] = f.y; }
inline void v2s(const FPoint3 &f, sglVec3f &s) { s[0] = f.x; s[1] = f.y; s[2] = f.z; }
inline void v2s(const RGBf &f, sglVec4f &s) { s[0] = f.r; s[1] = f.g; s[2] = f.b; s[3] = 1.0f; }
inline void v2s(const RGBAf &f, sglVec4f &s) { s[0] = f.r; s[1] = f.g; s[2] = f.b; s[3] = f.a; }

inline s2v(const sglVec2f &s, FPoint2 &f) { f.x = s[0]; f.y = s[1]; }
inline s2v(const sglVec3f &s, FPoint3 &f) { f.x = s[0]; f.y = s[1]; f.z = s[2]; }
inline s2v(const sglVec4f &s, RGBf &f) { f.r = s[0]; f.g = s[1]; f.b = s[2]; }
inline s2v(const sglVec4f &s, RGBAf &f) { f.r = s[0]; f.g = s[1]; f.b = s[2]; f.a = s[3]; }

inline void s2v(const sglSphereBoundf &s, FSphere &sph)
{
	const sglVec3f center = s.getCenter();
	sph.center.x = center[0];
	sph.center.y = center[1];
	sph.center.z = center[2];
	sph.radius = s.getRadius();
}

inline void ConvertMatrix4(const sglMat4f &mat_sgl, FMatrix4 &mat)
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			mat.Set(i, j, mat_sgl[j][i]);
		}
}

inline void ConvertMatrix4(const FMatrix4 &mat, sglMat4f &mat_sgl)
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			mat_sgl[i][j] = mat.Get(j, i);
		}
}

#endif

