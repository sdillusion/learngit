#if !defined(AFX_LOGINDLG_H__C15CFCE6_3E46_47C6_BB58_92355DE7A673__INCLUDED_)
#define AFX_LOGINDLG_H__C15CFCE6_3E46_47C6_BB58_92355DE7A673__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogInDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLogInDlg dialog

class CLogInDlg : public CDialog
{
// Construction
public:
	CLogInDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLogInDlg)
	enum { IDD = IDD_DIALOG_LOGIN };
	CString	m_edit_username_val;
	CString	m_edit_userpw_val;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogInDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLogInDlg)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGINDLG_H__C15CFCE6_3E46_47C6_BB58_92355DE7A673__INCLUDED_)
