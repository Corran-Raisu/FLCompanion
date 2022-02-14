// LimitationsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FLCompanion.h"
#include "FLCompanionDlg.h"
#include "LimitationsDlg.h"
#include "System.h"
#include "Faction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLimitationsDlg dialog


CLimitationsDlg::CLimitationsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLimitationsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLimitationsDlg)
	m_cargoSize = 1;
	m_maxInvestment = 0;
	m_maxDistance = 0;
	m_jumptradeTime = 0;
	m_minCSU = 0;
	//}}AFX_DATA_INIT
}


void CLimitationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	if (!pDX->m_bSaveAndValidate)
	{
		m_investmentText = m_maxInvestment == 0 ? "" : IntToString(m_maxInvestment);
		CheckDlgButton(IDC_INVESTMENT_CHECK, m_maxInvestment != 0 ? BST_CHECKED : BST_UNCHECKED);
		m_distanceText = m_maxDistance == 0 ? "" : IntToString(m_maxDistance/60000);
		CheckDlgButton(IDC_DISTANCE_CHECK, m_maxDistance != 0 ? BST_CHECKED : BST_UNCHECKED);
		m_jumptradeText = m_jumptradeTime == 0 ? "" : IntToString(m_jumptradeTime / 1000);
		CheckDlgButton(IDC_DISTANCE_CHECK, m_jumptrade != 0 ? BST_CHECKED : BST_UNCHECKED);
		m_minCSUText = m_minCSU == 0 ? "" : IntToString(m_minCSU);
	}
	//{{AFX_DATA_MAP(CLimitationsDlg)
	DDX_Control(pDX, IDC_GOODS_LIST, m_goodsList);
	DDX_Control(pDX, IDC_FACTIONS_LIST, m_factionsList);
	DDX_Control(pDX, IDC_SYSTEMS_LIST, m_systemsList);
	DDX_Control(pDX, IDC_FACTION_ID, m_factionID);
	DDX_Text(pDX, IDC_CARGO_SIZE, m_cargoSize);
	DDX_Check(pDX, IDC_AVOIDLOCKED, m_avoidLockedGates);
	DDX_Check(pDX, IDC_JUMPTRADE_CHECK, m_jumptrade);
	DDX_Check(pDX, IDC_AVOIDHOLES, m_avoidHoles);
	DDX_Check(pDX, IDC_AVOIDUNSTABLEHOLES, m_avoidUnstableHoles);
	DDX_Check(pDX, IDC_AVOIDGATES, m_avoidGates);
	DDX_Check(pDX, IDC_AVOIDLANES, m_avoidLanes);
	DDX_Check(pDX, IDC_TRANSPORT, m_isTransport);
	DDX_Text(pDX, IDC_INVESTMENT, m_investmentText);
	DDX_Text(pDX, IDC_DISTANCE, m_distanceText);
	DDX_Text(pDX, IDC_JUMPTRADE_TIME, m_jumptradeText);
	DDX_Text(pDX, IDC_MINCSU, m_minCSUText);
	//}}AFX_DATA_MAP
	if (pDX->m_bSaveAndValidate)
	{
		m_maxInvestment = _ttoi(m_investmentText);
		if (!IsDlgButtonChecked(IDC_INVESTMENT_CHECK))
			m_maxInvestment = 0;
		m_maxDistance = _ttoi(m_distanceText)*60000;
		if (!IsDlgButtonChecked(IDC_DISTANCE_CHECK))
			m_maxDistance = 0;
		m_jumptradeTime = _ttoi(m_jumptradeText) * 1000;
		if (!IsDlgButtonChecked(IDC_JUMPTRADE_CHECK))
			m_jumptrade = 0;
		m_minCSU = _ttoi(m_minCSUText);
	}
}


