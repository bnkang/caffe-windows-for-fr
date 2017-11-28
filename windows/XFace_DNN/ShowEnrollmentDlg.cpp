// ShowEnrollmentDlg.cpp : implementation file
//

#include "stdafx.h"
#include "XFace_DNN.h"
#include "ShowEnrollmentDlg.h"


// CShowEnrollmentDlg dialog

IMPLEMENT_DYNAMIC(CShowEnrollmentDlg, CDialog)

CShowEnrollmentDlg::CShowEnrollmentDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShowEnrollmentDlg::IDD, pParent)
{

}

CShowEnrollmentDlg::~CShowEnrollmentDlg()
{
}

void CShowEnrollmentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CShowEnrollmentDlg, CDialog)
	ON_COMMAND(ID_POPUP_DELETEENROLLMENTGALLERY, &CShowEnrollmentDlg::OnPopupDeleteenrollmentgallery)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


// CShowEnrollmentDlg message handlers



void CShowEnrollmentDlg::OnPopupDeleteenrollmentgallery()
{
	// TODO: Add your command handler code here
	if( MessageBox("Do you really want to delete this face?", "PIM_FaceRecognition", MB_OKCANCEL) != IDOK ) 
	{
		AfxMessageBox("Crazy~~~~~~~~~~~~~~~~~~~~~");

		return;
	}
}

void CShowEnrollmentDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: Add your message handler code here
	if( pWnd != NULL )
		return;

// 	if( ( m_nSelectedEnrollFaceIndex=GetRMouseSelectedEnrollFace(point) ) == -1 ) 
// 		return;

	CPoint pt = point;
	//ClientToScreen(&pt);

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_POPUP_MENU2));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
}
