// CompanyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "modbusConfigDialog.h"
#include "modbusConfigDialogDlg.h"
//#include "c5db.h"
#include "CompanyDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCompanyDialog dialog
extern	C5DB m_c5db;

CCompanyDialog::CCompanyDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCompanyDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCompanyDialog)
	m_edit_companydesc_val = _T("");
	m_edit_companyremark_val = _T("");
	m_edit_bigtypedesc_val = _T("");
	m_edit_bigtyperemark_val = _T("");
	//}}AFX_DATA_INIT
	//(CModbusConfigDialogDlg*)pParent->m_c5db;
	//m_c5db.closeDB();
}


void CCompanyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCompanyDialog)
	DDX_Control(pDX, IDC_LIST_BIGTYPE, m_list_bigtype);
	DDX_Control(pDX, IDC_LIST_COMPANY, m_list_company);
	DDX_Control(pDX, IDC_EDIT_BIGTYPEREMARK, m_edit_bigtyperemark);
	DDX_Control(pDX, IDC_EDIT_BIGTYPEDESC, m_edit_bigtypedesc);
	DDX_Control(pDX, IDC_EDIT_COMPANYREMARK, m_edit_companyremark);
	DDX_Control(pDX, IDC_EDIT_COMPANYDESC, m_edit_companydesc);
	DDX_Text(pDX, IDC_EDIT_COMPANYDESC, m_edit_companydesc_val);
	DDX_Text(pDX, IDC_EDIT_COMPANYREMARK, m_edit_companyremark_val);
	DDX_Text(pDX, IDC_EDIT_BIGTYPEDESC, m_edit_bigtypedesc_val);
	DDX_Text(pDX, IDC_EDIT_BIGTYPEREMARK, m_edit_bigtyperemark_val);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCompanyDialog, CDialog)
	//{{AFX_MSG_MAP(CCompanyDialog)
	ON_BN_CLICKED(IDC_BUTTON_COMPANYADD, OnButtonCompanyadd)
	ON_BN_CLICKED(IDC_BUTTON_BIGTYPEADD, OnButtonBigtypeadd)
	ON_LBN_SELCHANGE(IDC_LIST_COMPANY, OnSelchangeListCompany)
	ON_LBN_SELCHANGE(IDC_LIST_BIGTYPE, OnSelchangeListBigtype)
	ON_BN_CLICKED(IDC_BUTTON_COMPANYUPDATE, OnButtonCompanyupdate)
	ON_BN_CLICKED(IDC_BUTTON_COMPANYUPDATE2, OnButtonCompanyupdate2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCompanyDialog message handlers

void CCompanyDialog::OnButtonCompanyadd() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	
	if(m_edit_companydesc_val.GetLength() == 0){
		MessageBox("请填写描述");
		return;
	}

	COMPANYSTR companyStr;
	int id = 0;
	int len = vCompany.size();
	bool has;
	while(id < len + 1){
		has = false;
		for(int i = 0; i < len; i++){
			int tempid;
			tempid = atoi(vCompany.at(i).id);
			if(tempid == id){
				has = true;
				continue;
			}
		}
		if(!has){
			break;
		}
		id++;
	}
	
	companyStr.id.Format("%d",id);
	companyStr.desc = m_edit_companydesc_val;
	companyStr.remark = m_edit_companyremark_val;

	CString sqlStr;
	sqlStr.Format("insert into TB1101_COMPANY ([F1101_CODE],[F1101_DESC],[F1101_REMARK]) values ('%s','%s','%s');",
		companyStr.id, companyStr.desc, companyStr.remark);
	if(m_c5db.executeSQL(sqlStr)){
		MessageBox("添加成功");
	}else{
		MessageBox("添加失败");
	}
}

void CCompanyDialog::OnButtonBigtypeadd() 
{
	// TODO: Add your control notification handler code here
	int company_sel = m_list_company.GetCurSel();
	if(company_sel < 0){
		MessageBox("请选择厂家！"); 
		return;
	}
	UpdateData(TRUE);
	
	if(m_edit_bigtypedesc_val.GetLength() == 0){
		MessageBox("请填写描述");
		return;
	}

	BIGTYPESTR bigtypeStr;
	int id = 0;
	int len = vBigtype.size();
	bool has;
	while(id < len + 1){
		has = false;
		for(int i = 0; i < len; i++){
			int tempid;
			tempid = atoi(vBigtype.at(i).id);
			if(tempid == id){
				has = true;
				continue;
			}
		}
		if(!has){
			break;
		}
		id++;
	}
	
	bigtypeStr.id.Format("%d",id);
	bigtypeStr.desc = m_edit_bigtypedesc_val;
	bigtypeStr.remark = m_edit_bigtyperemark_val;

	CString sqlStr;
	sqlStr.Format("insert into TB1102_PROTOCOLTYPE ([F1101_CODE],[F1102_CODE],[F1102_DESC],[F1102_REMARK]) values ('%s','%s','%s','%s');",
		vCompany.at(company_sel).id,bigtypeStr.id, bigtypeStr.desc, bigtypeStr.remark);
	if(m_c5db.executeSQL(sqlStr)){
		MessageBox("添加成功");
	}else{
		MessageBox("添加失败");
	}
}

void CCompanyDialog::OnButtonCompanyupdate() 
{
	// TODO: Add your control notification handler code here
	int company_sel = m_list_company.GetCurSel();
	if(company_sel < 0){
		MessageBox("请选择厂家！"); 
		return;
	}
	UpdateData(TRUE);

	COMPANYSTR companyStr = vCompany.at(company_sel);
	vCompany.at(company_sel).desc = m_edit_companydesc_val;
	vCompany.at(company_sel).remark = m_edit_companyremark_val;

	CString sqlStr;
	sqlStr.Format("update TB1101_COMPANY set F1101_DESC = '%s',F1101_REMARK = '%s' where F1101_CODE = '%s';",
		m_edit_companydesc_val, m_edit_companydesc_val, companyStr.id);
	if(m_c5db.executeSQL(sqlStr)){
		MessageBox("更新成功");
	}else{
		MessageBox("更新失败");
	}
}

