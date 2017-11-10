// LogInDlg.cpp : implementation file
//

#include "stdafx.h"
#include "modbusconfigdialog.h"
#include "LogInDlg.h"
#include "modbusConfigDialogDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogInDlg dialog

extern	C5DB m_c5db;

CLogInDlg::CLogInDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogInDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLogInDlg)
	m_edit_username_val = _T("");
	m_edit_userpw_val = _T("");
	//}}AFX_DATA_INIT
}


void CLogInDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogInDlg)
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_edit_username_val);
	DDX_Text(pDX, IDC_EDIT_USERPW, m_edit_userpw_val);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLogInDlg, CDialog)
	//{{AFX_MSG_MAP(CLogInDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogInDlg message handlers

void CLogInDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData();
	CString sqlStr;
	sqlStr.Format("select count(*) as counts from TB0101_USER where F0101_NAME = '%s' and F0101_PW = '%s';", m_edit_username_val, m_edit_userpw_val);
	_RecordsetPtr m_pRecordset;
	if(m_c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;

			varName = m_pRecordset->GetCollect ("counts");
			int num = varName.intVal;
			if(num>0){
				CDialog::OnOK();
				return;
			}else{
				MessageBox("用户名密码错误");
				return;
			}
			
			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		MessageBox("查询错误");
		return;
	}
}
