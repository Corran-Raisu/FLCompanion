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


void CFaction::Init(const CString &nickname, const CString &caption)
{
	m_nickname = nickname;
	m_caption = caption;
}