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
}

void ControlPoint::GetMatrix(FMatrix4 &matrix, bool bPosOnly) const
{
	matrix.Identity();

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

double vtAnimPath::GetFirstTime() const
{
	if (m_TimeControlPointMap.empty())
		return 0.0;
	return m_TimeControlPointMap.begin()->first;
}

double vtAnimPath::GetLastTime() const
{
	if (m_TimeControlPointMap.empty())
		return 0.0;
	if (m_bLoop && m_TimeControlPointMap.size() >= 2)
		return m_fLoopSegmentTime;
	else
		return m_TimeControlPointMap.rbegin()->first;
}

void vtAnimPath::SetLoop(bool bFlag)
{
	m_bLoop = bFlag;

	// must reprocess with smoothing of the beginning and end
	if (m_TimeControlPointMap.size() >= 2)
		ProcessPoints();
}

void vtAnimPath::SetTimeFromLinearDistance()
{
	FPoint3 pos, last;
	float fTime=0;

	TimeControlPointMap newmap;

	for (TimeControlPointMap::iterator it = m_TimeControlPointMap.begin();
		it != m_TimeControlPointMap.end(); it++)
	{
		pos = it->second.m_Position;
		if (it != m_TimeControlPointMap.begin())
		{
			float dist = (pos - last).Length();
			fTime += dist;
		}
		last = pos;

		newmap[fTime] = it->second;
	}
	// Now switch to the new map
	m_TimeControlPointMap = newmap;
}

void vtAnimPath::ProcessPoints()
{
	m_Spline.Cleanup();

	if (m_TimeControlPointMap.size() < 2)
	{
		m_fLoopSegmentTime = 0.0f;
		return;
	}

	TimeControlPointMap::iterator first = m_TimeControlPointMap.begin();
	TimeControlPointMap::iterator last = m_TimeControlPointMap.end();
	last--;

	if (m_bLoop)
	{
		// Estimate time for last segment to make it loop
		TimeControlPointMap::const_iterator it0 = m_TimeControlPointMap.begin();
		TimeControlPointMap::const_iterator it1 = it0;
		it1++;
		TimeControlPointMap::const_reverse_iterator it2 = m_TimeControlPointMap.rbegin();

		// consider first two points
		float time_diff, pos_diff;
		time_diff = (it1->first - it0->first);
		pos_diff = (it1->second.m_Position - it0->second.m_Position).Length();
		float speed = pos_diff / time_diff;

		// then do the same for the last two
		pos_diff = (it0->second.m_Position - it2->second.m_Position).Length();
		m_fLoopSegmentTime = it2->first + (pos_diff/speed);

		m_LoopControlPoint = it0->second;
	}

	TimeControlPointMap::iterator it;

	if (m_bLoop)
	{
		// one more at the beginning to wraparound-loop smoothly
		m_Spline.AddPoint(last->second.m_Position);
	}

	it = first;
	int i = 0;
	while (it != m_TimeControlPointMap.end())
	{
		it->second.m_iIndex = i;
		m_Spline.AddPoint(it->second.m_Position);
		it++;
		i++;
	}

	if (m_bLoop)
	{
		// two more at the end to wraparound-loop smoothly
		it = first;
		m_Spline.AddPoint(it->second.m_Position);
		it++;
		m_Spline.AddPoint(it->second.m_Position);
	}

	// now create smooth curve, in case it's needed later
	m_Spline.Generate();
}

void vtAnimPath::InterpolateControlPoints(TimeControlPointMap::const_iterator &a,
										  TimeControlPointMap::const_iterator &b,
										  double time,
										  ControlPoint &result) const
{
	double delta_time = b->first - a->first;
	if (delta_time == 0.0)
		result = a->second;
	else
	{
		double elapsed = time - a->first;
		double ratio = elapsed/delta_time;

		result.Interpolate(ratio, a->second, b->second);
		if (m_InterpMode == CUBIC_SPLINE)
		{
			// Don't use that linear position.
			// Find the position on the spline.
			int num = a->second.m_iIndex;

			if (m_bLoop)	// indices are +1 for loop spline
				num++;

			DPoint3 dpos;
			m_Spline.Interpolate(num + ratio, &dpos);
			result.m_Position = dpos;
		}
	}
}

bool vtAnimPath::GetInterpolatedControlPoint(double time, ControlPoint &controlPoint) const
{
	if (m_TimeControlPointMap.empty())
		return false;

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
		InterpolateControlPoints(first, second, time, controlPoint);
	}
	else // (second==_timeControlPointMap.end())
	{
		if (m_bLoop && m_TimeControlPointMap.size() >= 2)
		{
			TimeControlPointMap dummy;
			dummy[m_fLoopSegmentTime] = m_LoopControlPoint;
			TimeControlPointMap::const_iterator it2 = dummy.begin();

			second--;
			InterpolateControlPoints(second, it2, time, controlPoint);
		}
		else
			controlPoint = m_TimeControlPointMap.rbegin()->second;
	}
	return true;
}

float vtAnimPath::GetTotalTime()
{
	if (m_bLoop)
	{
	}
	return 0;
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

void vtAnimPathEngine::Eval()
{
	if (m_pAnimationPath == NULL)
		return;

	float fNow = vtGetTime();
	if (m_fLastTime==DBL_MAX)
		m_fLastTime = fNow;

	float fElapsed = fNow - m_fLastTime;
	m_fTime += fElapsed * m_fSpeed;

	if (m_fTime > m_pAnimationPath->GetLastTime())
	{
		if (m_bContinuous)
		{
			// wrap around
			m_fTime -= m_pAnimationPath->GetPeriod();
		}
		else
		{
			// stop at the end
			m_fTime = m_pAnimationPath->GetLastTime();
			SetEnabled(false);
		}
	}

	UpdateTargets();

	m_fLastTime = fNow;
}

void vtAnimPathEngine::UpdateTargets()
{
	for (unsigned int i = 0; i < NumTargets(); i++)
	{
		vtTarget *target = GetTarget(i);
		vtTransform *tr = dynamic_cast<vtTransform*>(target);
		if (!tr)
			continue;

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
	}
}

void vtAnimPathEngine::Reset()
{
	m_fTime = 0;
}


///////////////////////////////////////////////////////////////////////

void vtAnimPath::CreateFromLineString(const vtProjection &proj,
										vtFeatureSetLineString3D &lines)
{
	// Clear our control points because we're going to fill it 
	m_TimeControlPointMap.clear();

	OCT *trans = NULL;
	vtProjection &line_proj = lines.GetAtProjection();
	if (!proj.IsSame(&line_proj))
	{
		// need transformation from feature CRS to terrain CRS
		trans = CreateCoordTransform(&line_proj, &proj, true);
	}

	DPoint3 current, previous(1E9,1E9,1E9);
	FPoint3 pos;

//	for (unsigned int i = 0; i < GetNumEntities(); i++)
	unsigned int i = 0, j;	// only first entity

	const DLine3 &dline = lines.GetPolyLine(i);
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

