// Base.h: interface for the CBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASE_H__05038F85_2503_4BC3_B274_7AEA157DA550__INCLUDED_)
#define AFX_BASE_H__05038F85_2503_4BC3_B274_7AEA157DA550__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <float.h>
#include "Dockable.h"
#include "Good.h"
#include "Faction.h"

class CSystem;

class CBase : public CDockable
{
public:
	CBase() : m_hasSell(false) { for (int i = 0; i < GOODS_MAX; i++) m_sell[i] = FLT_MAX; }
	~CBase() {}
	void Init(const CString &nickname, const CString &caption, CSystem *system, const CString &faction, BOOL isfreighteronly);
	void MakeMiningBase(CSystem *system, int x, int y, int z, UINT goodIndex);
	UINT GetDockingDelay();
public:
	//CString m_owner;
	float m_buy[GOODS_MAX];
	float m_sell[GOODS_MAX];
	BOOL m_hasSell;
	CFaction *m_faction;
	int m_ids_info;
	BOOL m_isfreighteronly;
};

extern CBase g_bases[BASES_MAX];
extern CSortedMap<CString,LPCTSTR,CBase*,CBase*> g_basesByNick;

CBase* MakeMiningBase(CSystem* system, int x, int y, int z, UINT goodIndex, double miningSpeed);


#endif // !defined(AFX_BASE_H__05038F85_2503_4BC3_B274_7AEA157DA550__INCLUDED_)
