#pragma once


// CShowEnrollmentDlg dialog

class CShowEnrollmentDlg : public CDialog
{
	DECLARE_DYNAMIC(CShowEnrollmentDlg)

public:
	CShowEnrollmentDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CShowEnrollmentDlg();

// Dialog Data
	enum { IDD = IDD_DLG_SHOW_ENROLL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPopupDeleteGallery();
	afx_msg void OnPopupDeleteenrollmentgallery();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);


public:
	void SetEnrollmentData();
};
