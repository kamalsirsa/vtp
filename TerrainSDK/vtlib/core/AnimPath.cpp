//
// AnimPath.cpp
//
// Implementation animation path capabilities.
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "AnimPath.h"
#include "vtdata/LocalConversion.h"


void ControlPoint::Interpolate(float ratio, const ControlPoint &first, const ControlPoint &second)
{
	float one_minus_ratio = 1.0f - ratio;
	m_Position = first.m_Position*one_minus_ratio + second.m_Position*ratio;
	m_Rotation.Slerp(first.m_Rotation, second.m_Rotation, ratio);
	m_Scale = first.m_Scale*one_minus_ratio + second.m_Scale*ratio;
}

void ControlPoint::GetMatrix(FMatrix4 &matrix, bool bPosOnly) const
{
	matrix.MakeScale(m_Scale.x, m_Scale.y, m_Scale.z);

	if (!bPosOnly)
	{
		FMatrix3 m3;
		m_Rotation.GetMatrix(m3);

		FMatrix4 m4;
		m4.SetFromMatrix3(m3);
		matrix.PostMult(m4);
	}

	matrix.Translate(m_Position);
}

void vtAnimPath::Insert(double time,const ControlPoint &controlPoint)
{
	m_TimeControlPointMap[time] = controlPoint;
}

void vtAnimPath::ProcessPoints()
{
	m_Spline.Cleanup();

	TimeControlPointMap::iterator it = m_TimeControlPointMap.begin();
	int i = 0;
	while (it != m_TimeControlPointMap.end())
	{
		it->second.m_iIndex = i;
		m_Spline.AddPoint(it->second.m_Position);

		it++;
		i++;
	}

	// one more in case they want to loop
//	it = m_TimeControlPointMap.begin();
//	m_Spline.AddPoint(it->second.m_Position);

	// now create smooth curve, in case it's needed later
	m_Spline.Generate();
}

bool vtAnimPath::GetInterpolatedControlPoint(double time, ControlPoint &controlPoint) const
{
	if (m_TimeControlPointMap.empty())
		return false;

	if (m_bLoop)
	{
		double modulated_time = (time - GetFirstTime())/GetPeriod();
		double fraction_part = modulated_time - floor(modulated_time);
		time = GetFirstTime()+fraction_part * GetPeriod();
	}

	TimeControlPointMap::const_iterator second = m_TimeControlPointMap.lower_bound(time);
	if (second==m_TimeControlPointMap.begin())
	{
		controlPoint = second->second;
	}
	else if (second != m_TimeControlPointMap.end())
	{
		TimeControlPointMap::const_iterator first = second;
		--first;

		// we have both a lower bound and the next item.

		// deta_time = second.time - first.time
		double delta_time = second->first - first->first;

		if (delta_time==0.0)
			controlPoint = first->second;
		else
		{
			double elapsed = time - first->first;
			double ratio = elapsed/delta_time;

			controlPoint.Interpolate(ratio, first->second, second->second);
			if (m_InterpMode == CUBIC_SPLINE)
			{
				// Don't use that linear position.
				// Find the position on the spline.
				int num = first->second.m_iIndex;

				DPoint3 dpos;
				m_Spline.Interpolate(num + ratio, &dpos);
				controlPoint.m_Position = dpos;
			}
		}
	}
	else // (second==_timeControlPointMap.end())
	{
		controlPoint = m_TimeControlPointMap.rbegin()->second;
	}
	return true;
}

void vtAnimPathEngine::SetEnabled(bool bOn)
{
	bool bWas = m_bEnabled;
	vtEnabledBase::SetEnabled(bOn);
	if (!bWas && bOn)
	{
		// turning this engine on
		m_fLastTime = vtGetTime();
	}
}

