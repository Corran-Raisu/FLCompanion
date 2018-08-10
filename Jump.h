// Jump.h: interface for the CJump class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JUMP_H__7A8453C1_712C_46E8_B439_CE24DB1BD9F9__INCLUDED_)
#define AFX_JUMP_H__7A8453C1_712C_46E8_B439_CE24DB1BD9F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Dockable.h"
#include "Base.h"

class CSystem;

class CJump : public CDockable
{
public:
	CJump() {}
	~CJump() {}

public:
	bool m_isgate;
	bool m_islocked;
	void Init(const CString &nickname, bool isgate, bool islocked, const CString &caption, CSystem *system);
	void SetMatchingJump(CJump *matchingJump);
	CJump *m_matchingJump;
	virtual CString LetterPos();
};

#endif // !defined(AFX_JUMP_H__7A8453C1_712C_46E8_B439_CE24DB1BD9F9__INCLUDED_)
