// MiningBaseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FLCompanion.h"
#include "FLCompanionDlg.h"
#include "MiningBaseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMiningBaseDlg dialog


CMiningBaseDlg::CMiningBaseDlg(CAsteroids* asteroids, CWnd* pParent /*=NULL*/)
	: CDialog(CMiningBaseDlg::IDD, pParent), m_asteroids(asteroids)
{
	//{{AFX_DATA_INIT(CMiningBaseDlg)
	m_lootStat = asteroids->m_lootStat;
	m_miningSpeed = 1.0;
	m_goodsPrice = 0;
	//}}AFX_DATA_INIT
}


void CMiningBaseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMiningBaseDlg)
	DDX_Control(pDX, IDC_MINING_SPEED, m_miningSpeedEdit);
	DDX_Control(pDX, IDC_GOODS_LIST, m_goodsCombo);
	DDX_Text(pDX, IDC_LOOT_STAT, m_lootStat);
	DDX_Text(pDX, IDC_MININGGOODS_PRICE, m_goodsPrice);
	DDX_Text(pDX, IDC_MINING_SPEED, m_miningSpeed);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMiningBaseDlg, CDialog)
	//{{AFX_MSG_MAP(CMiningBaseDlg)
	ON_EN_CHANGE(IDC_LOOT_STAT, OnChangeStats)
	ON_EN_CHANGE(IDC_MINING_SPEED, OnChangeStats)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMiningBaseDlg message handlers

BOOL CMiningBaseDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	for (UINT goodIndex = 0; goodIndex < GOODS_COUNT; goodIndex++)
	{
		int nIndex = m_goodsCombo.AddString(g_mainDlg->m_displayNicknames ? g_goods[goodIndex].m_nickname : g_goods[goodIndex].m_caption);
		m_goodsCombo.SetItemData(nIndex, goodIndex);
		if (*m_asteroids->m_good == goodIndex)
			m_goodsCombo.SetCurSel(nIndex);
	}
	CString str;
	GetDlgItemText(IDC_CARGO_SIZE, str);
	str.FormatMessage(str, g_mainDlg->m_cargoSize);
	SetDlgItemText(IDC_CARGO_SIZE, str);
	OnChangeStats();
	m_miningSpeedEdit.SetSel(0, -1);
	m_miningSpeedEdit.SetFocus();
	return FALSE;  // return TRUE unless you set the focus to a control
}

void CMiningBaseDlg::OnChangeStats() 
{
	UpdateData();
	SetDlgItemText(IDC_MINING_TIME, L"~ "+MinuteSeconds(UINT(g_mainDlg->m_cargoSize*1000/(m_lootStat*m_miningSpeed))));
}

void CMiningBaseDlg::OnOK() 
{
	m_goodIndex = m_goodsCombo.GetItemData(m_goodsCombo.GetCurSel());
	CDialog::OnOK();
}