void CCompanyDialog::OnButtonCompanyupdate2() 
{
	// TODO: Add your control notification handler code here
	int company_sel = m_list_company.GetCurSel();
	if(company_sel < 0){
		MessageBox("请选择厂家！"); 
		return;
	}
	int bigtype_sel = m_list_bigtype.GetCurSel();
	if(bigtype_sel < 0){
		MessageBox("请选择类型！"); 
		return;
	}
	UpdateData(TRUE);

	BIGTYPESTR bigtypeStr = vBigtype.at(bigtype_sel);
	vBigtype.at(bigtype_sel).desc = m_edit_bigtypedesc_val;
	vBigtype.at(bigtype_sel).remark = m_edit_bigtyperemark_val;

	CString sqlStr;
	sqlStr.Format("update TB1102_PROTOCOLTYPE set F1102_DESC= '%s',F1102_REMARK = '%s' where F1102_CODE = '%s';",
		bigtypeStr.desc, bigtypeStr.remark, bigtypeStr.id);
	if(m_c5db.executeSQL(sqlStr)){
		MessageBox("更新成功");
	}else{
		MessageBox("更新失败");
	}
}

bool CCompanyDialog::getCompanyFromC5DB(){

	CString sqlStr;
	sqlStr.Format("select [F1101_CODE],[F1101_DESC],[F1101_REMARK] from TB1101_COMPANY  order by convert(int, F1101_CODE);");
	_RecordsetPtr m_pRecordset;
	if(m_c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			COMPANYSTR companystr;

			varName = m_pRecordset->GetCollect ("F1101_CODE");
			companystr.id = (char *)_bstr_t(varName);
			companystr.id.TrimRight();
			varName = m_pRecordset->GetCollect ("F1101_DESC");
			companystr.desc = (char *)_bstr_t(varName);
			companystr.desc.TrimRight();
			varName = m_pRecordset->GetCollect ("F1101_REMARK");
			companystr.remark = (char *)_bstr_t(varName);
			companystr.remark.TrimRight();
			
			vCompany.push_back(companystr);
			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}
bool CCompanyDialog::getBigtypeFromC5DB(CString companyid){
	CString sqlStr;
	sqlStr.Format("select [F1101_CODE],[F1102_CODE],[F1102_DESC],[F1102_REMARK] from TB1102_PROTOCOLTYPE where F1101_CODE = '%s' order by convert(int, F1102_CODE);",
		companyid);
	_RecordsetPtr m_pRecordset;
	vBigtype.clear();
	if(m_c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			BIGTYPESTR bigtypestr;

			varName = m_pRecordset->GetCollect ("F1101_CODE");
			bigtypestr.companyid = (char *)_bstr_t(varName);
			bigtypestr.companyid.TrimRight();
			varName = m_pRecordset->GetCollect ("F1102_CODE");
			bigtypestr.id = (char *)_bstr_t(varName);
			bigtypestr.id.TrimRight();
			varName = m_pRecordset->GetCollect ("F1102_DESC");
			bigtypestr.desc = (char *)_bstr_t(varName);
			bigtypestr.desc.TrimRight();
			varName = m_pRecordset->GetCollect ("F1102_REMARK");
			bigtypestr.remark = (char *)_bstr_t(varName);
			bigtypestr.remark.TrimRight();
			
			vBigtype.push_back(bigtypestr);
			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}

BOOL CCompanyDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	getCompanyFromC5DB();
	refreshCompanyList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCompanyDialog::refreshCompanyList(){
	m_list_company.ResetContent();
	int len = vCompany.size();
	for(int i = 0; i < len; i++){
		CString str;
		COMPANYSTR companystr = vCompany.at(i);
		str.Format("%s: %s  -  %s", companystr.id, companystr.desc, companystr.remark);
		m_list_company.AddString(str.GetBuffer(0));
	}
}

void CCompanyDialog::refreshBigtypeList(){
	m_list_bigtype.ResetContent();
	int len = vBigtype.size();
	for(int i = 0; i < len; i++){
		CString str;
		BIGTYPESTR bigtypestr = vBigtype.at(i);
		str.Format("%s: %s  -  %s", bigtypestr.id, bigtypestr.desc, bigtypestr.remark);
		m_list_bigtype.AddString(str.GetBuffer(0));
	}
}

void CCompanyDialog::OnSelchangeListCompany() 
{
	// TODO: Add your control notification handler code here
	int company_sel = m_list_company.GetCurSel();
	COMPANYSTR companystr = vCompany.at(company_sel);
	getBigtypeFromC5DB(companystr.id);
	refreshBigtypeList();

	m_edit_companydesc.SetWindowText(companystr.desc.GetBuffer(0));
	m_edit_companyremark.SetWindowText(companystr.remark.GetBuffer(0));
}

void CCompanyDialog::OnSelchangeListBigtype() 
{
	// TODO: Add your control notification handler code here
	int bigtype_sel = m_list_bigtype.GetCurSel();
	BIGTYPESTR bigtypestr = vBigtype.at(bigtype_sel);

	m_edit_bigtypedesc.SetWindowText(bigtypestr.desc.GetBuffer(0));
	m_edit_bigtyperemark.SetWindowText(bigtypestr.remark.GetBuffer(0));
}


