//
// AnimPath.h
//
// Implementation animation path capabilities.
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/core/Engine.h"
#include "vtdata/CubicSpline.h"
#include "vtdata/Features.h"

/**
 * This class describes a single location, including both a position and
 * orientation, in world coordinates, which is used to define a path.
 */
struct ControlPoint
{
	ControlPoint() {}

	ControlPoint(const FPoint3 &position):
		m_Position(position),
		m_Rotation() {}

	ControlPoint(const FPoint3 &position, const FQuat &rotation):
		m_Position(position),
		m_Rotation(rotation) {}

	void Interpolate(float ratio,const ControlPoint &first, const ControlPoint &second);
	void GetMatrix(FMatrix4 &matrix, bool bPosOnly) const;

	FPoint3 m_Position;
	FQuat m_Rotation;
	int m_iIndex;
};

/**
 * An animation path is a series of locations (class ControlPoint) which
 * define a path through space in world coordinates.  It is useful to use
 * in conjunction with the vtAnimPathEngine class, which can move any
 * transform (such as scene graph object or camera) along the path.
 */
class vtAnimPath
{
public:
	vtAnimPath();
	vtAnimPath(const vtAnimPath &ap);
	virtual ~vtAnimPath();

	/// Must tell the AnimPath what projection its points are in, for serialization.
	void SetProjection(const vtProjection &proj);

	/// get the transformation matrix for a point in time.
	bool GetMatrix(double time, FMatrix4 &matrix, bool bPosOnly) const
	{
		ControlPoint cp;
		if (!GetInterpolatedControlPoint(time,cp))
			return false;
		cp.GetMatrix(matrix, bPosOnly);
		return true;
	}

	/// get the local ControlPoint frame for a point in time.
	virtual bool GetInterpolatedControlPoint(double time, ControlPoint &controlPoint) const;

	// Add a control point to this path
	void Insert(double time, const ControlPoint &controlPoint);

	unsigned int GetNumPoints() { return m_TimeControlPointMap.size(); }
	void SetTimeFromLinearDistance();
	void ProcessPoints();

	double GetFirstTime() const;
	double GetLastTime() const;
	double GetPeriod() const { return GetLastTime()-GetFirstTime();}
	float GetTotalTime();

	enum InterpMode
	{
		LINEAR,
		CUBIC_SPLINE
	};

	void SetInterpMode(InterpMode mode) { m_InterpMode = mode; }
	InterpMode GetInterpMode() const { return m_InterpMode; }

	typedef std::map<double,ControlPoint> TimeControlPointMap;

	TimeControlPointMap &GetTimeControlPointMap() { return m_TimeControlPointMap; }
	const TimeControlPointMap &GetTimeControlPointMap() const { return m_TimeControlPointMap; }

	bool IsEmpty() const { return m_TimeControlPointMap.empty(); }

	void SetLoop(bool bFlag);
	bool GetLoop() const { return m_bLoop; }

	bool Write(const char *fname);
	bool Read(const char *fname);

	bool CreateFromLineString(const vtProjection &proj, vtFeatureSet *pSet);

protected:
	void InterpolateControlPoints(TimeControlPointMap::const_iterator &a,
								  TimeControlPointMap::const_iterator &b,
								  double time,
								  ControlPoint &c) const;
	TimeControlPointMap m_TimeControlPointMap;

	InterpMode		m_InterpMode;
	CubicSpline		m_Spline;
	bool			m_bLoop;
	float			m_fLoopSegmentTime;
	ControlPoint	m_LoopControlPoint;

	// For dealing with global projection
	vtProjection	m_proj;
	OCT			*m_pConvertToWGS;
	OCT			*m_pConvertFromWGS;
	friend class AnimPathVisitor;
};

/**
 * A vtAnimPathEngine connects a path (vtAnimPath) to a target.
 * The target can be any transform (such as scene graph object or
 * camera) which will be moved along the path based on time.
 */
class vtAnimPathEngine : public vtEngine
{
public:
	vtAnimPathEngine():
		m_pAnimationPath(NULL),
		m_fTimeMultiplier(1.0),
		m_fLastTime(DBL_MAX),
		m_fTime(0.0f),
		m_bContinuous(false),
		m_bPosOnly(false),
		m_fSpeed(1.0f) {}

	vtAnimPathEngine(vtAnimPath *ap, double timeMultiplier=1.0f):
		m_pAnimationPath(ap),
		m_fTimeMultiplier(timeMultiplier),
		m_fLastTime(DBL_MAX),
		m_fTime(0.0f),
		m_bContinuous(false),
		m_bPosOnly(false),
		m_fSpeed(1.0f) {}

	~vtAnimPathEngine()
	{
		delete m_pAnimationPath;
	}

	void SetAnimationPath(vtAnimPath* path) { m_pAnimationPath = path; }
	vtAnimPath* GetAnimationPath() { return m_pAnimationPath; }
	const vtAnimPath* GetAnimationPath() const { return m_pAnimationPath; }

	void SetTimeMultiplier(double multiplier) { m_fTimeMultiplier = multiplier; }
	double GetTimeMultiplier() const { return m_fTimeMultiplier; }

	void UpdateTargets();
	void Reset();

	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	float GetSpeed() const { return m_fSpeed; }

	void SetContinuous(bool bFlag) { m_bContinuous = bFlag; }
	bool GetContinuous() const { return m_bContinuous; }

	void SetPosOnly(bool bFlag) { m_bPosOnly = bFlag; }
	bool GetPosOnly() const { return m_bPosOnly; }

	/// Virtual handler, will be called every frame to do the work of the engine.
	virtual void Eval();
	virtual void SetEnabled(bool bOn);

	void SetTime(float fTime) { m_fTime = fTime; }
	float GetTime() { return m_fTime; }

public:
	vtAnimPath *m_pAnimationPath;
	bool	m_bContinuous;
	double	m_fTimeMultiplier;
	double	m_fLastTime;
	double	m_fTime;
	bool	m_bPosOnly;
	float	m_fSpeed;
};