/*void vtAnimPath::Read(std::istream &in)
{
	while (!in.eof())
	{
		double time;
		FPoint3 position;
		FQuat rotation;
		in >> time >> position.x >> position.y >> position.z >> rotation.x >> rotation.y >> rotation.z >> rotation.w;
		if(!in.eof())
			insert(time,ControlPoint(position,rotation));
	}
}

void vtAnimPath::Write(std::ostream &fout) const
{
	int prec = fout.precision();
	fout.precision(15);

	const TimeControlPointMap &tcpm = getTimeControlPointMap();
	for(TimeControlPointMap::const_iterator tcpmitr=tcpm.begin();
		tcpmitr!=tcpm.end();
		++tcpmitr)
	{
		const ControlPoint &cp = tcpmitr->second;
		fout<<tcpmitr->first<<" "<<cp._position.x<<" "<<cp._position.y<<" "<<cp._position.z<<" "<<cp._rotation<<std::endl;
	}

	fout.precision(prec);
}*/

void vtAnimPathEngine::Eval()
{
	if (m_pAnimationPath == NULL)
		return;

	vtTarget *target = GetTarget();
	vtTransform *tr = dynamic_cast<vtTransform*>(target);
	if (!tr)
		return;

	float fNow = vtGetTime();
	if (m_fLastTime==DBL_MAX)
		m_fLastTime = fNow;

	float fElapsed = fNow - m_fLastTime;
	m_fTime += fElapsed * m_fSpeed;

	ControlPoint cp;
	if (m_pAnimationPath->GetInterpolatedControlPoint(m_fTime, cp))
	{
		FMatrix4 matrix;
		cp.GetMatrix(matrix, m_bPosOnly);
		if (m_bPosOnly)
		{
			// Only copy position
			FPoint3 pos = matrix.GetTrans();
			tr->SetTrans(pos);
		}
		else
			tr->SetTransform1(matrix);
	}
	m_fLastTime = fNow;
}

void vtAnimPathEngine::Reset()
{
	m_fTime = 0;
}


///////////////////////////////////////////////////////////////////////

void vtAnimPath3d::TransformToTerrain(const vtProjection &proj)
{
	// Clear our control points because we're going to fill it 
	m_TimeControlPointMap.clear();

	OCT *trans = NULL;
	if (!proj.IsSame(&m_proj))
	{
		// need transformation from feature CRS to terrain CRS
		trans = CreateCoordTransform(&m_proj, &proj, true);
	}

	DPoint3 current, previous(1E9,1E9,1E9);
	FPoint3 pos;

//	for (unsigned int i = 0; i < GetNumEntities(); i++)
	unsigned int i = 0, j;	// only first entity

	const DLine3 &dline = GetPolyLine(i);
	FLine3 fline;
	for (j = 0; j < dline.GetSize(); j++)
	{
		current = dline[j];

		// Must skip redundant points, smooth (spline) paths don't like them
		if (current == previous)
			continue;

		// Transform 1: feature CRS to terrain CRS
		if (trans)
			trans->Transform(1, &current.x, &current.y);

		// Transform 2: earth CRS to world CRS
		g_Conv.convert_earth_to_local_xz(current.x, current.y, pos.x, pos.z);
		pos.y = current.z;

		fline.Append(pos);

		previous = current;
	}
	double time = 0;
	for (j = 0; j < fline.GetSize(); j++)
	{
		// SHP file has no orientations, so derive from positions
		FPoint3 diff;
		if (j == 0)
			diff = (fline[j+1] - fline[j]);
		else
			diff = (fline[j] - fline[j-1]);

		if (j > 0)
		{
			// estimate time based on linear distance
			float distance = diff.Length();

			// default correlation: 1 approximate meter per second
			time += distance;
		}
		FMatrix3 m3;
		m3.MakeOrientation(diff, true);
		FQuat quat;
		quat.SetFromMatrix(m3);

		// Add as a control point
		Insert(time, ControlPoint(fline[j], quat));

	}

	// Set up spline, in case they want smooth motion
	ProcessPoints();
}

