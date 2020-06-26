// Faction.cpp: implementation of the CBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Faction.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

UINT FACTIONS_COUNT;
CFaction g_factions[FACTIONS_MAX];
CSortedMap<CString,LPCTSTR,CFaction*,CFaction*> g_factionsByNick;

UINT ID_COUNT;
CFaction g_ID[FACTIONS_MAX];
CSortedMap<CString, LPCTSTR, CFaction*, CFaction*> g_IDByNick;

void CFaction::Init(const CString &nickname, const CString &caption)
{
	m_nickname = nickname;
	m_caption = caption;
}

void CReputation::Init(const CString &nickname, const FLOAT &reputation)
{
	m_nickname = nickname;
	m_Reputation = reputation;
}