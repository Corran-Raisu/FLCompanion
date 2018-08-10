// Good.h: interface for the CGood class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GOOD_H__E114EA7A_5A62_445E_A87F_D4E8891CDE7D__INCLUDED_)
#define AFX_GOOD_H__E114EA7A_5A62_445E_A87F_D4E8891CDE7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define GOODS_MAX	1024
extern UINT GOODS_COUNT;

class CGood;
extern CGood g_goods[GOODS_MAX];

class CGood
{
public:
	CGood() : m_avoid(false) {}
	~CGood() {}
public:
	void InitPrice(float defaultPrice);
	void Init(const CString &nickname, const CString &caption, float volume, UINT decay_time);
	CString m_caption;
	CString m_nickname;
	float m_volume;
	float m_defaultPrice;
	UINT m_decay_time; // number of milliseconds before decay of 1 unit
	bool m_avoid;
	operator UINT() { return this-g_goods; }
};

extern CSortedMap<CString,LPCTSTR,CGood*,CGood*> g_goodsByNick;

#endif // !defined(AFX_GOOD_H__E114EA7A_5A62_445E_A87F_D4E8891CDE7D__INCLUDED_)
