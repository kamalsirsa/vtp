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

/**
 * This class describes a single location, including both a position and
 * orientation, in world coordinates, which is used to define a path.
 */
struct ControlPoint
{
	ControlPoint():
		m_Scale(1.0f,1.0f,1.0f) {}

	ControlPoint(const FPoint3 &position):
		m_Position(position),
		m_fRotation(),
		m_Scale(1.0f,1.0f,1.0f) {}

	ControlPoint(const FPoint3 &position, const FQuat &rotation):
		m_Position(position),
		m_fRotation(rotation),
		m_Scale(1.0f,1.0f,1.0f) {}

	ControlPoint(const FPoint3 &position, const FQuat &rotation, const FPoint3 &scale):
		m_Position(position),
		m_fRotation(rotation),
		m_Scale(scale) {}

	void Interpolate(float ratio,const ControlPoint &first, const ControlPoint &second);
	void GetMatrix(FMatrix4 &matrix) const;

	FPoint3 m_Position;
	FQuat m_fRotation;
	FPoint3 m_Scale;
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
	vtAnimPath():
		m_bLoop(true),
		m_InterpMode(LINEAR) {}

	vtAnimPath(const vtAnimPath &ap):
		m_TimeControlPointMap(ap.m_TimeControlPointMap),
		m_bLoop(ap.m_bLoop),
		m_InterpMode(ap.m_InterpMode) {}

	virtual ~vtAnimPath() {}

	/// get the transformation matrix for a point in time.        
	bool GetMatrix(double time, FMatrix4 &matrix) const
	{
		ControlPoint cp;
		if (!GetInterpolatedControlPoint(time,cp))
			return false;
		cp.GetMatrix(matrix);
		return true;
	}

	/// get the local ControlPoint frame for a point in time.
	virtual bool GetInterpolatedControlPoint(double time, ControlPoint &controlPoint) const;

	// Add a control point to this path
	void Insert(double time, const ControlPoint &controlPoint);

	void ProcessPoints();

	double GetFirstTime() const
	{
		if (m_TimeControlPointMap.empty())
			return 0.0;
		return m_TimeControlPointMap.begin()->first;
	}
	double GetLastTime() const
	{
		if (m_TimeControlPointMap.empty())
			return 0.0;
		return m_TimeControlPointMap.rbegin()->first;
	}
	double GetPeriod() const { return GetLastTime()-GetFirstTime();}

	enum InterpMode
	{
		LINEAR,
		CUBIC_SPLINE
	};

	void SetLoop(bool bLoop) { m_bLoop = bLoop; }
	bool GetLoop() const { return m_bLoop; }

	void SetInterpMode(InterpMode mode) { m_InterpMode = mode; }
	InterpMode GetInterpMode() const { return m_InterpMode; }

	typedef std::map<double,ControlPoint> TimeControlPointMap;

	TimeControlPointMap &GetTimeControlPointMap() { return m_TimeControlPointMap; }
	const TimeControlPointMap &GetTimeControlPointMap() const { return m_TimeControlPointMap; }

	bool IsEmpty() const { return m_TimeControlPointMap.empty(); }

/*	void Read(std::istream &in);
	void Write(std::ostream &out) const; */

protected:
	TimeControlPointMap m_TimeControlPointMap;
	bool	            m_bLoop;
	InterpMode			m_InterpMode;
	CubicSpline			m_Spline;
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
		m_fFirstTime(DBL_MAX),
		m_fLatestTime(0.0) {}

	vtAnimPathEngine(vtAnimPath *ap, double timeMultiplier=1.0f):
		m_pAnimationPath(ap),
		m_fTimeMultiplier(timeMultiplier),
		m_fFirstTime(DBL_MAX),
		m_fLatestTime(0.0) {}

	~vtAnimPathEngine()
	{
		delete m_pAnimationPath;
	}

	void SetAnimationPath(vtAnimPath* path) { m_pAnimationPath = path; }
	vtAnimPath* GetAnimationPath() { return m_pAnimationPath; }
	const vtAnimPath* GetAnimationPath() const { return m_pAnimationPath; }

	void SetTimeMultiplier(double multiplier) { m_fTimeMultiplier = multiplier; }
	double GetTimeMultiplier() const { return m_fTimeMultiplier; }

	void Reset();

	/// Get the time value used to specify the location along the path.
	double GetAnimationTime() const;

	/// Virtual handler, will be called every frame to do the work of the engine.
	virtual void Eval();

public:
	vtAnimPath *m_pAnimationPath;
	double	m_fTimeMultiplier;
	double	m_fFirstTime;
	double	m_fLatestTime;
};

