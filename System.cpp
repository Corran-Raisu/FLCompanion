// System.cpp: implementation of the CSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FLCompanion.h"
#include "System.h"
#include "Math.h"
#include "Datas.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

int SYSTEMS_COUNT;
CSystem g_systems[SYSTEMS_MAX];
CSortedMap<CString,LPCTSTR,CSystem*,CSystem*> g_systemsByNick;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSystem::CSystem() : m_navmapscale(1), m_avoid(false), m_x(0), m_y(0)
{
}

CSystem::~CSystem()
{
}

void CSystem::Init(const CString &nickname, const CString &caption, const CString &file)
{
	m_nickname = nickname;
	m_caption = caption;
	m_file = file;
}

void CSystem::SetMapScale(double navmapscale)
{
	m_navmapscale = navmapscale;
}

void CSystem::AddBase(CBase *base)
{
	m_bases.AddTail(base);
}

void CSystem::RemoveBase(CBase *base)
{
	m_bases.RemoveAt(m_bases.Find(base));
}

void CSystem::ResetPath()
{
	CString dummy;
	POSITION pos = m_bases.GetHeadPosition();
	while (pos)
	{
		CBase &base = *m_bases.GetNext(pos);
		base.ResetPath();
	}
	pos = m_jumpsByNick.GetStartPosition();
	while (pos)
	{
		CJump &jump = m_jumpsByNick.GetNextAssoc(pos, dummy);
		jump.ResetPath();
	}
	
}

void dumplane(CSystem *system)
{/*
	CString name, name2;
	CSortedArray<CString,CString&> names;
	POSITION pos = system->m_lanesByNick.GetStartPosition();
	while (pos)
	{
		CLane &lane = system->m_lanesByNick.GetNextAssoc(pos, name);
		names.Add(name);
	}
	int nCount = names.GetSize();
	for (int nIndex = 0; nIndex < nCount; nIndex++)
	{
		name = names.GetAt(nIndex);
		CLane &lane = system->m_lanesByNick[name];
		if (lane.m_closest)
		{
			pos = system->m_lanesByNick.GetStartPosition();
			while (pos)
			{
				CLane &lane2 = system->m_lanesByNick.GetNextAssoc(pos, name2);
				if (&lane2 == lane.m_closest)
					break;
			}
		}
		else
			name2.Empty();
		OutputDebugFormat(L"%-25s : %6d -> %s\n", name, lane.m_distance, name2);
	}
	OutputDebugString(L"-\n");*/
}
void CSystem::CalcLaneDistances(CDockable &to)
{
	//TODO: build up a CSortedMap with the distances, to scan them in increasing order
	CString dummy;
	POSITION pos = m_lanesByNick.GetStartPosition();
	while (pos)
	{
		CLane &lane = m_lanesByNick.GetNextAssoc(pos, dummy);
		lane.m_closest = NULL; // NULL means direct path, outside the lane
		lane.m_distance = Distance(lane, to.m_pos) * 1000 / ENGINE_SPEED + LANE_DELAY;
	}
	BOOL changeMade = false;
	BOOL debug = false;
	BOOL firstPass = false;
	do
	{
		if (debug) dumplane(this);
		if (firstPass)
			pos = m_laneEnds.GetHeadPosition();
		else
			pos = m_lanesByNick.GetStartPosition();
		while (pos)
		{
			CLane *lane = firstPass ? m_laneEnds.GetNext(pos) : &m_lanesByNick.GetNextAssoc(pos, dummy);
			CLane *lane2 = lane;
			while (lane2 = lane2->m_prev)
			{
				if (lane2->m_distance <= lane->m_distance)
					break;
				UINT dist = lane->m_distance + Distance(*lane, *lane2) * 1000 / LANE_SPEED;
				if (lane2->m_distance <= dist)
					break;
				lane2->m_distance = dist;
				lane2->m_closest = lane;
			}
			lane2 = lane;
			while (lane2 = lane2->m_next)
			{
				if (lane2->m_distance <= lane->m_distance)
					break;
				UINT dist = lane->m_distance + Distance(*lane, *lane2) * 1000 / LANE_SPEED;
				if (lane2->m_distance <= dist)
					break;
				lane2->m_distance = dist;
				lane2->m_closest = lane;
			}
			if (debug) dumplane(this);
		}
		if (debug) dumplane(this);
		changeMade = false;
		pos = m_laneEnds.GetHeadPosition();
		while (pos)
		{
			CLane *lane = m_laneEnds.GetNext(pos);
			POSITION pos2 = m_laneEnds.GetHeadPosition();
			while (pos2)
			{
				CLane *lane2 = m_laneEnds.GetNext(pos2);
				if (lane2->m_distance <= lane->m_distance)
					continue;
				UINT dist = lane->m_distance + Distance(*lane, *lane2) * 1000 / ENGINE_SPEED + LANE_DELAY;
				if (lane2->m_distance <= dist)
					continue;
				lane2->m_distance = dist;
				lane2->m_closest = lane;
				changeMade = true;
			}
		}
		firstPass = false;
	} while (changeMade);
}

UINT CSystem::ComputeDistance(CDockable &from, CDockable &to, CLane **pbestRoute)
{
	UINT bestDistance = Distance(from.m_pos, to.m_pos) * 1000 / ENGINE_SPEED;
	CLane *bestRoute = NULL;

	CString dummy;
	POSITION pos = m_lanesByNick.GetStartPosition();
	while (pos)
	{
		CLane &lane = m_lanesByNick.GetNextAssoc(pos, dummy);
		if (lane.m_distance >= bestDistance)
			continue;
		UINT dist = lane.m_distance + Distance(from.m_pos, lane) * 1000 / ENGINE_SPEED;
		if (dist >= bestDistance)
			continue;
		bestDistance = dist;
		bestRoute = &lane;
	}
	if (pbestRoute) *pbestRoute = bestRoute;
	return bestDistance;
}