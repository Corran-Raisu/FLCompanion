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


void CSpeedDelaysDlg::SetDefaults()
{
	//{{AFX_DATA_INIT(CSpeedDelaysDlg)
	SetDlgItemText(IDC_ENGINE_SPEED, L"350");
	SetDlgItemText(IDC_LANE_SPEED, L"1900");
	SetDlgItemText(IDC_JUMP_DELAY, L"15");
	SetDlgItemText(IDC_HOLE_DELAY, L"15");
	SetDlgItemText(IDC_LANE_DELAY, L"10");
	SetDlgItemText(IDC_BASE_DELAY, L"20");
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CSpeedDelaysDlg, CDialog)
	//{{AFX_MSG_MAP(CSpeedDelaysDlg)
	ON_BN_CLICKED(IDC_DEFAULTSPEED, SetDefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpeedDelaysDlg message handlers

