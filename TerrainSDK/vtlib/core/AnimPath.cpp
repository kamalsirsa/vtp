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


void ControlPoint::Interpolate(float ratio, const ControlPoint &first, const ControlPoint &second)
{
	float one_minus_ratio = 1.0f - ratio;
	m_Position = first.m_Position*one_minus_ratio + second.m_Position*ratio;
	m_fRotation.Slerp(first.m_fRotation, second.m_fRotation, ratio);
	m_Scale = first.m_Scale*one_minus_ratio + second.m_Scale*ratio;
}

void ControlPoint::GetMatrix(FMatrix4 &matrix) const
{
	matrix.MakeScale(m_Scale.x, m_Scale.y, m_Scale.z);

	FMatrix3 m3;
	m_fRotation.GetMatrix(m3);

	FMatrix4 m4;
	m4.SetFromMatrix3(m3);
	matrix.PostMult(m4);

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

double vtAnimPathEngine::GetAnimationTime() const
{
	return (m_fLatestTime - m_fFirstTime) * m_fTimeMultiplier;
}

void vtAnimPathEngine::Eval()
{
	if (m_pAnimationPath == NULL)
		return;

	vtTarget *target = GetTarget();
	vtTransform *tr = dynamic_cast<vtTransform*>(target);
	if (!tr)
		return;

	m_fLatestTime = vtGetTime();
	if (m_fFirstTime==DBL_MAX)
		m_fFirstTime = m_fLatestTime;

	ControlPoint cp;
	if (m_pAnimationPath->GetInterpolatedControlPoint(GetAnimationTime(),cp))
	{
		FMatrix4 matrix;
		cp.GetMatrix(matrix);
		tr->SetTransform1(matrix);
	}
}

void vtAnimPathEngine::Reset()
{
	m_fFirstTime = m_fLatestTime;
}

