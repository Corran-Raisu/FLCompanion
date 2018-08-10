// SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FLCompanion.h"
#include "SpeedDelaysDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpeedDelaysDlg dialog


CSpeedDelaysDlg::CSpeedDelaysDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpeedDelaysDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpeedDelaysDlg)
	m_engineSpeed = 0;
	m_laneSpeed = 0;
	m_jumpDelay = 0;
	m_holeDelay = 0;
	m_laneDelay = 0;
	m_baseDelay = 0;
	//}}AFX_DATA_INIT
}


void CSpeedDelaysDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpeedDelaysDlg)
	DDX_Text(pDX, IDC_ENGINE_SPEED, m_engineSpeed);
	DDV_MinMaxUInt(pDX, m_engineSpeed, 1, 4294967295);
	DDX_Text(pDX, IDC_JUMP_DELAY, m_jumpDelay);
	DDX_Text(pDX, IDC_LANE_DELAY, m_laneDelay);
	DDX_Text(pDX, IDC_LANE_SPEED, m_laneSpeed);
	DDV_MinMaxUInt(pDX, m_laneSpeed, 1, 4294967295);
	DDX_Text(pDX, IDC_BASE_DELAY, m_baseDelay);
	DDX_Text(pDX, IDC_HOLE_DELAY, m_holeDelay);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpeedDelaysDlg, CDialog)
	//{{AFX_MSG_MAP(CSpeedDelaysDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpeedDelaysDlg message handlers

