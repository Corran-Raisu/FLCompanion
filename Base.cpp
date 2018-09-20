// Base.cpp: implementation of the CBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FLCompanion.h"
#include "FLCompanionDlg.h"
#include "Base.h"
#include "System.h"
#include "Datas.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBase g_bases[BASES_MAX];
CBase* g_miningBase;
UINT g_miningDelay;
CSortedMap<CString,LPCTSTR,CBase*,CBase*> g_basesByNick;

void CBase::Init(const CString &nickname, const CString &caption, CSystem *system, const CString &faction, BOOL isfreighteronly)
{
	CDockable::Init(nickname, caption, system);
	m_isfreighteronly = isfreighteronly;
	for (UINT i = 0; i < GOODS_MAX; i++)	
		m_buy[i] = g_goods[i].m_defaultPrice;
	if (faction.IsEmpty())
		m_faction = NULL;
	else
	{
		m_faction = g_factionsByNick[faction];
		if (m_faction == NULL)
			ProblemFound(L"Base %s references an unknown faction %s", nickname, faction);
	}
}

UINT CBase::GetDockingDelay()
{
	if (this == g_miningBase)
		return g_miningDelay;
	else
		return BASE_DELAY;
}

void CBase::MakeMiningBase(CSystem *system, int x, int y, int z, UINT goodIndex, double goodsPrice)
{
	CDockable::Init(L"=mining operation=", L"<Mining Operation>", system);
	for (UINT i = 0; i < GOODS_MAX; i++) m_buy[i] = 0;
	m_sell[goodIndex] = goodsPrice;
	m_faction = NULL;
	SetPos(x,y,z);
}

CBase* MakeMiningBase(CSystem* system, int x, int y, int z, UINT goodIndex, double miningSpeed, double goodsPrice)
{
	if (g_miningBase == NULL)
		g_miningBase = &g_bases[BASES_COUNT++];
	else
		g_miningBase->m_system->RemoveBase(g_miningBase);
	g_miningDelay = UINT(g_mainDlg->m_cargoSize*1000/miningSpeed);
	g_miningBase->MakeMiningBase(system, x, y, z, goodIndex,goodsPrice);
	system->AddBase(g_miningBase);
	return g_miningBase;
}