BEGIN_MESSAGE_MAP(CLimitationsDlg, CDialog)
	//{{AFX_MSG_MAP(CLimitationsDlg)
	ON_EN_CHANGE(IDC_INVESTMENT, OnChangeInvestment)
	ON_BN_CLICKED(IDC_CHECK_ALL_FACTIONS,	OnSwitchAllFactions)
	ON_BN_CLICKED(IDC_CHECK_ALL_SYSTEMS,	OnSwitchAllSystems)
	ON_BN_CLICKED(IDC_CHECK_ALL_GOODS,		OnSwitchAllGoods)
	ON_CBN_SELCHANGE(IDC_FACTION_ID,        OnChangeFactionID)
	ON_CONTROL_RANGE(CLBN_CHKCHANGE, IDC_SYSTEMS_LIST, IDC_GOODS_LIST, OnChkchangeList)
	ON_BN_CLICKED(IDC_CLEAR_ALL_SYSTEMS,	OnSwitchAllSystems)
	ON_BN_CLICKED(IDC_CLEAR_ALL_FACTIONS,	OnSwitchAllFactions)
	ON_BN_CLICKED(IDC_CLEAR_ALL_GOODS,		OnSwitchAllGoods)
	ON_EN_CHANGE(IDC_DISTANCE, OnChangeDistance)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLimitationsDlg message handlers



void CLimitationsDlg::OnChangeInvestment() 
{
	CheckDlgButton(IDC_INVESTMENT_CHECK, BST_CHECKED);
	UpdateData();
	CheckDlgButton(IDC_INVESTMENT_CHECK, m_maxInvestment != 0 ? BST_CHECKED : BST_UNCHECKED);
}

void CLimitationsDlg::OnChangeDistance() 
{
	CheckDlgButton(IDC_DISTANCE_CHECK, BST_CHECKED);
	UpdateData();
	CheckDlgButton(IDC_DISTANCE_CHECK, m_maxDistance != 0 ? BST_CHECKED : BST_UNCHECKED);
}

void CLimitationsDlg::OnChangeJumpTrade()
{
	CheckDlgButton(IDC_JUMPTRADE_CHECK, BST_CHECKED);
	UpdateData();
	CheckDlgButton(IDC_JUMPTRADE_CHECK, m_jumptradeTime != 0 ? BST_CHECKED : BST_UNCHECKED);
}

void UpdateClearBtn(CWnd* clearallWnd, int count) 
{
	if (count == 0)
	{
		clearallWnd->SetWindowText(L"Clear all");
		clearallWnd->EnableWindow(FALSE);
	}
	else
	{
		CString str;
		str.Format(L"Clear all (%d)", count);
		clearallWnd->SetWindowText(str);
		clearallWnd->EnableWindow(TRUE);
	}
}
template<class type> void PopulateCombobox(type entries[], int COUNT, CComboBox& listbox)
{
	int checkCount = 0;
	for (int index = 0; index < COUNT; index++)
	{
		type* entry = &entries[index];
		CString caption = g_mainDlg->m_displayNicknames ? entry->m_nickname : entry->m_caption;
		caption.TrimLeft();
		if (caption.IsEmpty()) caption = entry->m_nickname;
		int nIndex = listbox.FindStringExact(-1, caption);
		if (nIndex != LB_ERR)
		{
			type* otherEntry = reinterpret_cast<type*>(listbox.GetItemDataPtr(nIndex));
			if (otherEntry)
			{
				listbox.SetItemDataPtr(nIndex, NULL);
				nIndex = listbox.AddString(caption + L" (" + otherEntry->m_nickname + L")");
				listbox.SetItemDataPtr(nIndex, otherEntry);
			}
			caption += L" (" + entry->m_nickname + L")";
		}
		nIndex = listbox.AddString(caption);
		listbox.SetItemDataPtr(nIndex, entry);
		if (entry->m_avoid) checkCount++;
	}
	int index = listbox.GetCount();
	while (index--)
	{
		if (listbox.GetItemDataPtr(index) == NULL)
			listbox.DeleteString(index);
	}
}

