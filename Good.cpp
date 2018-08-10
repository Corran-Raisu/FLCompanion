// Good.cpp: implementation of the CGood class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FLCompanion.h"
#include "Good.h"
#include "Datas.h"
#include "float.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CGood g_goods[GOODS_MAX];
CSortedMap<CString,LPCTSTR,CGood*,CGood*> g_goodsByNick;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CGood::InitPrice(float defaultPrice)
{
	m_defaultPrice = defaultPrice;
}

void CGood::Init(const CString &nickname, const CString &caption, float volume, UINT decay_time)
{
	m_nickname = nickname;
	m_caption = caption;
	m_volume = volume == 0.0 ? FLT_MAX : volume;
	m_decay_time = decay_time;
}
