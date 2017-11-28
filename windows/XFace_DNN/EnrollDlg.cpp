#include "stdafx.h"
#include "XFace_DNN.h"
#include "EnrollDlg.h"

CEnrollDlg::CEnrollDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEnrollDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEnrollDlg)
	m_strName = _T("");
	//}}AFX_DATA_INIT
}


void CEnrollDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEnrollDlg)
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEnrollDlg, CDialog)
	//{{AFX_MSG_MAP(CEnrollDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CEnrollDlg::OnBnClickedOk)
END_MESSAGE_MAP()

void CEnrollDlg::OnOK() 
{
	UpdateData(TRUE);
	m_strName.TrimLeft(" \t\n");
	m_strName.TrimRight(" \t\n");
	if(m_strName.GetLength() <= 0) {
		AfxMessageBox("Please input your name.");
		return;
	}	
	UpdateData(FALSE);
	CDialog::OnOK();
}

BOOL CEnrollDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_strName = "";
	UpdateData(FALSE);	
	return TRUE;  
}

BOOL CEnrollDlg::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN) {
		OnOK();
		return TRUE;
    }
	return CDialog::PreTranslateMessage(pMsg);
}


void CEnrollDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