template<class type> void PopulateListbox(type entries[], int COUNT, CCheckListBox& listbox)
{
	int checkCount = 0;
	for (int index = 0; index < COUNT; index++)
	{
		type* entry = &entries[index];
		CString caption = g_mainDlg->m_displayNicknames ? entry->m_nickname : entry->m_caption;
		caption.TrimLeft();
		if (caption.IsEmpty()) caption = entry->m_nickname;
		int nIndex = listbox.FindStringExact(-1, caption);
		if (nIndex != LB_ERR)
		{
			type* otherEntry = reinterpret_cast<type*>(listbox.GetItemDataPtr(nIndex));
			if (otherEntry)
			{
				listbox.SetItemDataPtr(nIndex, NULL);
				nIndex = listbox.AddString(caption+L" ("+otherEntry->m_nickname+L")");
				listbox.SetItemDataPtr(nIndex, otherEntry);
				listbox.SetCheck(nIndex, otherEntry->m_avoid);
			}
			caption += L" ("+entry->m_nickname+L")";
		}
		nIndex = listbox.AddString(caption);
		listbox.SetItemDataPtr(nIndex, entry);
		listbox.SetCheck(nIndex, entry->m_avoid);
		if (entry->m_avoid) checkCount++;
	}	
	int index = listbox.GetCount();
	while (index--)
	{
		if (listbox.GetItemDataPtr(index) == NULL)
			listbox.DeleteString(index);
	}
	UpdateClearBtn(listbox.GetNextWindow()->GetNextWindow(), checkCount);
}

template<class type> void UpdateFromListBox(type entries[], CCheckListBox& listbox, LPCTSTR keyName)
{
	CString excludes;
	int nIndex = listbox.GetCount();
	while (nIndex--)
	{
		type* entry = reinterpret_cast<type*>(listbox.GetItemDataPtr(nIndex));
		entry->m_avoid = listbox.GetCheck(nIndex) != 0;
		if (entry->m_avoid)
			excludes += entry->m_nickname+L',';
	}
	excludes.Delete(excludes.GetLength()-1);
	excludes.MakeLower();
	theApp.WriteProfileString(L"Settings", keyName, excludes);
}

BOOL CLimitationsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	PopulateListbox(g_systems,	SYSTEMS_COUNT,	m_systemsList);
	PopulateListbox(g_factions, FACTIONS_COUNT, m_factionsList);
	PopulateListbox(g_goods,	GOODS_COUNT,	m_goodsList);
	PopulateCombobox(g_ID, ID_COUNT, m_factionID);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLimitationsDlg::OnOK() 
{
	UpdateFromListBox(g_systems,	m_systemsList,	L"ExcludeSystems");
	UpdateFromListBox(g_factions,	m_factionsList,	L"ExcludeFactions");
	UpdateFromListBox(g_goods,		m_goodsList,	L"ExcludeGoods");
	CDialog::OnOK();
}

void CLimitationsDlg::SwitchAll(CCheckListBox& listbox, int clearallID) 
{
	int nCheck = LOWORD(GetCurrentMessage()->wParam) != clearallID;
	int index = listbox.GetCount();
	UpdateClearBtn(GetDlgItem(clearallID), nCheck ? index : 0);
	while (index--)
		listbox.SetCheck(index, nCheck);
}

void CLimitationsDlg::OnChangeFactionID()
{
	int nIndex = m_factionID.GetCurSel();
	CFaction *f = (nIndex == CB_ERR) ? NULL : (CFaction*)m_factionID.GetItemDataPtr(nIndex);
	//CFaction f = m_factionID.GetItemDataPtr(m_factionID.GetCurSel());
	for (size_t i = 0; i <= f->m_reputationCount; i++)
	{
		CFaction &r = *g_factionsByNick[f->m_reputations[i].m_nickname];
		if (&r==NULL)
			continue;
		if (f->m_reputations[i].m_Reputation <= -55)
			r.m_avoid = true;
		else
			r.m_avoid = false;
	}
	m_factionsList.ResetContent();
	PopulateListbox(g_factions, FACTIONS_COUNT, m_factionsList);
}

void CLimitationsDlg::OnSwitchAllSystems() 
{
	SwitchAll(m_systemsList,IDC_CLEAR_ALL_SYSTEMS);
}
void CLimitationsDlg::OnSwitchAllFactions() 
{
	SwitchAll(m_factionsList,IDC_CLEAR_ALL_FACTIONS);
}
void CLimitationsDlg::OnSwitchAllGoods() 
{
	SwitchAll(m_goodsList,IDC_CLEAR_ALL_GOODS);
}

void CLimitationsDlg::OnChkchangeList(UINT nID) 
{
	CCheckListBox* listbox = static_cast<CCheckListBox*>(GetDlgItem(nID));
	int count = 0;
	int index = listbox->GetCount();
	while (index--)
		count += listbox->GetCheck(index);
	UpdateClearBtn(listbox->GetNextWindow()->GetNextWindow(), count);
}

