#if !defined(AFX_ENROLLDLG_H__1736EF53_3CC0_487F_9302_C7F0D71DCD70__INCLUDED_)
#define AFX_ENROLLDLG_H__1736EF53_3CC0_487F_9302_C7F0D71DCD70__INCLUDED_

class CEnrollDlg : public CDialog
{
public:
	CEnrollDlg(CWnd* pParent = NULL);  
	int		 m_nRFIDHandle;

	//{{AFX_DATA(CEnrollDlg)
	enum { IDD = IDD_DIALOG_ENROLL };
	CString	m_strName;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CEnrollDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX); 
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CEnrollDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

#endif 
