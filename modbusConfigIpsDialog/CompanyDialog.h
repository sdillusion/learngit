#if !defined(AFX_COMPANYDIALOG_H__4326F1FD_7C83_43F7_A964_52B4A46CCA71__INCLUDED_)
#define AFX_COMPANYDIALOG_H__4326F1FD_7C83_43F7_A964_52B4A46CCA71__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CompanyDialog.h : header file
//

//#include "c5db.h"
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CCompanyDialog dialog

struct COMPANYSTR{
	CString id;
	CString desc;
	CString remark;
};
struct BIGTYPESTR{
	CString id;
	CString desc;
	CString companyid;
	CString remark;
};

class CCompanyDialog : public CDialog
{
public:
	
// Construction
public:
	CCompanyDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCompanyDialog)
	enum { IDD = IDD_COMPANYDIALOG };
	CListBox	m_list_bigtype;
	CListBox	m_list_company;
	CEdit	m_edit_bigtyperemark;
	CEdit	m_edit_bigtypedesc;
	CEdit	m_edit_companyremark;
	CEdit	m_edit_companydesc;
	CString	m_edit_companydesc_val;
	CString	m_edit_companyremark_val;
	CString	m_edit_bigtypedesc_val;
	CString	m_edit_bigtyperemark_val;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCompanyDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCompanyDialog)
	afx_msg void OnButtonCompanyadd();
	afx_msg void OnButtonBigtypeadd();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListCompany();
	afx_msg void OnSelchangeListBigtype();
	afx_msg void OnButtonCompanyupdate();
	afx_msg void OnButtonCompanyupdate2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	bool getCompanyFromC5DB();
	bool getBigtypeFromC5DB(CString companyid);

	void refreshCompanyList();
	void refreshBigtypeList();

private:
	vector<COMPANYSTR> vCompany;
	vector<BIGTYPESTR> vBigtype;


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPANYDIALOG_H__4326F1FD_7C83_43F7_A964_52B4A46CCA71__INCLUDED_)
