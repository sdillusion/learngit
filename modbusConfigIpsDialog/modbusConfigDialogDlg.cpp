// modbusConfigDialogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "modbusConfigDialog.h"
#include "modbusConfigDialogDlg.h"
#include "CompanyDialog.h"
#include "LogInDlg.h"
#include "c5db.h"
#include "sqlite3.h"
//#include "libdb.h"

#include <iostream>
#include <string>
#include <sstream>
//#include <fstream>
#include <vector>
//#include <stdio.h>
//#include <io.h>

#include <time.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

using namespace std;
//#import "c:\program files\common files\system\ado\msado15.dll"  no_namespace rename("EOF", "adoEOF")
//using namespace System;

/////////////////////////////////////////////////////////////////////////////
// CModbusConfigDialogDlg dialog

const char* inipath = ".\\method.ini";
const char* txtpath = "method.txt";
/*
struct RTUSTRUCT{
	char desc[CHARLEN];
	int rtuno;
};
*/

//vector<RTUSTRUCT>vRtu;
vector<METHODSTRUCT>vMethod;
vector<bigstruct> vbig;

vector<COMPANYSTRUCT>vCompany;
vector<BIGTYPESTRUCT>vBigtype;

sqlite3 * pDB = NULL;
char gDBpath[CHARLEN];

struct yctype gYcType;
struct yxtype gYxType;
struct ddtype gDdType;
//struct yktype gYkType;
//struct yttype gYtType;

//commonData commond;

//bigstruct bigs[SECTIONNUM];


bool gIsChanged = false;
const int YCNUMLIMITE = 30;
const int YXNUMLIMITE = 17;
const int DDNUMLIMITE = 20;
//int gSys = -1;

//C5DB m_c5db = C5DB("111.231.135.99","wontex@1");
C5DB m_c5db = C5DB("127.0.0.1","Qwertyuiop1");
//C5DB m_c5db = C5DB("192.168.1.30","Wontex@1");
//C5DB m_c5db = C5DB("111.231.135.99","wontex@1");

CModbusConfigDialogDlg::CModbusConfigDialogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModbusConfigDialogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModbusConfigDialogDlg)
	m_combo_addr_val = _T("");
	m_combo_horl_val = 0;
	m_combo_crc_val = 0;
	m_combo_method_val = 0;
	m_combo_datalen_val = 0;
	m_edit_datanum_val = 0;
	m_edit_startindex_val = 0;
	m_combo_addrhorl_val = 0;
	m_edit_reqnum_val = 0;
	m_edit_cbdatalenbit_val = 0;
	m_edit_bigid_val = 0;
	m_combo_cbdatahorl_val = 0;
	m_check_hasnan_val = FALSE;
	m_edit_nankey_val = 0;
	m_edit_nanvalue_val = 0;
	m_check_bigused_val = FALSE;
	m_edit_remark_val = _T("");
	m_combo_company_val = -1;
	m_combo_bigtype_val = -1;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_state = 0;
	
	m_group_sel = -1;
	m_type_sel = -1;
	

}

void CModbusConfigDialogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModbusConfigDialogDlg)
	DDX_Control(pDX, IDC_COMBO_BIGTYPE, m_combo_bigtype);
	DDX_Control(pDX, IDC_COMBO_COMPANY, m_combo_company);
	DDX_Control(pDX, IDC_CHECK_BIGUSED, m_check_bigused);
	DDX_Control(pDX, IDC_EDIT_NANVALUE, m_edit_nanvalue);
	DDX_Control(pDX, IDC_EDIT_NANKEY, m_edit_nankey);
	DDX_Control(pDX, IDC_CHECK_HASNAN, m_check_hasnan);
	DDX_Control(pDX, IDC_EDIT_BIGID, m_edit_bigid);
	DDX_Control(pDX, IDC_EDIT_REQNUM, m_edit_reqnum);
	DDX_Control(pDX, IDC_RADIO_YC, m_radio_yc);
	DDX_Control(pDX, IDC_STATIC_TYPEINDEX, m_static_typeindex);
	DDX_Control(pDX, IDC_EDIT_GROUPDESC, m_edit_groupdesc);
	DDX_Control(pDX, IDC_EDIT_TYPEDESC, m_edit_typedesc);
	DDX_Control(pDX, IDC_LIST_TYPE, m_list_type);
	DDX_Control(pDX, IDC_COMBO_DATALEN, m_combo_datalen);
	DDX_Control(pDX, IDC_COMBO_METHOD, m_combo_method);
	DDX_Control(pDX, IDC_LIST_GROUP, m_list_group);
	DDX_Control(pDX, IDC_EDIT_TMP, m_edit_tmp);
	DDX_Control(pDX, IDC_LIST_ATTR, m_list_attr);
	DDX_CBString(pDX, IDC_COMBO_ADDR, m_combo_addr_val);
	DDX_CBIndex(pDX, IDC_COMBO_HORL, m_combo_horl_val);
	DDX_CBIndex(pDX, IDC_COMBO_CRC, m_combo_crc_val);
	DDX_CBIndex(pDX, IDC_COMBO_METHOD, m_combo_method_val);
	DDX_CBIndex(pDX, IDC_COMBO_DATALEN, m_combo_datalen_val);
	DDX_Text(pDX, IDC_EDIT_DATANUM, m_edit_datanum_val);
	DDX_CBIndex(pDX, IDC_COMBO_ADDRHORL, m_combo_addrhorl_val);
	DDX_Text(pDX, IDC_EDIT_REQNUM, m_edit_reqnum_val);
	DDX_Text(pDX, IDC_EDIT_CBDATALENBIT, m_edit_cbdatalenbit_val);
	DDX_Text(pDX, IDC_EDIT_BIGID, m_edit_bigid_val);
	DDX_CBIndex(pDX, IDC_COMBO_CBDATAHORL, m_combo_cbdatahorl_val);
	DDX_Check(pDX, IDC_CHECK_HASNAN, m_check_hasnan_val);
	DDX_Text(pDX, IDC_EDIT_NANKEY, m_edit_nankey_val);
	DDX_Text(pDX, IDC_EDIT_NANVALUE, m_edit_nanvalue_val);
	DDX_Check(pDX, IDC_CHECK_BIGUSED, m_check_bigused_val);
	DDX_Text(pDX, IDC_EDIT_REMARK, m_edit_remark_val);
	DDX_CBIndex(pDX, IDC_COMBO_COMPANY, m_combo_company_val);
	DDX_CBIndex(pDX, IDC_COMBO_BIGTYPE, m_combo_bigtype_val);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CModbusConfigDialogDlg, CDialog)
	//{{AFX_MSG_MAP(CModbusConfigDialogDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ATTR, OnDblclkListAttr)
	ON_EN_KILLFOCUS(IDC_EDIT_TMP, OnKillfocusEditTmp)
	ON_BN_CLICKED(IDC_BUTTON_ADDGROUP, OnButtonAddgroup)
	ON_LBN_SELCHANGE(IDC_LIST_GROUP, OnSelchangeListGroup)
	ON_CBN_EDITCHANGE(IDC_COMBO_ADDR, OnEditchangeComboAddr)
	ON_CBN_SELCHANGE(IDC_COMBO_ADDR, OnSelchangeComboAddr)
	ON_CBN_SELCHANGE(IDC_COMBO_HORL, OnSelchangeComboHorl)
	ON_CBN_SELCHANGE(IDC_COMBO_DATALEN, OnSelchangeComboDatalen)
	ON_CBN_SELCHANGE(IDC_COMBO_CRC, OnSelchangeComboCrc)
	ON_CBN_SELCHANGE(IDC_COMBO_METHOD, OnSelchangeComboMethod)
	ON_BN_CLICKED(IDC_BUTTON_ADDTYPE, OnButtonAddType)
	ON_LBN_SELCHANGE(IDC_LIST_TYPE, OnSelchangeListType)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, OnButtonOpen)
	ON_LBN_DBLCLK(IDC_LIST_GROUP, OnDblclkListGroup)
	ON_BN_CLICKED(IDC_BUTTON_CHANGETYPEDESC, OnButtonChangetypedesc)
	ON_BN_CLICKED(IDC_BUTTON_CHANGEGROUPDESC, OnButtonChangegroupdesc)
	ON_BN_CLICKED(IDC_BUTTON_NEWDB, OnButtonNewdb)
	ON_BN_CLICKED(IDC_BUTTON_SAVECONFIGTODB, OnButtonSaveconfigtodb)
	ON_BN_CLICKED(IDC_BUTTON_ADDLISTCTRL, OnButtonAddlistctrl)
	ON_BN_CLICKED(IDC_BUTTON_DELETELISTCTRL, OnButtonDeletelistctrl)
	ON_BN_CLICKED(IDC_RADIO_YC, OnRadioYc)
	ON_BN_CLICKED(IDC_RADIO_DD, OnRadioDd)
	ON_BN_CLICKED(IDC_RADIO_YX, OnRadioYx)
	ON_BN_CLICKED(IDC_BUTTON_DELETETYPE, OnButtonDeletetype)
	ON_BN_CLICKED(IDC_BUTTON_DELETEGROUP, OnButtonDeletegroup)
	ON_CBN_SELCHANGE(IDC_COMBO_ADDRHORL, OnSelchangeComboAddrhorl)
	ON_EN_CHANGE(IDC_EDIT_REQNUM, OnChangeEditReqnum)
	ON_BN_CLICKED(IDC_CHECK_HASNAN, OnCheckHasnan)
	ON_EN_CHANGE(IDC_EDIT_NANKEY, OnChangeEditNankey)
	ON_EN_CHANGE(IDC_EDIT_NANVALUE, OnChangeEditNanvalue)
	ON_BN_CLICKED(IDC_BUTTON_METHOD_EXPLAIN, OnButtonMethodExplain)
	ON_BN_CLICKED(IDC_BUTTON_TESTC5DB, OnButtonTestc5db)
	ON_BN_CLICKED(IDC_CHECK_BIGUSED, OnCheckBigused)
	ON_EN_CHANGE(IDC_EDIT_REMARK, OnChangeEditRemark)
	ON_CBN_SELCHANGE(IDC_COMBO_COMPANY, OnSelchangeComboCompany)
	ON_CBN_SELCHANGE(IDC_COMBO_BIGTYPE, OnSelchangeComboBigtype)
	ON_BN_CLICKED(IDC_BUTTON_ADDCOMPANY, OnButtonAddcompany)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModbusConfigDialogDlg message handlers

BOOL CModbusConfigDialogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//ShowWindow(SW_SHOWMAXIMIZED);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	getMethodFromFile();

	m_edit_tmp.ShowWindow(SW_HIDE);

	m_list_attr.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	m_list_attr.InsertColumn(0,"序号",LVCFMT_CENTER);
	m_list_attr.InsertColumn(1,"点号",LVCFMT_CENTER);
	m_list_attr.InsertColumn(2,"描述",LVCFMT_CENTER);
	m_list_attr.InsertColumn(3,"系数",LVCFMT_CENTER);

	m_list_attr.SetColumnWidth(0,50);
	m_list_attr.SetColumnWidth(1,0);
	m_list_attr.SetColumnWidth(2,140);
	m_list_attr.SetColumnWidth(3,80);

	//m_list_attr.DeleteColumn(3);
	//int columnNum = m_list_attr.GetHeaderCtrl()->GetItemCount();
	//initListCtrl(10);
	
	setComboOption();

	//m_list_attr.InsertItem(0,"0");
	//m_list_attr.DeleteAllItems();
	//int i = m_list_attr.GetItemCount();//获取list里面有多少行  
	//CString ttt;  
	//ttt.Format("%d",i);  
	//m_list_attr.InsertItem(i,ttt);//开辟一个行，并且设置行的内容为i的内容  
	//m_list_attr.SetItemText(i,1,"Str_Name");//i代指在第几行插入数据，第二个参数代指第几列，第三个参数代指插入数据的值  

	//m_list_attr.SetItemText(2,2,"test");
	m_radio_yc.SetCheck(1);

	if(!m_c5db.openDB()){
		MessageBox("打开MSSQL数据库时出错");
		EndDialog(IDCANCEL);
		return TRUE;
	}

	CLogInDlg loginDlg;
	if(loginDlg.DoModal()!=IDOK){
		EndDialog(IDCANCEL);
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CModbusConfigDialogDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CModbusConfigDialogDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CModbusConfigDialogDlg::OnDblclkListAttr(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView=(NM_LISTVIEW*)pNMHDR;
	CRect rc;  
    m_Row = pNMListView->iItem;//获得选中的行  
    m_Col = pNMListView->iSubItem;//获得选中列  

    if   (m_Row >= 0 && pNMListView->iSubItem != 0 && m_Col != 1) //如果选择的是子项;  
    {  
        m_list_attr.GetSubItemRect(m_Row,m_Col,LVIR_LABEL,rc);//获得子项的RECT；  
        m_edit_tmp.SetParent(&m_list_attr);//转换坐标为列表框中的坐标  
        m_edit_tmp.MoveWindow(rc);//移动Edit到RECT坐在的位置;  
        m_edit_tmp.SetWindowText(m_list_attr.GetItemText(m_Row,m_Col));//将该子项中的值放在Edit控件中；  
        m_edit_tmp.ShowWindow(SW_SHOW);//显示Edit控件；  
        m_edit_tmp.SetFocus();//设置Edit焦点  
        m_edit_tmp.ShowCaret();//显示光标  
        m_edit_tmp.SetSel(-1);//将光标移动到最后  
    }  
	
	*pResult = 0;
}

void CModbusConfigDialogDlg::OnKillfocusEditTmp() 
{
	// TODO: Add your control notification handler code here
	
	CString tem;  
	m_edit_tmp.GetWindowText(tem);					//得到用户输入的新的内容  
	m_list_attr.SetItemText(m_Row,m_Col,tem);	//设置编辑框的新内容  
	m_edit_tmp.ShowWindow(SW_HIDE);					//隐藏编辑框  
	
	gIsChanged = true;
	saveConfig();
}

void CModbusConfigDialogDlg::OnButtonAddType() 
{
	// TODO: Add your control notification handler code here
	checkIsChanged();

	CString tem;
	m_edit_typedesc.GetWindowText(tem);
	if(!tem.GetLength()){
		MessageBox("请先填写规约名");
		return;
	}
	//m_typeNum++;
	bigstruct bigstr;
	int id = 1;
	int len = vbig.size();
	bool has;
	while(id < len + 1){
		has = false;
		for(int i = 0; i < len; i++){
			int tempid;
			tempid = atoi(vbig.at(i).id);
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
	itoa(id, bigstr.id, 10);
	
	
	strcpy(bigstr.desc, tem.GetBuffer(0));
	bigstr.checkcrc = -1;
	bigstr.addrhorl = 'l';
	vbig.push_back(bigstr);
	
	int ret = openDB();
	if(ret == -1){
		MessageBox("请新建或打开数据库");
		return;
	}
	insertDataBigToDB(bigstr);
	bigQuery();
	closeDB();

	//if(m_c5db.openDB()){
		insertDataBigToC5DB(bigstr, m_c5db);
		C5DBbigQuery(m_c5db);

	//	m_c5db.closeDB();
	//}
	
	refreshTypeUI();
}


void CModbusConfigDialogDlg::OnButtonAddgroup() 
{
	// TODO: Add your control notification handler code here
	
	if(m_list_type.GetCurSel() < 0){
		MessageBox("请选择规约！"); 
		return;
	}

	CString tem;  
	m_edit_groupdesc.GetWindowText(tem);
	if(!tem.GetLength()){
		MessageBox("请先填写组名");
		return;
	}

	if(m_state == 0){
		if(gYcType.ycseclen >= SECTIONNUM){
			MessageBox("超出数量限制");
			return;
		}

		int id = 0;
		int len = gYcType.ycseclen;
		bool has;
		while(id < len + 1){
			has = false;
			for(int i = 0; i < len; i++){
				int tempid;
				tempid = atoi(gYcType.m_ycss[i].id);
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
		itoa(id, gYcType.m_ycss[gYcType.ycseclen].id, 10);
		
		strcpy(gYcType.m_ycss[gYcType.ycseclen].desc, tem.GetBuffer(0));
		gYcType.ycseclen++;
	}else if(m_state == 1){
		if(gYxType.yxseclen >= SECTIONNUM){
			MessageBox("超出数量限制");
			return;
		}

		int id = 0;
		int len = gYxType.yxseclen;
		bool has;
		while(id < len + 1){
			has = false;
			for(int i = 0; i < len; i++){
				int tempid;
				tempid = atoi(gYxType.m_yxss[i].id);
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
		itoa(id, gYxType.m_yxss[gYxType.yxseclen].id, 10);
		
		strcpy(gYxType.m_yxss[gYxType.yxseclen].desc, tem.GetBuffer(0));
		gYxType.yxseclen++;
	}else if(m_state == 2){
		if(gDdType.ddseclen >= SECTIONNUM){
			MessageBox("超出数量限制");
			return;
		}

		int id = 0;
		int len = gDdType.ddseclen;
		bool has;
		while(id < len + 1){
			has = false;
			for(int i = 0; i < len; i++){
				int tempid;
				tempid = atoi(gDdType.m_ddss[i].id);
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
		itoa(id, gDdType.m_ddss[gDdType.ddseclen].id, 10);
		
		strcpy(gDdType.m_ddss[gDdType.ddseclen].desc, tem.GetBuffer(0));
		gDdType.ddseclen++;
	}
	refreshGroupListBox();
}

void CModbusConfigDialogDlg::refreshGroupListBox(){
	m_list_group.ResetContent();
	if(m_state == 0){
		int len = gYcType.ycseclen;
		for(int i = 0; i < len; i++){
			m_list_group.AddString(gYcType.m_ycss[i].desc);
		}
	}else if(m_state == 1){
		int len = gYxType.yxseclen;
		for(int i = 0; i < len; i++){
			m_list_group.AddString(gYxType.m_yxss[i].desc);
		}
	}else if(m_state == 2){
		int len = gDdType.ddseclen;
		for(int i = 0; i < len; i++){
			m_list_group.AddString(gDdType.m_ddss[i].desc);
		}
	}
}

void CModbusConfigDialogDlg::updateBigidInDB(char* oldid, char* newid){
	CString str;
	str.Format("UPDATE big SET id = '%s' WHERE id = '%s';UPDATE yc SET bigid = '%s' WHERE bigid = '%s';UPDATE yx SET bigid = '%s' WHERE bigid = '%s';UPDATE dd SET bigid = '%s' WHERE bigid = '%s';",newid,oldid ,newid,oldid ,newid,oldid ,newid,oldid);
	char pstr[256];
	strcpy(pstr,str);
	char* cErrMsg;
	int nRes = sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("更新bigid失败！"); 
	}
}

void CModbusConfigDialogDlg::updateBigDescInDB(char* oldid, char* newdesc){
	CString str;
	str.Format("UPDATE big SET desc = '%s' WHERE id = '%s';",newdesc,oldid);
	char pstr[256];
	strcpy(pstr,str);
	char* cErrMsg;
	int nRes = sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("更新big描述失败！"); 
	}
}


void CModbusConfigDialogDlg::deleteGroup(int index){

	if(m_state == 0){
		for(int i = index; i < gYcType.ycseclen; i++){
			gYcType.m_ycss[i] = gYcType.m_ycss[i+1];
		}
		gYcType.ycseclen--;
	}else if(m_state == 1){
		for(int i = index; i < gYxType.yxseclen; i++){
			gYxType.m_yxss[i] = gYxType.m_yxss[i+1];
		}
		gYxType.yxseclen--;
	}else if(m_state == 2){
		for(int i = index; i < gDdType.ddseclen; i++){
			gDdType.m_ddss[i] = gDdType.m_ddss[i+1];
		}
		gDdType.ddseclen--;	
	}
	gIsChanged = true;
	m_list_group.DeleteString(index);
}

void CModbusConfigDialogDlg::deleteType(int index){

	//m_typeNum--;
	int ret = openDB();
	if(ret == -1){
		MessageBox("请新建或打开数据库");
		return;
	}
	deleteTypeInDB(vbig.at(index).id);
	bigQuery();
	closeDB();
	
	//if(m_c5db.openDB()){
		deleteTypeInC5DB(vbig.at(index).id, m_c5db);
		C5DBbigQuery(m_c5db);

	//	m_c5db.closeDB();
	//}

	m_list_group.ResetContent();
	refreshTypeUI();
	clearUI();
}

void CModbusConfigDialogDlg::deleteTypeInDB(char* bigid){
	char* cErrMsg;
	CString sqlStr;
	char pstr[512];
	
	sqlStr.Format("delete from big where id = '%s'",bigid);
	strcpy(pstr,sqlStr);
	int nRes = sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("删除big表数据失败！"); 
	}
	deleteYcInDB(bigid);
	deleteYxInDB(bigid);
	deleteDdInDB(bigid);
}

void CModbusConfigDialogDlg::deleteYcInDB(char *bigid){
	char* cErrMsg;
	CString sqlStr;
	char pstr[512];
	
	sqlStr.Format("delete from yc where bigid = '%s'",bigid);
	strcpy(pstr,sqlStr);
	int nRes = sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("删除yc表数据失败！"); 
	}
}

void CModbusConfigDialogDlg::deleteYxInDB(char *bigid){
	char* cErrMsg;
	CString sqlStr;
	char pstr[512];
	
	sqlStr.Format("delete from yx where bigid = '%s'",bigid);
	strcpy(pstr,sqlStr);
	int nRes = sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("删除yx表数据失败！"); 
	}
}

void CModbusConfigDialogDlg::deleteDdInDB(char *bigid){
	char* cErrMsg;
	CString sqlStr;
	char pstr[512];
	
	sqlStr.Format("delete from dd where bigid = '%s'",bigid);
	strcpy(pstr,sqlStr);
	int nRes = sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("删除dd表数据失败！"); 
	}
}


void CModbusConfigDialogDlg::deleteYkInDB(char *bigid){
	char* cErrMsg;
	CString sqlStr;
	char pstr[512];
	
	sqlStr.Format("delete from yk where bigid = '%s'",bigid);
	strcpy(pstr,sqlStr);
	int nRes = sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("删除yk表数据失败！"); 
	}
}

void CModbusConfigDialogDlg::deleteYtInDB(char *bigid){
	char* cErrMsg;
	CString sqlStr;
	char pstr[512];
	
	sqlStr.Format("delete from yt where bigid = '%s'",bigid);
	strcpy(pstr,sqlStr);
	int nRes = sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("删除yt表数据失败！"); 
	}
}


void CModbusConfigDialogDlg::setComboOption(){
	if(m_state == 1){
		m_combo_datalen.EnableWindow(false);
		m_check_hasnan.EnableWindow(false);
		m_edit_nankey.EnableWindow(false);
		m_edit_nanvalue.EnableWindow(false);
		m_edit_reqnum.EnableWindow();
	}else if(m_state ==0 || m_state == 2){
		m_combo_datalen.EnableWindow();
		m_edit_reqnum.EnableWindow(false);
		m_check_hasnan.EnableWindow();
		m_edit_nankey.EnableWindow();
		m_edit_nanvalue.EnableWindow();
	}
	m_combo_method.ResetContent();
	int len = vMethod.size();
	for(int i = 0; i < len; i++){
		m_combo_method.AddString(vMethod.at(i).desc.GetBuffer(0));
	}
}

void CModbusConfigDialogDlg::OnSelchangeListGroup() 
{
	// TODO: Add your control notification handler code here
	int index = m_list_group.GetCurSel();
	m_group_sel = index;
	if(m_state == 0){
		m_edit_groupdesc.SetWindowText(gYcType.m_ycss[m_group_sel].desc);
	}else if(m_state == 1){
		m_edit_groupdesc.SetWindowText(gYxType.m_yxss[m_group_sel].desc);
	}else if(m_state == 2){
		m_edit_groupdesc.SetWindowText(gDdType.m_ddss[m_group_sel].desc);
	}
	refreshUI();
}

void CModbusConfigDialogDlg::OnSelchangeListType() 
{
	// TODO: Add your control notification handler code here
	checkIsChanged();
	int index = m_list_type.GetCurSel();
	m_type_sel = index;

	////////初始化
	memset(&gYcType,   0,   sizeof(gYcType));
	memset(&gYxType,   0,   sizeof(gYxType));
	memset(&gDdType,   0,   sizeof(gDdType));
	///////

	bigstruct bigstr = vbig.at(index);
	/*
	int res = openDB();
	if(res == -1){
		MessageBox("请新建或打开数据库");
		return;
	}

	ycQuery(bigstr.id);
	yxQuery(bigstr.id);
	ddQuery(bigstr.id);
	int ret = closeDB();
	*/

	//if(m_c5db.openDB()){
		C5DBycGroupQuery(bigstr.id, m_c5db);
		C5DByxGroupQuery(bigstr.id, m_c5db);
		C5DBddGroupQuery(bigstr.id, m_c5db);
		int i,l;
		for(i = 0,l=gYcType.ycseclen; i < l ; i++){
			C5DBycQuery(bigstr.id,gYcType.m_ycss[i].id,i,m_c5db);
		}
		for(i = 0,l=gYxType.yxseclen; i < l ; i++){
			C5DByxQuery(bigstr.id,gYxType.m_yxss[i].id,i,m_c5db);
		}
		for(i = 0,l=gDdType.ddseclen; i < l ; i++){
			C5DBddQuery(bigstr.id,gDdType.m_ddss[i].id,i,m_c5db);
		}
		//m_c5db.closeDB();
	//}

	m_edit_typedesc.SetWindowText(bigstr.desc);

	//m_edit_bigid.SetWindowText(bigstr.id);
	m_edit_bigid_val = atoi(bigstr.id);
	m_static_typeindex.SetWindowText(bigstr.id);

	refreshGroupListBox();
	
	clearUI();

	m_combo_crc_val = bigstr.checkcrc;
	m_combo_addrhorl_val = bigstr.addrhorl == 'h' ? 0 : 1;
	m_check_bigused_val = bigstr.used ? true : false;
	//
	getBigTypeFromC5DBByBig(CString(bigstr.type));
	m_combo_bigtype.ResetContent();
	int len = vBigtype.size();
	for(i = 0; i < len; i++){
		m_combo_bigtype.AddString(vBigtype.at(i).desc.GetBuffer(0));
	}

	m_combo_bigtype_val = FindIndexInBigtypeVector(vBigtype, CString(bigstr.type));
	if(m_combo_bigtype_val >= 0){
		m_combo_company_val = FindIndexInCompanyVector(vCompany, vBigtype.at(m_combo_bigtype_val).companyid);
	}else{
		m_combo_company_val = -1;
	}

	m_edit_remark_val.Format("%s", bigstr.remark);
	UpdateData(FALSE);
}

void CModbusConfigDialogDlg::clearUI(){
	m_combo_addr_val = "";
	m_edit_startindex_val = 0;
	m_combo_datalen_val = 0;
	m_edit_datanum_val = 0;
	m_combo_horl_val = 0;
	m_combo_method_val = 0;
	m_edit_cbdatalenbit_val = 1;
	m_combo_cbdatahorl_val = 0;
	m_check_hasnan_val = false;
	m_edit_nankey_val = 0;
	m_edit_nanvalue_val = 0;
	UpdateData(FALSE);
	initListCtrl(m_edit_datanum_val);
}

void CModbusConfigDialogDlg::refreshUI(){
	if(m_state == 0){
		refreshUIyc();
	}else if(m_state == 1){
		refreshUIyx();
	}else if(m_state == 2){
		refreshUIdd();
	}
	UpdateData(FALSE);
}

//void CModbusConfigDialogDlg::refreshUIcommon(){
//	m_combo_crc_val = commond.cbcheckcrc;
//}

void CModbusConfigDialogDlg::refreshUIyc(){
	m_combo_addr_val.Format("%d", gYcType.m_ycss[m_group_sel].addr);
	m_edit_startindex_val = gYcType.m_ycss[m_group_sel].startindex;
	m_edit_cbdatalenbit_val = gYcType.m_ycss[m_group_sel].cbdatalenbit ? gYcType.m_ycss[m_group_sel].cbdatalenbit : 1;
	m_combo_cbdatahorl_val = gYcType.m_ycss[m_group_sel].cbdatahorl == 'h' ? 0 : 1;
	m_check_hasnan_val = gYcType.m_ycss[m_group_sel].hasnan ? true : false;
	m_edit_nankey_val = gYcType.m_ycss[m_group_sel].nankey;
	m_edit_nanvalue_val = gYcType.m_ycss[m_group_sel].nanvalue;
	if(gYcType.m_ycss[m_group_sel].ycdatalen == 2){
		m_combo_datalen_val = 0;
	}else if(gYcType.m_ycss[m_group_sel].ycdatalen == 4){
		m_combo_datalen_val = 1;
	}else if(gYcType.m_ycss[m_group_sel].ycdatalen == 8){
		m_combo_datalen_val = 2;
	}
	m_edit_datanum_val = gYcType.m_ycss[m_group_sel].ycslen;
	if(gYcType.m_ycss[m_group_sel].ycs[0].horl == 'h'){
		m_combo_horl_val = 0;
	}else if(gYcType.m_ycss[m_group_sel].ycs[0].horl == 'l'){
		m_combo_horl_val = 1;
	}
	m_combo_method_val = FindIndexInMethodVector(vMethod, gYcType.m_ycss[m_group_sel].ycs[0].method);
	UpdateData(FALSE);

	initListCtrl(m_edit_datanum_val);
	updateListCtrlData();
}

void CModbusConfigDialogDlg::refreshUIyx(){
	m_edit_reqnum_val = gYxType.m_yxss[m_group_sel].yxnum;
	m_combo_addr_val.Format("%d", gYxType.m_yxss[m_group_sel].addr);
	m_edit_startindex_val = gYxType.m_yxss[m_group_sel].startindex;
	m_edit_datanum_val = gYxType.m_yxss[m_group_sel].yxslen;
	m_edit_cbdatalenbit_val = gYxType.m_yxss[m_group_sel].cbdatalenbit ? gYxType.m_yxss[m_group_sel].cbdatalenbit : 1;
	m_combo_cbdatahorl_val = gYxType.m_yxss[m_group_sel].cbdatahorl == 'h' ? 0 : 1;
	m_combo_method_val = FindIndexInMethodVector(vMethod, gYxType.m_yxss[m_group_sel].method);
	if(gYxType.m_yxss[m_group_sel].yxs[0].horl == 'h'){
		m_combo_horl_val = 0;
	}else if(gYxType.m_yxss[m_group_sel].yxs[0].horl == 'l'){
		m_combo_horl_val = 1;
	}
	UpdateData(FALSE);

	initListCtrl(m_edit_datanum_val);
	updateListCtrlData();
}

void CModbusConfigDialogDlg::refreshUIdd(){
	m_combo_addr_val.Format("%d", gDdType.m_ddss[m_group_sel].addr);
	m_edit_startindex_val = gDdType.m_ddss[m_group_sel].startindex;
	m_edit_cbdatalenbit_val = gDdType.m_ddss[m_group_sel].cbdatalenbit ? gDdType.m_ddss[m_group_sel].cbdatalenbit :1;
	m_combo_cbdatahorl_val = gDdType.m_ddss[m_group_sel].cbdatahorl == 'h' ? 0 : 1;
	m_check_hasnan_val = gDdType.m_ddss[m_group_sel].hasnan ? true : false;
	m_edit_nankey_val = gDdType.m_ddss[m_group_sel].nankey;
	m_edit_nanvalue_val = gDdType.m_ddss[m_group_sel].nanvalue;
	//m_combo_datalen_val = m_ddss[m_group_sel].dddatalen;
	if(gDdType.m_ddss[m_group_sel].dddatalen == 2){
		m_combo_datalen_val = 0;
	}else if(gDdType.m_ddss[m_group_sel].dddatalen == 4){
		m_combo_datalen_val = 1;
	}else if(gDdType.m_ddss[m_group_sel].dddatalen == 8){
		m_combo_datalen_val = 2;
	}
	m_edit_datanum_val = gDdType.m_ddss[m_group_sel].ddslen;
	if(gDdType.m_ddss[m_group_sel].dds[0].horl == 'h'){
		m_combo_horl_val = 0;
	}else if(gDdType.m_ddss[m_group_sel].dds[0].horl == 'l'){
		m_combo_horl_val = 1;
	}
	m_combo_method_val = FindIndexInMethodVector(vMethod, gDdType.m_ddss[m_group_sel].dds[0].method);
	UpdateData(FALSE);

	initListCtrl(m_edit_datanum_val);
	updateListCtrlData();
}



void CModbusConfigDialogDlg::saveConfig(){
	if(m_list_type.GetCurSel() < 0){
		MessageBox("请选择规约！"); 
		return;
	}
	//if(m_list_group.GetCurSel() < 0){
	//	MessageBox("请选择组！"); 
	//	return;
	//}
	UpdateData(TRUE);

	for(int i = 0; i < m_edit_datanum_val; i++){
		int val;
		if(m_list_attr.GetItemText(i,1).GetLength()){
			val = atoi(m_list_attr.GetItemText(i,1));
		}else{
			val = 999999;
		}
		m_list_attr.SetItemData(i, val); 
	}
	m_list_attr.SortItems(listCtrlCompareProc,(DWORD)&m_list_attr);
	//saveCommonConfig();
	saveBigConfig();
	if(m_list_group.GetCurSel() >= 0){
		if(m_state == 0){
			saveYcConfig();
		}else if(m_state == 1){
			saveYxConfig();
		}else if(m_state == 2){
			saveDdConfig();
		}
	}
	
}


void CModbusConfigDialogDlg::changeListCtrlColumn(){
	int columnNum = m_list_attr.GetHeaderCtrl()->GetItemCount();
	if(m_state == 0){
		if(columnNum == 3){
			m_list_attr.InsertColumn(3,"系数",LVCFMT_CENTER);
			m_list_attr.SetColumnWidth(3,80);
		}
	}else if(m_state == 1){
		if(columnNum == 4){
			m_list_attr.DeleteColumn(3);
		}
	}else if(m_state == 2){
		if(columnNum == 3){
			m_list_attr.InsertColumn(3,"系数",LVCFMT_CENTER);
			m_list_attr.SetColumnWidth(3,80);
		}
	}
}

void CModbusConfigDialogDlg::initListCtrl(int num){
	m_list_attr.DeleteAllItems();
	if(num < 0){
		MessageBox("数据数小于0！"); 
		return;
	}
	
	int len = num;
	for (int i = 0; i < len; i++){
		CString ttt;  
		ttt.Format("%d",i); 
		m_list_attr.InsertItem(i,ttt);
		m_list_attr.SetCheck(i, TRUE);
	}
	changeListCtrlColumn();
}

void CModbusConfigDialogDlg::updateListCtrlData(){
	if(m_state==0){
		for(int i = 0; i < m_edit_datanum_val; i++){
			m_list_attr.SetItemText(i,2,gYcType.m_ycss[m_group_sel].ycs[i].desc);
			CString s;
			float coe = gYcType.m_ycss[m_group_sel].ycs[i].coe;
			s.Format("%.3f",(coe ? coe : 1.00));
			m_list_attr.SetItemText(i,3,s);
			if(gYcType.m_ycss[m_group_sel].ycs[i].dit != -1){
				s.Format("%d",gYcType.m_ycss[m_group_sel].ycs[i].dit);
				m_list_attr.SetItemText(i,1,s);
			}

			bool check = gYcType.m_ycss[m_group_sel].ycs[i].used?TRUE:FALSE;
			m_list_attr.SetCheck(i, check);
		}
	}else if(m_state==1){
		for(int i = 0; i < m_edit_datanum_val; i++){
			m_list_attr.SetItemText(i,2,gYxType.m_yxss[m_group_sel].yxs[i].desc);

			if(gYxType.m_yxss[m_group_sel].yxs[i].dit != -1){
				CString s;
				s.Format("%d",gYxType.m_yxss[m_group_sel].yxs[i].dit);
				m_list_attr.SetItemText(i,1,s);
			}

			bool check = gYxType.m_yxss[m_group_sel].yxs[i].used?TRUE:FALSE;
			m_list_attr.SetCheck(i, check);
		}
	}else if(m_state==2){
		for(int i = 0; i < m_edit_datanum_val; i++){
			m_list_attr.SetItemText(i,2,gDdType.m_ddss[m_group_sel].dds[i].desc);
			CString s;
			float coe = gDdType.m_ddss[m_group_sel].dds[i].coe;
			s.Format("%.3f",(coe ? coe : 1.00));
			m_list_attr.SetItemText(i,3,s);

			if(gDdType.m_ddss[m_group_sel].dds[i].dit != -1){
				s.Format("%d",gDdType.m_ddss[m_group_sel].dds[i].dit);
				m_list_attr.SetItemText(i,1,s);
			}

			bool check = gDdType.m_ddss[m_group_sel].dds[i].used?TRUE:FALSE;
			m_list_attr.SetCheck(i, check);
		}
	}
}

static int CALLBACK listCtrlCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) 
{ 
	return lParam1 - lParam2;
}

//void CModbusConfigDialogDlg::saveCommonConfig(){
//	commond.cbcheckcrc = m_combo_crc_val;
//	commond.checkhorl = m_combo_crc_val == 1?'h':'l';
//}

void CModbusConfigDialogDlg::saveBigConfig(){
	vbig.at(m_type_sel).checkcrc = m_combo_crc_val;
	vbig.at(m_type_sel).addrhorl = m_combo_addrhorl_val == 1?'l':'h';
	vbig.at(m_type_sel).used = m_check_bigused_val ? 1 : 0;
	if(m_combo_bigtype_val >= 0){
		strcpy(vbig.at(m_type_sel).type , vBigtype.at(m_combo_bigtype_val).id.GetBuffer(0));
	}
	strcpy(vbig.at(m_type_sel).remark, m_edit_remark_val.GetBuffer(0));

	//for(int i = 0; i < m_yctypeNum; i++){
	//	char id[32];
	//	itoa(i,id,10);
	//	strcpy(bigs[i].id,id);
	//}
	
}

void CModbusConfigDialogDlg::saveYcConfig(){
	gYcType.m_ycss[m_group_sel].addr = atoi(m_combo_addr_val);
	gYcType.m_ycss[m_group_sel].startindex = m_edit_startindex_val;
	gYcType.m_ycss[m_group_sel].cbdatalenbit = m_edit_cbdatalenbit_val;
	gYcType.m_ycss[m_group_sel].cbdatahorl = m_combo_cbdatahorl_val ? 'l' : 'h';
	gYcType.m_ycss[m_group_sel].hasnan = m_check_hasnan_val ? 1 : 0;
	gYcType.m_ycss[m_group_sel].nankey = m_edit_nankey_val;
	gYcType.m_ycss[m_group_sel].nanvalue = m_edit_nanvalue_val;
	if(m_combo_datalen_val == 0){
		gYcType.m_ycss[m_group_sel].ycdatalen = 2;
	}else if(m_combo_datalen_val == 1){
		gYcType.m_ycss[m_group_sel].ycdatalen = 4;
	}else if(m_combo_datalen_val == 2){
		gYcType.m_ycss[m_group_sel].ycdatalen = 8;
	}
	gYcType.m_ycss[m_group_sel].ycslen = m_edit_datanum_val;
	
	
	int len = gYcType.m_ycss[m_group_sel].ycslen;

	int i = m_list_attr.GetItemText(1,2).GetLength();

	for(i = 0; i < m_edit_datanum_val; i++){
		int val;
		if(m_list_attr.GetItemText(i,1).GetLength()){
			val = atoi(m_list_attr.GetItemText(i,1));
		}else{
			val = 999999;
		}
		m_list_attr.SetItemData(i, val); 
	}
	m_list_attr.SortItems(listCtrlCompareProc,(DWORD)&m_list_attr);

	if(m_combo_horl_val == 0){
			gYcType.m_ycss[m_group_sel].horl = 'h';
		}else if(m_combo_horl_val == 1){
			gYcType.m_ycss[m_group_sel].horl = 'l';
		}else{
			gYcType.m_ycss[m_group_sel].horl = 'n';
		}
	for(i = 0; i < m_edit_datanum_val; i++){

		if(m_combo_horl_val == 0){
			gYcType.m_ycss[m_group_sel].ycs[i].horl = 'h';
		}else if(m_combo_horl_val == 1){
			gYcType.m_ycss[m_group_sel].ycs[i].horl = 'l';
		}else{
			gYcType.m_ycss[m_group_sel].ycs[i].horl = 'n';
		}

		///////////////////////////////////////////////
		gYcType.m_ycss[m_group_sel].ycs[i].method = vMethod.at(m_combo_method_val).no;
		///////////////////////////////////////////////

		strcpy(gYcType.m_ycss[m_group_sel].ycs[i].desc,m_list_attr.GetItemText(i,2));
		gYcType.m_ycss[m_group_sel].ycs[i].coe = atof(m_list_attr.GetItemText(i,3));
		gYcType.m_ycss[m_group_sel].ycs[i].used = m_list_attr.GetCheck(i);
		if(m_list_attr.GetItemText(i,1).GetLength()){
			gYcType.m_ycss[m_group_sel].ycs[i].dit = atoi(m_list_attr.GetItemText(i,1));
		}else{
			gYcType.m_ycss[m_group_sel].ycs[i].dit = -1;
		}
	}
}

void CModbusConfigDialogDlg::saveYxConfig(){
	gYxType.m_yxss[m_group_sel].addr = atoi(m_combo_addr_val);
	gYxType.m_yxss[m_group_sel].startindex = m_edit_startindex_val;
	gYxType.m_yxss[m_group_sel].yxnum = m_edit_reqnum_val;
	gYxType.m_yxss[m_group_sel].cbdatalenbit = m_edit_cbdatalenbit_val;
	gYxType.m_yxss[m_group_sel].cbdatahorl = m_combo_cbdatahorl_val ? 'l' : 'h';
	if(m_combo_datalen_val == 0){
		gYxType.m_yxss[m_group_sel].yxdatalen = 2;
	}else if(m_combo_datalen_val == 1){
		gYxType.m_yxss[m_group_sel].yxdatalen = 4;
	}else if(m_combo_datalen_val == 2){
		gYxType.m_yxss[m_group_sel].yxdatalen = 8;
	}
	gYxType.m_yxss[m_group_sel].yxslen = m_edit_datanum_val;
	
	int i = m_list_attr.GetItemText(1,2).GetLength();

	for(i = 0; i < m_edit_datanum_val; i++){
		int val;
		if(m_list_attr.GetItemText(i,1).GetLength()){
			val = atoi(m_list_attr.GetItemText(i,1));
		}else{
			val = 999999;
		}
		m_list_attr.SetItemData(i, val); 
	}
	m_list_attr.SortItems(listCtrlCompareProc,(DWORD)&m_list_attr);

	///////////////////////////////////////////////////
	gYxType.m_yxss[m_group_sel].method = vMethod.at(m_combo_method_val).no;
	///////////////////////////////////////////////////


	if(m_combo_horl_val == 0){
		gYxType.m_yxss[m_group_sel].horl = 'h';
	}else if(m_combo_horl_val == 1){
		gYxType.m_yxss[m_group_sel].horl = 'l';
	}else{
		gYxType.m_yxss[m_group_sel].horl = 'n';
	}
	for(i = 0; i < m_edit_datanum_val; i++){

		if(m_combo_horl_val == 0){
			gYxType.m_yxss[m_group_sel].yxs[i].horl = 'h';
		}else if(m_combo_horl_val == 1){
			gYxType.m_yxss[m_group_sel].yxs[i].horl = 'l';
		}else{
			gYxType.m_yxss[m_group_sel].yxs[i].horl = 'n';
		}

		strcpy(gYxType.m_yxss[m_group_sel].yxs[i].desc,m_list_attr.GetItemText(i,2));
		gYxType.m_yxss[m_group_sel].yxs[i].used = m_list_attr.GetCheck(i);
		if(m_list_attr.GetItemText(i,1).GetLength()){
			gYxType.m_yxss[m_group_sel].yxs[i].dit = atoi(m_list_attr.GetItemText(i,1));
		}else{
			gYxType.m_yxss[m_group_sel].yxs[i].dit = -1;
		}
	}
}

void CModbusConfigDialogDlg::saveDdConfig(){
	gDdType.m_ddss[m_group_sel].addr = atoi(m_combo_addr_val);
	gDdType.m_ddss[m_group_sel].startindex = m_edit_startindex_val;
	gDdType.m_ddss[m_group_sel].cbdatalenbit = m_edit_cbdatalenbit_val;
	gDdType.m_ddss[m_group_sel].cbdatahorl = m_combo_cbdatahorl_val ? 'l' : 'h';
	gDdType.m_ddss[m_group_sel].hasnan = m_check_hasnan_val ? 1 : 0;
	gDdType.m_ddss[m_group_sel].nankey = m_edit_nankey_val;
	gDdType.m_ddss[m_group_sel].nanvalue = m_edit_nanvalue_val;
	if(m_combo_datalen_val == 0){
		gDdType.m_ddss[m_group_sel].dddatalen = 2;
	}else if(m_combo_datalen_val == 1){
		gDdType.m_ddss[m_group_sel].dddatalen = 4;
	}else if(m_combo_datalen_val == 2){
		gDdType.m_ddss[m_group_sel].dddatalen = 8;
	}
	gDdType.m_ddss[m_group_sel].ddslen = m_edit_datanum_val;
	
	int len = gDdType.m_ddss[m_group_sel].ddslen;
	int i = m_list_attr.GetItemText(1,2).GetLength();

	for(i = 0; i < m_edit_datanum_val; i++){
		int val;
		if(m_list_attr.GetItemText(i,1).GetLength()){
			val = atoi(m_list_attr.GetItemText(i,1));
		}else{
			val = 999999;
		}
		m_list_attr.SetItemData(i, val); 
	}
	m_list_attr.SortItems(listCtrlCompareProc,(DWORD)&m_list_attr);


	if(m_combo_horl_val == 0){
		gDdType.m_ddss[m_group_sel].horl = 'h';
	}else if(m_combo_horl_val == 1){
		gDdType.m_ddss[m_group_sel].horl = 'l';
	}else{
		gDdType.m_ddss[m_group_sel].horl = 'n';
	}
	for(i = 0; i < m_edit_datanum_val; i++){

		if(m_combo_horl_val == 0){
			gDdType.m_ddss[m_group_sel].dds[i].horl = 'h';
		}else if(m_combo_horl_val == 1){
			gDdType.m_ddss[m_group_sel].dds[i].horl = 'l';
		}else{
			gDdType.m_ddss[m_group_sel].dds[i].horl = 'n';
		}
		
		///////////////////////////////////////////////
		gDdType.m_ddss[m_group_sel].dds[i].method = vMethod.at(m_combo_method_val).no;
		///////////////////////////////////////////////

		strcpy(gDdType.m_ddss[m_group_sel].dds[i].desc,m_list_attr.GetItemText(i,2));
		gDdType.m_ddss[m_group_sel].dds[i].coe = atof(m_list_attr.GetItemText(i,3));
		gDdType.m_ddss[m_group_sel].dds[i].used = m_list_attr.GetCheck(i);
		if(m_list_attr.GetItemText(i,1).GetLength()){
			gDdType.m_ddss[m_group_sel].dds[i].dit = atoi(m_list_attr.GetItemText(i,1));
		}else{
			gDdType.m_ddss[m_group_sel].dds[i].dit = -1;
		}
	}
}
/*
void CModbusConfigDialogDlg::OnKillfocusEditDatanum() 
{
	// TODO: Add your control notification handler code here
	int index = m_list_type.GetCurSel();
	if(index<0){
		MessageBox("请选择规约");
		return;
	}
	index = m_list_group.GetCurSel();
	if(index<0){
		MessageBox("请选择组");
		return;
	}
	int old = m_edit_datanum_val;
	UpdateData(TRUE);
	if(m_state == 0){
		for(int i = old; i <m_edit_datanum_val; i++){
			gYcType.m_ycss[m_group_sel].ycs[i].used = 1;
			gYcType.m_ycss[m_group_sel].ycs[i].coe = 1;
			gYcType.m_ycss[m_group_sel].ycs[i].dit = -1;
			strcpy(gYcType.m_ycss[m_group_sel].ycs[i].desc, "");
		}
	}else if(m_state == 1){
		for(int i = old; i <m_edit_datanum_val; i++){
			gYxType.m_yxss[m_group_sel].yxs[i].used = 1;
			gYxType.m_yxss[m_group_sel].yxs[i].dit = -1;
			strcpy(gYxType.m_yxss[m_group_sel].yxs[i].desc, "");
		}
	}else if(m_state == 2){
		for(int i = old; i <m_edit_datanum_val; i++){
			gDdType.m_ddss[m_group_sel].dds[i].used = 1;
			gDdType.m_ddss[m_group_sel].dds[i].coe = 1;
			gDdType.m_ddss[m_group_sel].dds[i].dit = -1;
			strcpy(gDdType.m_ddss[m_group_sel].dds[i].desc, "");
		}
	}
	initListCtrl(m_edit_datanum_val);
	updateListCtrlData();
}
*/

void CModbusConfigDialogDlg::creatYcTable(){
	char* cErrMsg;
	std::string sqlStr = "CREATE TABLE yc(bigid CHAR(50), id CHAR(50),desc CHAR(50),addr INT, coe FLOAT, relation INT,length INT,";
		sqlStr += "horl CHAR(1), method INT, used INT, sort INTEGER PRIMARY KEY  AUTOINCREMENT, startindex INT, cbdatalenbit INT,";
		sqlStr += "cbdatahorl CHAR(1), hasnan INT, nankey INT, nanvalue INT);";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("生成数据库yc表失败！"); 
	}
}

void CModbusConfigDialogDlg::insertDataYcToDB(){
	int nRes;
	
	char* cErrMsg;
	int index = m_list_type.GetCurSel();
	if(index<0){
		MessageBox("请选择规约");
		return;
	}
	
	for(int i = 0; i < gYcType.ycseclen; i++){
		int ycslen = gYcType.m_ycss[i].ycslen;
		if(!ycslen){
			CString str;
			str.Format("遥测数据 %s : 数据数为0，数据未保存！",gYcType.m_ycss[i].desc);
			MessageBox(str.GetBuffer(0));
		}
		for(int j = 0; j < ycslen; j++){
			CString str;
			if(j==0){
				str.Format("INSERT INTO yc VALUES ('%s','%s','%s',%d,%f,null,%d,'%c',%d,%d,null,%d,%d,'%c',%d,%d,%d)",
					vbig.at(m_type_sel).id, gYcType.m_ycss[i].desc, gYcType.m_ycss[i].ycs[j].desc, gYcType.m_ycss[i].addr,
					gYcType.m_ycss[i].ycs[j].coe,  gYcType.m_ycss[i].ycdatalen,  gYcType.m_ycss[i].horl, 
					gYcType.m_ycss[i].ycs[j].method, gYcType.m_ycss[i].ycs[j].used, gYcType.m_ycss[i].ycs[j].dit,
					gYcType.m_ycss[i].cbdatalenbit, gYcType.m_ycss[i].cbdatahorl,gYcType.m_ycss[i].hasnan,
					gYcType.m_ycss[i].nankey, gYcType.m_ycss[i].nanvalue);
			}else{
				str.Format("INSERT INTO yc VALUES ('%s','%s','%s',null,%f,null,%d,'%c',%d,%d,null,%d,%d,'%c',%d,%d,%d)",
					vbig.at(m_type_sel).id, gYcType.m_ycss[i].ycs[j].id, gYcType.m_ycss[i].ycs[j].desc, gYcType.m_ycss[i].ycs[j].coe,
					gYcType.m_ycss[i].ycdatalen, gYcType.m_ycss[i].ycs[j].horl,  gYcType.m_ycss[i].ycs[j].method,
					gYcType.m_ycss[i].ycs[j].used, gYcType.m_ycss[i].ycs[j].dit, gYcType.m_ycss[i].cbdatalenbit,
					gYcType.m_ycss[i].cbdatahorl,gYcType.m_ycss[i].hasnan,
					gYcType.m_ycss[i].nankey, gYcType.m_ycss[i].nanvalue);
			}
			char pstr[512];
			strcpy(pstr,str);
			nRes = sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
			if (nRes != SQLITE_OK)
			{
				MessageBox("添加yc表数据失败！"); 
				MessageBox(str); 
				cout<<"query fail: "<<cErrMsg<<endl;
			}
		}
	}
}

void CModbusConfigDialogDlg::creatYxTable(){
	char* cErrMsg;
	std::string sqlStr = "CREATE TABLE yx(bigid CHAR(50), id CHAR(50),desc CHAR(50),addr INT, horl CHAR(1), ";
		sqlStr += "method INT, used INT, sort INTEGER PRIMARY KEY  AUTOINCREMENT, startindex INT, offset INT, yxnum INT,";
		sqlStr += "cbdatalenbit INT,cbdatahorl CHAR(1));";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("生成数据库yx表失败！"); 
		cout<<"query fail: "<<cErrMsg<<endl;
	}
}

void CModbusConfigDialogDlg::insertDataYxToDB(){
	int nRes;
	
	char* cErrMsg;
	int index = m_list_type.GetCurSel();
	if(index<0){
		MessageBox("请选择规约");
		return;
	}
	for(int i = 0; i < gYxType.yxseclen; i++){
		int yxslen = gYxType.m_yxss[i].yxslen;
		if(!yxslen){
			CString str;
			str.Format("遥信数据 %s : 数据数为0，数据未保存！",gYxType.m_yxss[i].desc);
			MessageBox(str.GetBuffer(0));
		}
		for(int j = 0; j < yxslen; j++){
			CString str;
			if(j==0){
				str.Format(_T("INSERT INTO yx VALUES ('%s','%s','%s',%d,'%c',%d,%d,null,%d,%d,%d,%d,'%c')"),
					vbig.at(m_type_sel).id, gYxType.m_yxss[i].desc, gYxType.m_yxss[i].yxs[j].desc, gYxType.m_yxss[i].addr,
					gYxType.m_yxss[i].yxs[j].horl, gYxType.m_yxss[i].method, 
					gYxType.m_yxss[i].yxs[j].used, gYxType.m_yxss[i].yxs[j].dit,
					gYxType.m_yxss[i].offset,gYxType.m_yxss[i].yxnum, gYxType.m_yxss[i].cbdatalenbit,
					gYxType.m_yxss[i].cbdatahorl);
			}else{
				str.Format(_T("INSERT INTO yx VALUES ('%s','%s','%s',null,'%c',%d,%d,null,%d,null,null,%d,'%c')"),
					vbig.at(m_type_sel).id, gYxType.m_yxss[i].yxs[j].id, gYxType.m_yxss[i].yxs[j].desc, 
					gYxType.m_yxss[i].yxs[j].horl, gYxType.m_yxss[i].method, 
					gYxType.m_yxss[i].yxs[j].used, gYxType.m_yxss[i].yxs[j].dit, gYxType.m_yxss[i].cbdatalenbit,
					gYxType.m_yxss[i].cbdatahorl);
			}
			char pstr[512];
			strcpy(pstr,str);
			nRes = sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
			if (nRes != SQLITE_OK)
			{
				MessageBox("添加yx表数据失败！"); 
				MessageBox(str); 
				cout<<"query fail: "<<cErrMsg<<endl;
			}
		}
	}
}

void CModbusConfigDialogDlg::creatDdTable(){
	char* cErrMsg;
	std::string sqlStr = "CREATE TABLE dd(bigid CHAR(50), id CHAR(50),desc CHAR(50),addr INT, coe FLOAT, relation INT,length INT,";
		sqlStr += "horl CHAR(1), method INT, used INT, sort INTEGER PRIMARY KEY AUTOINCREMENT, startindex INT, reqnum INT,";
		sqlStr += "cbdatalenbit INT, cbdatahorl CHAR(1), hasnan INT, nankey INT, nanvalue INT);";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("生成数据库dd表失败！"); 
		cout<<"query fail: "<<cErrMsg<<endl;
	}
}

void CModbusConfigDialogDlg::insertDataDdToDB(){
	int nRes;
	
	char* cErrMsg;
	int index = m_list_type.GetCurSel();

	for(int i = 0; i < gDdType.ddseclen; i++){
		int ddslen = gDdType.m_ddss[i].ddslen;
		if(!ddslen){
			CString str;
			str.Format("电度数据 %s : 数据数为0，数据未保存！",gDdType.m_ddss[i].desc);
			MessageBox(str.GetBuffer(0));
		}
		for(int j = 0; j < ddslen; j++){
			CString str;
			if(j==0){
				str.Format("INSERT INTO dd VALUES ('%s','%s','%s',%d,%f,null,%d,'%c',%d,%d,null,%d,%d,%d,'%c',%d,%d,%d)",
					vbig.at(m_type_sel).id, gDdType.m_ddss[i].desc, gDdType.m_ddss[i].dds[j].desc, gDdType.m_ddss[i].addr, 
					gDdType.m_ddss[i].dds[j].coe, gDdType.m_ddss[i].dddatalen,  gDdType.m_ddss[i].dds[j].horl,
					gDdType.m_ddss[i].dds[j].method, gDdType.m_ddss[i].dds[j].used, gDdType.m_ddss[i].dds[j].dit,
					gDdType.m_ddss[i].reqnum, gDdType.m_ddss[i].cbdatalenbit, gDdType.m_ddss[i].cbdatahorl,
					gDdType.m_ddss[i].hasnan, gDdType.m_ddss[i].nankey, gDdType.m_ddss[i].nanvalue);
			}else{
				str.Format("INSERT INTO dd VALUES ('%s','%s','%s',null,%f,null,%d,'%c',%d,%d,null,%d,null,%d,'%c',%d,%d,%d)",
					vbig.at(m_type_sel).id, gDdType.m_ddss[i].dds[j].id, gDdType.m_ddss[i].dds[j].desc, gDdType.m_ddss[i].dds[j].coe,
					gDdType.m_ddss[i].dddatalen, gDdType.m_ddss[i].dds[j].horl,  gDdType.m_ddss[i].dds[j].method, 
					gDdType.m_ddss[i].dds[j].used, gDdType.m_ddss[i].dds[j].dit, gDdType.m_ddss[i].cbdatalenbit,
					gDdType.m_ddss[i].cbdatahorl, gDdType.m_ddss[i].hasnan, gDdType.m_ddss[i].nankey, gDdType.m_ddss[i].nanvalue);
			}
			char pstr[512];
			strcpy(pstr,str);
			nRes = sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
			if (nRes != SQLITE_OK)
			{
				MessageBox("添加dd表数据失败！"); 
				MessageBox(str); 
				cout<<"query fail: "<<cErrMsg<<endl;
			}
		}
	}
}

void CModbusConfigDialogDlg::creatYkTable(){
	char* cErrMsg;
	std::string sqlStr = "CREATE TABLE yk(bigid CHAR(50), id CHAR(50),desc CHAR(50),horl CHAR(1),addr INT, ";
		sqlStr += "index1 INT, used INT, sort INTEGER PRIMARY KEY AUTOINCREMENT);";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("生成数据库yk表失败！"); 
		cout<<"query fail: "<<cErrMsg<<endl;
	}
}

void CModbusConfigDialogDlg::insertDataYkToDB(){
	
}

void CModbusConfigDialogDlg::creatYtTable(){
	char* cErrMsg;
	std::string sqlStr = "CREATE TABLE yt(bigid CHAR(50), id CHAR(50),desc CHAR(50),horl CHAR(1),addr INT, length INT,";
		sqlStr += " used INT, index1 INT,sort INTEGER PRIMARY KEY AUTOINCREMENT);";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("生成数据库yt表失败！"); 
		cout<<"query fail: "<<cErrMsg<<endl;
	}
}

void CModbusConfigDialogDlg::insertDataYtToDB(){
	
}
/*
void CModbusConfigDialogDlg::creatCommonTable(){
	char* cErrMsg;
	std::string sqlStr = "CREATE TABLE common(rtuInterval INT, reqInterval INT,registerhorl CHAR(1),addrhorl CHAR(1),gcbcheckcrc INT,gcbdatalenbit INT, cbdatalenhorl CHAR(1));";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("生成数据库common表失败！"); 
		cout<<"query fail: "<<cErrMsg<<endl;
	}
}

void CModbusConfigDialogDlg::insertDataCommonToDB(){

	CString str;
	char* cErrMsg;
	str.Format("INSERT INTO common VALUES (%d,%d,'%c','%c',%d,%d,'%c')",commond.reqInterval ? commond.reqInterval : 3,commond.reqInterval ? commond.reqInterval : 3,
		'h',commond.checkhorl ? commond.checkhorl: 'l',commond.cbcheckcrc ? commond.cbcheckcrc : 1,2,'l');
	char pstr[256];
	strcpy(pstr,str);
	sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
}
*/

void CModbusConfigDialogDlg::dropAllTable(){
	char* cErrMsg;
	std::string sqlStr = "DROP TABLE common;";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		//MessageBox("删除数据库表失败！");
		//printf("查询数据库common表失败，%s",cErrMsg);
		cout<<"query fail: "<<cErrMsg<<endl;
	}
	sqlStr = "DROP TABLE yc;";
	sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	sqlStr = "DROP TABLE yx;";
	sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	sqlStr = "DROP TABLE dd;";
	sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	sqlStr = "DROP TABLE yt;";
	sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	sqlStr = "DROP TABLE yk;";
	sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	sqlStr = "DROP TABLE big;";
	sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
}

void CModbusConfigDialogDlg::creatBigTable(){
	char* cErrMsg;
	std::string sqlStr = "CREATE TABLE big(id CHAR(50), desc CHAR(50), sort INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sqlStr += " addrhorl CHAR(1),gcheckcrc INT,version INT);";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("生成数据库big表失败！"); 
		cout<<"query fail: "<<cErrMsg<<endl;
	}
}
void CModbusConfigDialogDlg::insertDataBigToDB(bigstruct bigstr){

	CString str;
	str.Format("INSERT INTO big VALUES ('%s','%s',null,'%c',%d,%d)",bigstr.id,bigstr.desc,
		 bigstr.addrhorl, bigstr.checkcrc, bigstr.version);
	char pstr[256];
	strcpy(pstr,str);
	char* cErrMsg;
	int nRes = sqlite3_exec(pDB , pstr ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("新增big数据失败！"); 
		cout<<"query fail: "<<cErrMsg<<endl;
	}
}

void CModbusConfigDialogDlg::checkIsChanged(){
	if(gIsChanged && m_type_sel != -1){
		int key = MessageBox("设置已改变，是否要保存？","警告",MB_YESNO);
		if(key == IDYES){
			OnButtonSaveconfigtodb();
		}else{
			gIsChanged = false;
		}
	}
	return;
}

void CModbusConfigDialogDlg::OnEditchangeComboAddr() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	saveConfig();
}

void CModbusConfigDialogDlg::OnSelchangeComboAddr() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	saveConfig();
}

void CModbusConfigDialogDlg::OnSelchangeComboAddrhorl() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	UpdateData(TRUE);
	saveBigConfig();
}

void CModbusConfigDialogDlg::OnSelchangeComboHorl() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	saveConfig();
}

void CModbusConfigDialogDlg::OnSelchangeComboDatalen() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	saveConfig();
}

void CModbusConfigDialogDlg::OnChangeEditReqnum() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	saveConfig();
}

void CModbusConfigDialogDlg::OnCheckHasnan() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	saveConfig();
}

void CModbusConfigDialogDlg::OnChangeEditNankey() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	CString tem;
	m_edit_nankey.GetWindowText(tem);
	if(tem != "-" && tem != ""){
		saveConfig();
	}
}

void CModbusConfigDialogDlg::OnChangeEditNanvalue() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	CString tem;
	m_edit_nanvalue.GetWindowText(tem);
	if(tem != "-" && tem != ""){
		saveConfig();
	}
}

/*
void CModbusConfigDialogDlg::OnChangeEditDatanum() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	gIsChanged = true;
}
*/
void CModbusConfigDialogDlg::OnSelchangeComboCrc() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	UpdateData(TRUE);
	saveBigConfig();
}
/*
void CModbusConfigDialogDlg::OnEditchangeComboInterval() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	saveConfig();
}
*/
/*
void CModbusConfigDialogDlg::OnSelchangeComboInterval() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	saveConfig();
}
*/
void CModbusConfigDialogDlg::OnSelchangeComboMethod() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	saveConfig();
}
/*
void CModbusConfigDialogDlg::OnChangeEditStartindex() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	saveConfig();
}
*/
/*
int C2RtuResult(void *NotUsed, int argc, char **argv, char **azColName)
{
	RTUSTRUCT rtu;
	rtu.rtuno = atoi(argv[0]);
	strcpy(rtu.desc, argv[1]);
	vRtu.push_back(rtu);
    return 0;
}

void CModbusConfigDialogDlg::getC2RtuInfo(){
	vRtu.clear();
	char dbPath[128];

	char* envvar = getenv("C2PLAT");
	if( envvar == NULL ){
		MessageBox("获取环境变量C2PLAT失败！");
		return;
	}
	sprintf(dbPath, "%s/db/jskpara.db", envvar);

	int nRes = sqlite3_open(dbPath, &pDB);

	char* cErrMsg;
	std::string sqlStr = "select 装置序号,装置名称 from T004_装置表;";
	nRes = sqlite3_exec(pDB , sqlStr.c_str() ,C2RtuResult ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("查询jskpara.db,获取C2 RTU信息失败！");
	}
	sqlite3_close(pDB);
}
*/
int CModbusConfigDialogDlg::openDB(){
	char *path = gDBpath;
	int nRes = sqlite3_open(path, &pDB);
	if (nRes != SQLITE_OK)
	{
	  MessageBox("打开数据库失败");
	  ::MessageBox(NULL,"打开数据库失败",path,MB_OK);
	  return -1;
	}
	return 0;
}

int CModbusConfigDialogDlg::closeDB(){
	int nRes = sqlite3_close(pDB);
	if (nRes != SQLITE_OK)
	{
	  MessageBox("关闭数据库失败");
	  return -1;
	}
	return 0;
}

int CModbusConfigDialogDlg::sqlExec(char *sql){
	char* cErrMsg;
	int nRes = sqlite3_exec(pDB , sql ,0 ,0, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		::MessageBox(NULL,"执行语句失败！",sql,MB_OK);
		MessageBox(cErrMsg);
		return -1;
	}
	
	return 0;
}

int ycResult(void *NotUsed, int argc, char **argv, char **azColName)
{
	if(argc != 17){
		MessageBox(NULL, "yc表列数不符","",MB_OK);
		return -1;
	}
	int bigid = atoi(argv[0]);
	int ycssindex = gYcType.ycseclen-1;
	
	if(argv[3] && *argv[3]){
		gYcType.ycseclen++;
		ycssindex = gYcType.ycseclen-1;
		gYcType.m_ycss[gYcType.ycseclen-1].ycslen = 0;
		gYcType.m_ycss[gYcType.ycseclen-1].addr = atoi(argv[3]);
		gYcType.m_ycss[gYcType.ycseclen-1].startindex = argv[11] ? atoi(argv[11]) : 0;
		gYcType.m_ycss[gYcType.ycseclen-1].ycdatalen = atoi(argv[6]);
		gYcType.m_ycss[gYcType.ycseclen-1].cbdatalenbit = argv[12] ? atoi(argv[12]) : 0;

		gYcType.m_ycss[gYcType.ycseclen-1].cbdatahorl = argv[13] ? *argv[13] : 'h';
		gYcType.m_ycss[gYcType.ycseclen-1].hasnan = argv[14] ? atoi(argv[14]) : 0;
		gYcType.m_ycss[gYcType.ycseclen-1].nankey = argv[15] ? atoi(argv[15]) : 0;
		gYcType.m_ycss[gYcType.ycseclen-1].nanvalue = argv[16] ? atoi(argv[16]) : 0;
		strcpy(gYcType.m_ycss[ycssindex].desc, argv[1]);
	}
	
	gYcType.m_ycss[gYcType.ycseclen-1].ycslen++;
	int ycsindex = gYcType.m_ycss[gYcType.ycseclen-1].ycslen-1;

	strcpy(gYcType.m_ycss[ycssindex].ycs[ycsindex].desc, argv[2]);
	gYcType.m_ycss[ycssindex].ycs[ycsindex].coe = argv[4] ? atof(argv[4]) : 1.0;
	gYcType.m_ycss[ycssindex].ycs[ycsindex].rel = argv[5] ? atoi(argv[5]) : -1;
	gYcType.m_ycss[ycssindex].ycs[ycsindex].length = atoi(argv[6]);
	gYcType.m_ycss[ycssindex].ycs[ycsindex].horl = *argv[7];
	gYcType.m_ycss[ycssindex].ycs[ycsindex].method = atoi(argv[8]);
	gYcType.m_ycss[ycssindex].ycs[ycsindex].used = atoi(argv[9]);
	gYcType.m_ycss[ycssindex].ycs[ycsindex].sort = atoi(argv[10]);
	gYcType.m_ycss[ycssindex].ycs[ycsindex].dit = argv[11] ? atoi(argv[11]) : -1;

    return 0;
}

int CModbusConfigDialogDlg::ycQuery(char* bigid){
	char* cErrMsg;
	string id = bigid;
	gYcType.ycseclen = 0;
	string sqlStr = "select * from yc where bigid = '" + id +"'";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,ycResult ,this, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("查询数据库yc表失败");
		return -1;
	}
	return 0;
}

int yxResult(void *NotUsed, int argc, char **argv, char **azColName)
{

	if(argc != 13){
		//MessageBox("yx表列数不符");
		MessageBox(NULL, "yx表列数不符","",MB_OK);
		return -1;
	}
	CModbusConfigDialogDlg* t = (CModbusConfigDialogDlg*)NotUsed;

	int bigid = atoi(argv[0]);
	int yxssindex = gYxType.yxseclen-1;
	
	if(argv[3] && *argv[3]){
		gYxType.yxseclen++;
		yxssindex = gYxType.yxseclen-1;
		gYxType.m_yxss[gYxType.yxseclen-1].yxslen = 0;

		gYxType.m_yxss[yxssindex].addr = atoi(argv[3]);
		gYxType.m_yxss[yxssindex].startindex = argv[8] ? atoi(argv[8]) : 0;
		gYxType.m_yxss[yxssindex].yxnum = argv[10] ? atoi(argv[10]) : 0;
		gYxType.m_yxss[yxssindex].offset = argv[9] ? atoi(argv[9]) : 0;
		gYxType.m_yxss[yxssindex].method = atoi(argv[5]);
		gYxType.m_yxss[yxssindex].cbdatalenbit = argv[11] ? atoi(argv[11]) : 1;
		gYxType.m_yxss[yxssindex].horl = *argv[4];

		gYxType.m_yxss[yxssindex].cbdatahorl = argv[12] ? *argv[12] : 'h';
		strcpy(gYxType.m_yxss[yxssindex].desc, argv[1]);
	}
	
	gYxType.m_yxss[gYxType.yxseclen-1].yxslen++;
	int yxsindex = gYxType.m_yxss[gYxType.yxseclen-1].yxslen-1;
	
	strcpy(gYxType.m_yxss[yxssindex].yxs[yxsindex].desc, argv[2]);
	gYxType.m_yxss[yxssindex].yxs[yxsindex].horl = *argv[4];
	gYxType.m_yxss[yxssindex].yxs[yxsindex].used = argv[6] ? atoi(argv[6]) : 0;
	gYxType.m_yxss[yxssindex].yxs[yxsindex].dit = argv[8] ? atoi(argv[8]) : -1;

    return 0;
}

int CModbusConfigDialogDlg::yxQuery(char* bigid){
	char* cErrMsg;
	string id = bigid;
	gYxType.yxseclen = 0;
	string sqlStr = "select * from yx where bigid = '" + id +"'";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,yxResult ,this, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("查询数据库yx表失败");
		return -1;
	}
	return 0;
}

int ddResult(void *NotUsed, int argc, char **argv, char **azColName)
{
	if(argc != 18){
		//MessageBox("dd表列数不符");
		MessageBox(NULL, "dd表列数不符","",MB_OK);
		return -1;
	}
	CModbusConfigDialogDlg* t = (CModbusConfigDialogDlg*)NotUsed;

	int bigid = atoi(argv[0]);
	int ddssindex = gDdType.ddseclen-1;
	
	if(argv[3] && *argv[3]){
		gDdType.ddseclen++;
		ddssindex = gDdType.ddseclen-1;
		gDdType.m_ddss[ddssindex].ddslen = 0;
		
		gDdType.m_ddss[ddssindex].addr = atoi(argv[3]);
		gDdType.m_ddss[ddssindex].startindex = argv[11] ? atoi(argv[11]) : 0;
		gDdType.m_ddss[ddssindex].reqnum = argv[12] ? atoi(argv[12]) : 0;
		gDdType.m_ddss[ddssindex].dddatalen = atoi(argv[6]);
		gDdType.m_ddss[ddssindex].cbdatalenbit = argv[13] ? atoi(argv[13]) : 1;


		gDdType.m_ddss[ddssindex].cbdatahorl = argv[14] ? *argv[14] : 'h';

		gDdType.m_ddss[ddssindex].hasnan = argv[15] ? atoi(argv[15]) : 1;
		gDdType.m_ddss[ddssindex].nankey = argv[16] ? atoi(argv[16]) : 1;
		gDdType.m_ddss[ddssindex].nanvalue = argv[17] ? atoi(argv[17]) : 1;
		strcpy(gDdType.m_ddss[ddssindex].desc, argv[1]);
	}
		
	gDdType.m_ddss[ddssindex].ddslen++;
	int ddsindex = gDdType.m_ddss[ddssindex].ddslen-1;

	strcpy(gDdType.m_ddss[ddssindex].dds[ddsindex].desc, argv[2]);
	gDdType.m_ddss[ddssindex].dds[ddsindex].coe = argv[4] ? atof(argv[4]) : 1.0;
	gDdType.m_ddss[ddssindex].dds[ddsindex].rel = argv[5] ? atoi(argv[5]) : -1;
	gDdType.m_ddss[ddssindex].dds[ddsindex].length = atoi(argv[6]);
	gDdType.m_ddss[ddssindex].dds[ddsindex].horl = *argv[7];
	gDdType.m_ddss[ddssindex].dds[ddsindex].method = atoi(argv[8]);
	gDdType.m_ddss[ddssindex].dds[ddsindex].used = atoi(argv[9]);
	gDdType.m_ddss[ddssindex].dds[ddsindex].sort = atoi(argv[10]);
	gDdType.m_ddss[ddssindex].dds[ddsindex].dit = argv[11] ? atoi(argv[11]) : -1;

    return 0;
}


int CModbusConfigDialogDlg::ddQuery(char* bigid){
	char* cErrMsg;
	string id = bigid;
	gDdType.ddseclen = 0;
	string sqlStr = "select * from dd where bigid = '" + id +"'";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,ddResult ,this, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("查询数据库dd表失败");
		return -1;
	}
	return 0;
}

/*
int commonResult(void *NotUsed, int argc, char **argv, char **azColName)
{
	if(argc != 6){
		//MessageBox("common表列数不符");
		MessageBox(NULL, "common表列数不符","",MB_OK);
		return -1;
	}
	CModbusConfigDialogDlg* t = (CModbusConfigDialogDlg*)NotUsed;

	commond.reqInterval = atoi(argv[1]);
	commond.cbcheckcrc = atoi(argv[4]);

    return 0;
}


int CModbusConfigDialogDlg::commonQuery(){
	char* cErrMsg;
	string sqlStr = "select * from common";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,commonResult ,this, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("查询数据库common表失败");
		return -1;
	}
	 
	return 0;
}
*/

int bigResult(void *NotUsed, int argc, char **argv, char **azColName)
{
	if(argc != 6){
		//MessageBox("big表列数不符");
		MessageBox(NULL, "big表列数不符","",MB_OK);
		return -1;
	}
	CModbusConfigDialogDlg* t = (CModbusConfigDialogDlg*)NotUsed;

	//t->m_yctypeNum++;
	//t->m_yxtypeNum++;
	//t->m_ddtypeNum++;
	//t->m_typeNum++;
	
	bigstruct bigstr;
	strcpy(bigstr.desc,argv[1]);
	strcpy(bigstr.id,argv[0]);
	bigstr.checkcrc = atoi(argv[4]);
	bigstr.addrhorl = *argv[3];
	bigstr.version = argv[5] ? atoi(argv[5]) : 0;
	vbig.push_back(bigstr);

    return 0;
}

int CModbusConfigDialogDlg::bigQuery(){

	//m_typeNum=0;

	//m_yctypeNum=0;
	//m_yxtypeNum=0;
	//m_ddtypeNum=0;

	char* cErrMsg;
	string sqlStr = "select * from big order by desc";

	vbig.clear();
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,bigResult ,this, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("查询数据库big表失败");
		return -1;
	}

	return 0;
}


int ykResult(void *NotUsed, int argc, char **argv, char **azColName)
{
	CModbusConfigDialogDlg* t = (CModbusConfigDialogDlg*)NotUsed;

    return 0;
}

int CModbusConfigDialogDlg::ykQuery(){
	char* cErrMsg;
	string sqlStr = "select * from yk";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,ykResult ,this, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("查询数据库yk表失败");
		return -1;
	}
	return 0;
}

int ytResult(void *NotUsed, int argc, char **argv, char **azColName)
{
	CModbusConfigDialogDlg* t = (CModbusConfigDialogDlg*)NotUsed;

    return 0;
}

int CModbusConfigDialogDlg::ytQuery(){
	char* cErrMsg;
	string sqlStr = "select * from yt";
	int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,ytResult ,this, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		MessageBox("查询数据库yt表失败");
		return -1;
	}
	return 0;
}

void CModbusConfigDialogDlg::OnButtonOpen() 
{
	// TODO: Add your control notification handler code here
/*
	CString dbPath;
	UpdateData(TRUE);
	char szFilter[]="DB files(*.db)|*.db";
	CFileDialog dlg(1,NULL,NULL,NULL,szFilter);
	if(dlg.DoModal()==IDOK){
		dbPath = dlg.GetPathName();
		UpdateData(FALSE);
		strcpy(gDBpath, dbPath.GetBuffer(0));
		int ret = openDB();
		if(ret == -1){
			MessageBox("请新建或打开数据库");
			return;
		}
		
		getDataFromDB();

		closeDB();
	}
*/	
	//if(m_c5db.openDB()){
		getCompanyFromC5DB();
		C5DBbigQuery(m_c5db);
		refreshTypeUI();
		//m_c5db.closeDB();
	//}else{
	//	MessageBox("打开MSSQL数据库时出错");
	//}
	
}

int CModbusConfigDialogDlg::getDataFromDB(){
	bigQuery();

	refreshTypeUI();

	return 0;
}

void CModbusConfigDialogDlg::refreshTypeUI(){
	m_type_sel = -1;
	m_list_type.ResetContent();

	int len = vbig.size();
	if(len){
		for(int i = 0; i < len; i++){
			CString str;
			CString str1 = vbig.at(i).id;
			CString str2 = vbig.at(i).desc;
			//str1.TrimRight();
			//str2.TrimRight();
			//str = str1.TrimRight() + ": " + str2.TrimRight();
			str.Format("%s: %s", str1, str2);
			//str = str + "、";
			//str = str + vbig.at(i).desc;
			m_list_type.AddString(str.GetBuffer(0));
		}
	}

	m_combo_company.ResetContent();
	len = vCompany.size();
	for(int i = 0; i < len; i++){
		m_combo_company.AddString(vCompany.at(i).desc.GetBuffer(0));
	}
}

void CModbusConfigDialogDlg::OnDblclkListGroup() 
{
	// TODO: Add your control notification handler code here
	int key = MessageBox("是否要删除所选项？","警告",MB_YESNO);
	if(key == IDYES){
		deleteGroup(m_group_sel);
	}
}
/*
void CModbusConfigDialogDlg::OnDblclkListType() 
{
	// TODO: Add your control notification handler code here
	checkIsChanged();
	int key = MessageBox("是否要删除所选项？","警告",MB_YESNO);
	if(key == IDYES){
		deleteType(m_type_sel);
	}
}
*/

void CModbusConfigDialogDlg::OnButtonChangetypedesc() 
{
	// TODO: Add your control notification handler code here
	if(m_list_type.GetCurSel() < 0){
		MessageBox("请选择规约！"); 
		return;
	}
	CString tem;  
	m_edit_typedesc.GetWindowText(tem);
	//char newid[CHARLEN];
	char newdesc[CHARLEN];

	strcpy(newdesc, tem.GetBuffer(0));

	//m_edit_bigid.GetWindowText(tem);
	//if(checkBigid(atoi(tem.GetBuffer(0)), m_type_sel)){
	//	strcpy(newid, tem.GetBuffer(0));
	//}
	/*
	int ret = openDB();
	if(ret == -1){
		MessageBox("请新建或打开数据库");
		return;
	}
	updateBigidInDB(vbig.at(m_type_sel).id, newid);
	updateBigDescInDB(newid, newdesc);
	bigQuery();
	closeDB();
*/
	//if(m_c5db.openDB()){
		//updateBigidInC5DB(vbig.at(m_type_sel).id, newid, m_c5db);
		updateBigDescInC5DB(vbig.at(m_type_sel).id, newdesc, m_c5db);
		C5DBbigQuery(m_c5db);

	//	m_c5db.closeDB();
	//}
	
	refreshTypeUI();
}

bool CModbusConfigDialogDlg::checkBigid(int id, int index){
	int l = vbig.size();
	for(int i = 0; i < l; i++){
		if(i != index){
			if(atoi(vbig.at(i).id) == id){
				MessageBox("规约号重复, 重命名失败");
				return false;
			}
		}
	}
	return true;
}

void CModbusConfigDialogDlg::OnButtonChangegroupdesc()
{
	// TODO: Add your control notification handler code here
	if(m_list_type.GetCurSel() < 0){
		MessageBox("请选择规约！"); 
		return;
	}
	if(m_list_group.GetCurSel() < 0){
		MessageBox("请选择组！"); 
		return;
	}
	CString tem;  
	m_edit_groupdesc.GetWindowText(tem);
	strcpy(gYcType.m_ycss[m_group_sel].desc, tem.GetBuffer(0));
	strcpy(gYxType.m_yxss[m_group_sel].desc, tem.GetBuffer(0));
	strcpy(gDdType.m_ddss[m_group_sel].desc, tem.GetBuffer(0));
	refreshGroupListBox();
}

void CModbusConfigDialogDlg::OnButtonNewdb() 
{
	// TODO: Add your control notification handler code here
/*
	CString dbPath;
	char szFilter[]="DB files(*.db)|*.db";
	CFileDialog dlg(0,"db",NULL,NULL,szFilter);
	if(dlg.DoModal()==IDOK){
		dbPath = dlg.GetPathName();
		UpdateData(FALSE);
		strcpy(gDBpath, dbPath.GetBuffer(0));
		int ret = openDB();
		if(ret == -1){
			MessageBox("新建数据库失败");
			return;
		}
		dropAllTable();
		creatBigTable();
		creatYcTable();
		creatYxTable();
		creatDdTable();
		creatYkTable();
		creatYtTable();

		//bigQuery();
		closeDB();

		if(m_c5db.openDB()){
			creatC5BigTable(m_c5db);
			creatC5YcTable(m_c5db);
			creatC5YxTable(m_c5db);
			creatC5DdTable(m_c5db);
			creatC5YcGroupTable(m_c5db);
			creatC5YxGroupTable(m_c5db);
			creatC5DdGroupTable(m_c5db);
			m_c5db.closeDB();
		}

		refreshTypeUI();
		initTypes();
	}
*/
	/*
	//m_c5db.createDB();
	if(m_c5db.openDB()){
		creatC5BigTable(m_c5db);
		creatC5YcTable(m_c5db);
		creatC5YxTable(m_c5db);
		creatC5DdTable(m_c5db);
		creatC5YcGroupTable(m_c5db);
		creatC5YxGroupTable(m_c5db);
		creatC5DdGroupTable(m_c5db);
		m_c5db.closeDB();
	}else{
		MessageBox("连接数据库失败");
	}

	refreshTypeUI();
	initTypes();
	*/
}
/*
void CModbusConfigDialogDlg::clearBigInC5DB(C5DB c5db){
	//if(c5db.openDB()){
		//CString str = "truncate table big;truncate table ycgroup;truncate table yc;truncate table ddgroup;truncate table dd;truncate table yxgroup;truncate table yx";
	c5db.executeSQL(str);
		//c5db.closeDB();
	//}
}
*/
void CModbusConfigDialogDlg::OnButtonSaveconfigtodb() 
{
	// TODO: Add your control notification handler code here
	int index = m_list_type.GetCurSel();
	if(index<0){
		MessageBox("请选择规约");
		return;
	}
	saveConfig();
	//checkDit();
	autoAddDit();
	bigstruct bigstr = vbig.at(m_type_sel);
	if(gIsChanged){
		bigstr.version++;
	}
/*
	int ret = openDB();
	if(ret == -1){
		MessageBox("请新建或打开数据库");
		return;
	}
	
	deleteTypeInDB(bigstr.id);
	//如果有改变，则版本加1
	
	insertDataBigToDB(bigstr);
	insertDataYcToDB();
	insertDataYxToDB();
	insertDataDdToDB();

	ret = closeDB();
*/
	m_errorNum = 0;
	m_c5db.beginTrans();
	deleteTypeInC5DB(bigstr.id, m_c5db);
	insertDataBigToC5DB(bigstr, m_c5db);
	insertDataYcToC5DB(m_c5db);
	insertDataYxToC5DB(m_c5db);
	insertDataDdToC5DB(m_c5db);
	if(m_errorNum == 0){
		m_c5db.commitTrans();
		MessageBox("保存完成");
	}else{
		m_c5db.rollbackTrans();
		MessageBox("！保存失败 ！");
	}

	gIsChanged = false;
	/*
	if(ret == -1){
		MessageBox("关闭数据库时出错");
		return;
	}else{
		MessageBox("保存完成");
		return;
	}
	*/
}

bool CModbusConfigDialogDlg::checkPointNumLimit(int num){
	if(m_state == 0){
		return num > YCNUMLIMITE;
	}else if(m_state == 1){
		return num > YXNUMLIMITE;
	}else if(m_state == 2){
		return num > DDNUMLIMITE;
	}
	return false;
}

void CModbusConfigDialogDlg::OnButtonAddlistctrl() 
{
	// TODO: Add your control notification handler code here

	int index = m_list_group.GetCurSel();
	if(index<0){
		MessageBox("请选择组");
		return;
	}
	if(checkPointNumLimit(m_edit_datanum_val + 1)){
		MessageBox("测点数量已达到最大值");
		return;
	}
	m_edit_datanum_val++;
	if(m_state == 0){
		gYcType.m_ycss[m_group_sel].ycslen = m_edit_datanum_val;
		gYcType.m_ycss[m_group_sel].ycs[m_edit_datanum_val - 1].used = 1;
		gYcType.m_ycss[m_group_sel].ycs[m_edit_datanum_val - 1].coe = 1;
		gYcType.m_ycss[m_group_sel].ycs[m_edit_datanum_val - 1].dit = -1;
		strcpy(gYcType.m_ycss[m_group_sel].ycs[m_edit_datanum_val - 1].desc, "");
	}else if(m_state == 1){
		gYxType.m_yxss[m_group_sel].yxslen = m_edit_datanum_val;
		gYxType.m_yxss[m_group_sel].yxs[m_edit_datanum_val - 1].used = 1;
		gYxType.m_yxss[m_group_sel].yxs[m_edit_datanum_val - 1].dit = -1;
		strcpy(gYxType.m_yxss[m_group_sel].yxs[m_edit_datanum_val - 1].desc, "");
	}else if(m_state == 2){
		gDdType.m_ddss[m_group_sel].ddslen = m_edit_datanum_val;
		gDdType.m_ddss[m_group_sel].dds[m_edit_datanum_val - 1].used = 1;
		gDdType.m_ddss[m_group_sel].dds[m_edit_datanum_val - 1].dit = -1;
		gDdType.m_ddss[m_group_sel].dds[m_edit_datanum_val - 1].coe = 1;
		strcpy(gDdType.m_ddss[m_group_sel].dds[m_edit_datanum_val - 1].desc, "");
	}
	UpdateData(FALSE);
	initListCtrl(m_edit_datanum_val);
	updateListCtrlData();
	gIsChanged = true;
}

void CModbusConfigDialogDlg::OnButtonDeletelistctrl() 
{
	// TODO: Add your control notification handler code here
	if(m_edit_datanum_val <= 0){
		return;
	}
	m_edit_datanum_val--;
	UpdateData(FALSE);
	int cursel = m_list_attr.GetSelectionMark();
	if(cursel >= 0){
		if(m_state == 0){
			for(int i = cursel; i < m_edit_datanum_val; i++){
				gYcType.m_ycss[m_group_sel].ycs[i].used = gYcType.m_ycss[m_group_sel].ycs[i+1].used;
				gYcType.m_ycss[m_group_sel].ycs[i].coe = gYcType.m_ycss[m_group_sel].ycs[i+1].coe;
				gYcType.m_ycss[m_group_sel].ycs[i].dit = gYcType.m_ycss[m_group_sel].ycs[i+1].dit;
				strcpy(gYcType.m_ycss[m_group_sel].ycs[i].desc, gYcType.m_ycss[m_group_sel].ycs[i+1].desc);
			}
			
		}else if(m_state == 1){
			for(int i = cursel; i < m_edit_datanum_val; i++){
				gYxType.m_yxss[m_group_sel].yxs[i].used = gYxType.m_yxss[m_group_sel].yxs[i+1].used;
				gYxType.m_yxss[m_group_sel].yxs[i].dit = gYxType.m_yxss[m_group_sel].yxs[i+1].dit;
				strcpy(gYxType.m_yxss[m_group_sel].yxs[i].desc, gYxType.m_yxss[m_group_sel].yxs[i+1].desc);
			}
		}else if(m_state == 2){
			for(int i = cursel; i < m_edit_datanum_val; i++){
				gDdType.m_ddss[m_group_sel].dds[i].used = gDdType.m_ddss[m_group_sel].dds[i+1].used;
				gDdType.m_ddss[m_group_sel].dds[i].dit = gDdType.m_ddss[m_group_sel].dds[i+1].dit;
				gDdType.m_ddss[m_group_sel].dds[i].coe = gDdType.m_ddss[m_group_sel].dds[i+1].coe;
				strcpy(gDdType.m_ddss[m_group_sel].dds[i].desc, gDdType.m_ddss[m_group_sel].dds[i+1].desc);
			}
		}
	}
	initListCtrl(m_edit_datanum_val);
	updateListCtrlData();
	gIsChanged = true;
}

void CModbusConfigDialogDlg::OnRadioYc() 
{
	// TODO: Add your control notification handler code here
	clearUI();

	m_state = 0;
	initListCtrl(0);
	refreshGroupListBox();
	setComboOption();
}

void CModbusConfigDialogDlg::OnRadioDd() 
{
	// TODO: Add your control notification handler code here
	clearUI();

	m_state = 2;
	initListCtrl(0);
	refreshGroupListBox();
	setComboOption();
}

void CModbusConfigDialogDlg::OnRadioYx() 
{
	// TODO: Add your control notification handler code here
	clearUI();

	m_state = 1;
	initListCtrl(0);
	refreshGroupListBox();
	setComboOption();
}

void CModbusConfigDialogDlg::OnButtonDeletetype() 
{
	// TODO: Add your control notification handler code here
	if(m_list_type.GetCurSel()<0){
		MessageBox("请先选择规约");
		return;
	}
	deleteType(m_type_sel);
}

void CModbusConfigDialogDlg::OnButtonDeletegroup() 
{
	// TODO: Add your control notification handler code here
	if(m_list_group.GetCurSel()<0){
		MessageBox("请先选择组");
		return;
	}
	deleteGroup(m_group_sel);
}

void CModbusConfigDialogDlg::getMethodFromFile(){
	int methodnum = ::GetPrivateProfileInt("para","methodnum",-1,inipath);
	if(methodnum == -1){
		MessageBox("读取method.ini文件失败");
		return;
	}
	for(int i =0; i < methodnum; i++){
		string indexstr;
		int2str(i,indexstr);
		string id = "method" + indexstr;
		METHODSTRUCT mstruct;
		//CString method;
		::GetPrivateProfileString(id.c_str(),"desc","Error",mstruct.desc.GetBuffer(20),20,inipath);
		mstruct.no = ::GetPrivateProfileInt(id.c_str(),"no",-1,inipath);
		if(mstruct.desc != "Error" && mstruct.no != -1){
			vMethod.push_back(mstruct);
		}else{
			MessageBox("读取method.ini文件内容失败");
			return;
		}
	}
}

void CModbusConfigDialogDlg::getCompanyFromC5DB(){
	CString sqlStr = "select RTRIM(F1101_CODE) as F1101_CODE ,RTRIM(F1101_DESC) as F1101_DESC from TB1101_COMPANY order by CONVERT(int,F1101_CODE)";
	_RecordsetPtr m_pRecordset;
	if(m_c5db.querySQL(sqlStr, m_pRecordset)){
		vCompany.clear();
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			COMPANYSTRUCT companystr;
			varName = m_pRecordset->GetCollect ("F1101_DESC");
			companystr.desc = (char *)_bstr_t(varName);
			varName = m_pRecordset->GetCollect ("F1101_CODE");
			companystr.id =  (char *)_bstr_t(varName);

			vCompany.push_back(companystr);

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
	}
}

void CModbusConfigDialogDlg::getBigTypeFromC5DBByCompany(CString companyid){
	CString sqlStr = "select RTRIM(F1102_CODE) as F1102_CODE,RTRIM(F1102_DESC) as F1102_DESC,RTRIM(F1101_CODE) as F1101_CODE from TB1102_PROTOCOLTYPE order by CONVERT(int,F1102_CODE)";
	if(companyid.GetLength()){
		sqlStr = "select RTRIM(F1102_CODE) as F1102_CODE,RTRIM(F1102_DESC) as F1102_DESC,RTRIM(F1101_CODE) as F1101_CODE from TB1102_PROTOCOLTYPE where F1101_CODE = '" + companyid + "'";
	}
	getBigTypeFromC5DBBySQL(sqlStr);
}

void CModbusConfigDialogDlg::getBigTypeFromC5DBByBig(CString bigid){
	CString sqlStr = " select RTRIM(F1102_CODE) as F1102_CODE,RTRIM(F1102_DESC) as F1102_DESC,RTRIM(F1101_CODE) as F1101_CODE from TB1102_PROTOCOLTYPE where F1101_CODE = (select F1101_CODE from TB1102_PROTOCOLTYPE where F1102_CODE = '" + bigid + "')";
	getBigTypeFromC5DBBySQL(sqlStr);
}

void CModbusConfigDialogDlg::getBigTypeFromC5DBBySQL(CString sqlStr){
	_RecordsetPtr m_pRecordset;
	if(m_c5db.querySQL(sqlStr, m_pRecordset)){
		vBigtype.clear();
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			BIGTYPESTRUCT bigtypestr;
			varName = m_pRecordset->GetCollect ("F1102_DESC");
			bigtypestr.desc = (char *)_bstr_t(varName);
			varName = m_pRecordset->GetCollect ("F1102_CODE");
			bigtypestr.id =  (char *)_bstr_t(varName);
			varName = m_pRecordset->GetCollect ("F1101_CODE");
			bigtypestr.companyid =  (char *)_bstr_t(varName);
			vBigtype.push_back(bigtypestr);

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
	}
}

void CModbusConfigDialogDlg::int2str(const int &int_temp,string &string_temp)  
{  
	stringstream stream;  
	stream<<int_temp;
	string_temp=stream.str();   //此处也可以用 stream>>string_temp  
}  

int FindIndexInMethodVector(vector<METHODSTRUCT> v, int no){
	int size = v.size();
	while(size>0){
		size--;
		if(no == v.at(size).no){
			return size;
		}
	}
	return -1;
}

int FindIndexInCompanyVector(vector<COMPANYSTRUCT> v, CString id){
	int size = v.size();
	while(size>0){
		size--;
		if(id.Compare(v.at(size).id) == 0){
			return size;
		}
	}
	return -1;
}

int FindIndexInBigtypeVector(vector<BIGTYPESTRUCT> v, CString id){
	int size = v.size();
	while(size>0){
		size--;
		BIGTYPESTRUCT bigstr = v.at(size);
		if(id.Compare(bigstr.id) == 0){
			return size;
		}
	}
	return -1;
}

void CModbusConfigDialogDlg::autoAddDit(){
	int i,j,dit;
	dit = 0;
	for(i = 0; i < gYcType.ycseclen; i++){
		for(j = 0; j < gYcType.m_ycss[i].ycslen; j++){
			gYcType.m_ycss[i].ycs[j].dit = dit;
			dit++;
		}
	}
	dit = 0;
	for(i = 0; i < gYxType.yxseclen; i++){
		for(j = 0; j < gYxType.m_yxss[i].yxslen; j++){
			gYxType.m_yxss[i].yxs[j].dit = dit;
			dit++;
		}
	}
	dit = 0;
	for(i = 0; i < gDdType.ddseclen; i++){
		for(j = 0; j < gDdType.m_ddss[i].ddslen; j++){
			gDdType.m_ddss[i].dds[j].dit = dit;
			dit++;
		}
	}
}

void CModbusConfigDialogDlg::checkDit(){
	int i,j;
	for(i = 0; i < gYcType.ycseclen; i++){
		for(j = 0; j < gYcType.m_ycss[i].ycslen; j++){
			if(gYcType.m_ycss[i].ycs[j].dit == -1){
				CString str;
				str.Format("遥测：%s第%d行点号为空！", gYcType.m_ycss[i].desc, j+1);
				MessageBox(str.GetBuffer(0));
				return;
			}
		}
	}
	for(i = 0; i < gYxType.yxseclen; i++){
		for(j = 0; j < gYxType.m_yxss[i].yxslen; j++){
			if(gYxType.m_yxss[i].yxs[j].dit == -1){
				CString str;
				str.Format("遥信：%s第%d行点号为空！", gYxType.m_yxss[i].desc, j+1);
				MessageBox(str.GetBuffer(0));
				return;
			}
		}
	}
	for(i = 0; i < gDdType.ddseclen; i++){
		for(j = 0; j < gDdType.m_ddss[i].ddslen; j++){
			if(gDdType.m_ddss[i].dds[j].dit == -1){
				CString str;
				str.Format("电度：%s第%d行点号为空！", gDdType.m_ddss[i].desc, j+1);
				MessageBox(str.GetBuffer(0));
				return;
			}
		}
	}
}

void CModbusConfigDialogDlg::initTypes(){
	/*
	int ret = openDB();
	if(ret == -1){
		MessageBox("请新建或打开数据库");
		return;
	}
	for(int id = 0; id < 300; id++){
		bigstruct bigstr;
		
		itoa(id, bigstr.id, 10);
		
		strcpy(bigstr.desc, "备用");
		bigstr.checkcrc = -1;
		bigstr.addrhorl = 'l';
		bigstr.version = 0;
		vbig.push_back(bigstr);

		insertDataBigToDB(bigstr);
	}
	
	bigQuery();
	closeDB();
*/
	/*
	//if(m_c5db.openDB()){
		int key = MessageBox("是否清空规约表并重新初始化？","警告",MB_YESNO);
		if(key == IDYES){
			clearBigInC5DB(m_c5db);
			//initTypes();
			for(int id = 0; id < 300; id++){
				bigstruct bigstr;
				itoa(id, bigstr.id, 10);
				strcpy(bigstr.desc, "备用");
				bigstr.checkcrc = -1;
				bigstr.addrhorl = 'l';
				bigstr.version = 0;
				vbig.push_back(bigstr);

				insertDataBigToC5DB(bigstr, m_c5db);
			}
		}
		
		C5DBbigQuery(m_c5db);

		//m_c5db.closeDB();
	//}
	
	refreshTypeUI();
	*/
}




void CModbusConfigDialogDlg::OnButtonMethodExplain() 
{
	// TODO: Add your control notification handler code here
	ShellExecute(NULL,_T("open"),txtpath,NULL,NULL,SW_SHOWNORMAL);
	int i = GetLastError();
	if(i){
		MessageBox("打开方法说明文档失败");
	}
}

void CModbusConfigDialogDlg::OnButtonTestc5db() 
{
	// TODO: Add your control notification handler code here
	C5DB c5db = C5DB("127.0.0.1","Qwertyuiop1");
	c5db.createDB();
	if(c5db.openDB()){
		CString strSQL = "create table table1 (LastName varchar)";
		c5db.executeSQL(strSQL);
		CString strSQL1 = "INSERT INTO table1 VALUES ('a'),('b'),('c')";
		c5db.executeSQL(strSQL1);
		CString strSQL2 = "select * from table1";
		_RecordsetPtr m_pRecordset;
		if(c5db.querySQL(strSQL2, m_pRecordset)){
			while(!m_pRecordset->GetadoEOF()){
				_variant_t varName;
				varName = m_pRecordset->GetCollect ("LastName");
				CString strName =(char *)_bstr_t(varName);
				strName.TrimRight();

				cout<<strName.GetBuffer(0)<<endl;
				OutputDebugString(strName.GetBuffer(0));
				OutputDebugString("\n");

				m_pRecordset->MoveNext();
			}
			m_pRecordset->Close();
			m_pRecordset.Release();
		}
		//c5db.querySQL(strSQL2, m_pRecordset);
		c5db.closeDB();
	}
}

void CModbusConfigDialogDlg::createC5DB(){
	//C5DB c5db = C5DB("127.0.0.1","Qwertyuiop1");
	m_c5db.createDB();
	//if(c5db.openDB()){
		creatC5BigTable(m_c5db);
		creatC5YcGroupTable(m_c5db);
		creatC5YxGroupTable(m_c5db);
		creatC5DdGroupTable(m_c5db);
		creatC5YcTable(m_c5db);
		creatC5YxTable(m_c5db);
		creatC5DdTable(m_c5db);
	//	c5db.closeDB();
	//}
	
}
void CModbusConfigDialogDlg::creatC5BigTable(C5DB c5db){
	//CString strSQL = "create table big (id varchar(50), describe varchar(50), sort int PRIMARY KEY IDENTITY(1,1), addrhorl char, gcheckcrc int, version int, used int, remark varchar(50))";
	CString strSQL = "	create table TB2001_PROTOCOL (F2001_CODE char(32) NOT NULL, F2001_DESC char(32) NOT NULL, F2001_USED int NOT NULL, F2001_SORT int,F2001_ADDRHL char NOT NULL,F2001_CRC int NOT NULL, F2001_VERSION int NOT NULL, F1102_CODE char(32) NOT NULL, F2001_REMARK char(32), primary key(F2001_CODE));";
	c5db.executeSQL(strSQL);
}
void CModbusConfigDialogDlg::creatC5YcGroupTable(C5DB c5db){
	CString strSQL = "create table ycgroup (bigid varchar(50), id varchar(50), describe varchar(50), addr int, length int, horl char,method int, sort int PRIMARY KEY IDENTITY(1,1), cbdatalenbit int, cbdatahorl char, hasnan int, nankey int, nanvalue int)";
	c5db.executeSQL(strSQL);
}
void CModbusConfigDialogDlg::creatC5YxGroupTable(C5DB c5db){
	CString strSQL = "create table yxgroup (bigid varchar(50), id varchar(50), describe varchar(50), addr int, horl char,method int, sort int PRIMARY KEY IDENTITY(1,1), yxnum int, cbdatalenbit int, cbdatahorl char)";
	c5db.executeSQL(strSQL);
}
void CModbusConfigDialogDlg::creatC5DdGroupTable(C5DB c5db){
	CString strSQL = "create table ddgroup (bigid varchar(50), id varchar(50), describe varchar(50), addr int, length int, horl char,method int, sort int PRIMARY KEY IDENTITY(1,1), cbdatalenbit int, cbdatahorl char, hasnan int, nankey int, nanvalue int)";
	c5db.executeSQL(strSQL);
}
void CModbusConfigDialogDlg::creatC5YcTable(C5DB c5db){
	CString strSQL = "create table yc (bigid varchar(50), groupid varchar(50), id varchar(50), describe varchar(50), coe float, used int, sort int PRIMARY KEY IDENTITY(1,1), startindex int)";
	c5db.executeSQL(strSQL);
}
void CModbusConfigDialogDlg::creatC5YxTable(C5DB c5db){
	CString strSQL = "create table yx (bigid varchar(50), groupid varchar(50), id varchar(50), describe varchar(50), used int, sort int PRIMARY KEY IDENTITY(1,1), startindex int)";
	c5db.executeSQL(strSQL);
}
void CModbusConfigDialogDlg::creatC5DdTable(C5DB c5db){
	CString strSQL = "create table dd (bigid varchar(50), groupid varchar(50), id varchar(50), describe varchar(50), coe float, used int, sort int PRIMARY KEY IDENTITY(1,1), startindex int)";
	c5db.executeSQL(strSQL);
}

void CModbusConfigDialogDlg::insertDataToC5DB(){
}
void CModbusConfigDialogDlg::insertDataYcToC5DB(C5DB c5db){
	/*
	char* ycGroupHeadName = "bigid,id,describe,addr,length,horl,method,cbdatalenbit,cbdatahorl,hasnan,nankey,nanvalue";
	char* ycHeadName = "bigid,groupid,id,describe,coe,used,startindex";
	for(int i = 0; i < gYcType.ycseclen; i++){
		int ycslen = gYcType.m_ycss[i].ycslen;
		if(!ycslen){
			CString str;
			str.Format("遥测数据 %s : 数据数为0，数据未保存！",gYcType.m_ycss[i].desc);
			MessageBox(str.GetBuffer(0));
		}
		
		for(int j = 0; j < ycslen; j++){
			CString str;
			if(j==0){
				str.Format("INSERT INTO ycgroup (%s) VALUES ('%s','%s','%s',%d,%d,'%c',%d,%d,'%c',%d,%d,%d)",
					ycGroupHeadName,
					vbig.at(m_type_sel).id, gYcType.m_ycss[i].id, gYcType.m_ycss[i].desc, gYcType.m_ycss[i].addr,
					gYcType.m_ycss[i].ycdatalen,  gYcType.m_ycss[i].ycs[j].horl, gYcType.m_ycss[i].ycs[j].method,
					gYcType.m_ycss[i].cbdatalenbit, gYcType.m_ycss[i].cbdatahorl, gYcType.m_ycss[i].hasnan,
					gYcType.m_ycss[i].nankey, gYcType.m_ycss[i].nanvalue);
				if(!c5db.executeSQL(str)){
					m_errorNum++;
					MessageBox("MSSQL添加ycgroup表数据失败！"); 
				}
			}
			str.Format("insert into yc (%s) values ('%s','%s','%s','%s',%f,%d,%d)",
				ycHeadName,
				vbig.at(m_type_sel).id,
				gYcType.m_ycss[i].id, gYcType.m_ycss[i].ycs[j].id, gYcType.m_ycss[i].ycs[j].desc, gYcType.m_ycss[i].ycs[j].coe,
				gYcType.m_ycss[i].ycs[j].used, gYcType.m_ycss[i].ycs[j].dit);
			if(!c5db.executeSQL(str)){
				m_errorNum++;
				MessageBox("MSSQL添加yc表数据失败！"); 
			}
		}
	}
	*/

	char* ycGroupHeadName = "F2001_CODE,F2002_CODE,F2002_DESC,F2002_ADDR,F2002_RXDATALEN,F2002_DATALENHL,F2002_METHOD,F2002_RXDATALENBIT,F2002_RXDATALENBITHL,F2002_HASINVALIDVAL,F2002_INVALIDIFVAL,F2002_INVALIDREVAL";
	char* ycHeadName = "F2001_CODE,F2002_CODE,F2003_CODE,F2003_DESC,F2003_COE,F2003_USED,F2003_POINTNO";
	for(int i = 0; i < gYcType.ycseclen; i++){
		int ycslen = gYcType.m_ycss[i].ycslen;
		if(!ycslen){
			CString str;
			str.Format("遥测数据 %s : 数据数为0，数据未保存！",gYcType.m_ycss[i].desc);
			MessageBox(str.GetBuffer(0));
		}
		
		for(int j = 0; j < ycslen; j++){
			CString str;
			if(j==0){
				str.Format("INSERT INTO TB2002_YCGROUP (%s) VALUES ('%s','%s','%s',%d,%d,'%c',%d,%d,'%c',%d,%d,%d)",
					ycGroupHeadName,
					vbig.at(m_type_sel).id, gYcType.m_ycss[i].id, gYcType.m_ycss[i].desc, gYcType.m_ycss[i].addr,
					gYcType.m_ycss[i].ycdatalen,  gYcType.m_ycss[i].horl, gYcType.m_ycss[i].ycs[j].method,
					gYcType.m_ycss[i].cbdatalenbit, gYcType.m_ycss[i].cbdatahorl, gYcType.m_ycss[i].hasnan,
					gYcType.m_ycss[i].nankey, gYcType.m_ycss[i].nanvalue);
				if(!c5db.executeSQL(str)){
					m_errorNum++;
					MessageBox("MSSQL添加ycgroup表数据失败！"); 
				}
			}
			str.Format("insert into TB2003_YCPOINT (%s) values ('%s','%s','%d','%s',%f,%d,%d)",
				ycHeadName,
				vbig.at(m_type_sel).id,
				gYcType.m_ycss[i].id, j, gYcType.m_ycss[i].ycs[j].desc, gYcType.m_ycss[i].ycs[j].coe,
				gYcType.m_ycss[i].ycs[j].used, gYcType.m_ycss[i].ycs[j].dit);
			if(!c5db.executeSQL(str)){
				m_errorNum++;
				MessageBox("MSSQL添加yc表数据失败！"); 
			}
		}
	}
}
void CModbusConfigDialogDlg::insertDataYxToC5DB(C5DB c5db){
	char * yxgroupHeadName = "F2001_CODE,F2004_CODE,F2004_DESC,F2004_ADDR,F2004_DATALENHL,F2004_METHOD,F2004_YXNUM,F2004_RXDATALENBIT,F2004_RXDATALENBITHL";
	char * yxHeadName = "F2001_CODE,F2004_CODE,F2005_CODE,F2005_DESC,F2005_USED,F2005_POINTNO";
	for(int i = 0; i < gYxType.yxseclen; i++){
		int yxslen = gYxType.m_yxss[i].yxslen;
		if(!yxslen){
			CString str;
			str.Format("遥信数据 %s : 数据数为0，数据未保存！",gYxType.m_yxss[i].desc);
			MessageBox(str.GetBuffer(0));
		}
		for(int j = 0; j < yxslen; j++){
			CString str;
			if(j==0){
				str.Format(_T("INSERT INTO TB2004_YXGROUP (%s) VALUES ('%s','%s','%s',%d,'%c',%d,%d,%d,'%c')"),
					yxgroupHeadName,
					vbig.at(m_type_sel).id, gYxType.m_yxss[i].id, gYxType.m_yxss[i].desc, gYxType.m_yxss[i].addr,
					gYxType.m_yxss[i].horl, gYxType.m_yxss[i].method, 
					gYxType.m_yxss[i].yxnum, gYxType.m_yxss[i].cbdatalenbit,
					gYxType.m_yxss[i].cbdatahorl);
				if(!c5db.executeSQL(str)){
					m_errorNum++;
					MessageBox("MSSQL添加yxgroup表数据失败！"); 
				}
			}
			str.Format(_T("INSERT INTO TB2005_YXPOINT (%s) VALUES ('%s','%s','%d','%s',%d,%d)"),
				yxHeadName,
				vbig.at(m_type_sel).id,
				gYxType.m_yxss[i].id, j, gYxType.m_yxss[i].yxs[j].desc, 
				gYxType.m_yxss[i].yxs[j].used, gYxType.m_yxss[i].yxs[j].dit);
			if(!c5db.executeSQL(str)){
				m_errorNum++;
				MessageBox("MSSQL添加yx表数据失败！"); 
			}
		}
	}
}
void CModbusConfigDialogDlg::insertDataDdToC5DB(C5DB c5db){
	char * ddGroupHeadName = "F2001_CODE,F2006_CODE,F2006_DESC,F2006_ADDR,F2006_RXDATALEN,F2006_DATALENHL,F2006_METHOD,F2006_RXDATALENBIT,F2006_RXDATALENBITHL,F2006_HASINVALIDVAL,F2006_INVALIDIFVAL,F2006_INVALIDREVAL";
	char * ddHeadName = "F2001_CODE,F2006_CODE,F2007_CODE,F2007_DESC,F2007_COE,F2007_USED,F2007_POINTNO";
	for(int i = 0; i < gDdType.ddseclen; i++){
		int ddslen = gDdType.m_ddss[i].ddslen;
		if(!ddslen){
			CString str;
			str.Format("电度数据 %s : 数据数为0，数据未保存！",gDdType.m_ddss[i].desc);
			MessageBox(str.GetBuffer(0));
		}
		for(int j = 0; j < ddslen; j++){
			CString str;
			if(j==0){
				str.Format("INSERT INTO TB2006_DDGROUP (%s) VALUES ('%s','%s','%s',%d,%d,'%c',%d,%d,'%c',%d,%d,%d)",
					ddGroupHeadName,
					vbig.at(m_type_sel).id, gDdType.m_ddss[i].id, gDdType.m_ddss[i].desc, gDdType.m_ddss[i].addr, 
					 gDdType.m_ddss[i].dddatalen,  gDdType.m_ddss[i].horl,
					gDdType.m_ddss[i].dds[j].method, gDdType.m_ddss[i].cbdatalenbit, gDdType.m_ddss[i].cbdatahorl,
					gDdType.m_ddss[i].hasnan, gDdType.m_ddss[i].nankey, gDdType.m_ddss[i].nanvalue);
				if(!c5db.executeSQL(str)){
					m_errorNum++;
					MessageBox("MSSQL添加ddgroup表数据失败！"); 
				}
			}
			str.Format("INSERT INTO TB2007_DDPOINT (%s) VALUES ('%s','%s','%d','%s',%f,%d,%d)",
				ddHeadName,
				vbig.at(m_type_sel).id,
				gDdType.m_ddss[i].id, j, gDdType.m_ddss[i].dds[j].desc, gDdType.m_ddss[i].dds[j].coe,
				gDdType.m_ddss[i].dds[j].used, gDdType.m_ddss[i].dds[j].dit);
			if(!c5db.executeSQL(str))
			{
				m_errorNum++;
				MessageBox("MSSQL添加dd表数据失败！"); 
			}
		}
	}
}

void CModbusConfigDialogDlg::insertDataBigToC5DB(bigstruct bigstr, C5DB c5db){
	CString strSQL;
	//strSQL.Format("INSERT INTO big (id,describe,addrhorl,gcheckcrc,version,used,remark) VALUES ('%s','%s','%c',%d,%d,%d,'%s')",bigstr.id,bigstr.desc,
	//	 bigstr.addrhorl, bigstr.checkcrc, bigstr.version, bigstr.used, bigstr.remark);
	//CString strSQL = "insert into big values (id varchar(50), describe varchar(50), sort int PRIMARY KEY IDENTITY(1,1), addrhorl char, gcheckcrc int, version int)";
	strSQL.Format("INSERT INTO TB2001_PROTOCOL (F2001_CODE,F2001_DESC,F2001_ADDRHL,F2001_CRC,F2001_VERSION,F2001_USED,F2001_REMARK,F1102_CODE,F2001_SORT) VALUES ('%s','%s','%c',%d,%d,%d,'%s','%s',%d)",bigstr.id,bigstr.desc,
		 bigstr.addrhorl, bigstr.checkcrc, bigstr.version, bigstr.used, bigstr.remark, bigstr.type,atoi(bigstr.id));
	if(!c5db.executeSQL(strSQL))
	{
		m_errorNum++;
		MessageBox("MSSQL添加big表数据失败！"); 
	}
}


void CModbusConfigDialogDlg::deleteTypeInC5DB(char *bigid, C5DB c5db){
	CString sqlStr;
	
	sqlStr.Format("delete from TB2001_PROTOCOL where F2001_CODE = '%s'",bigid);
	if(!c5db.executeSQL(sqlStr))
	{
		m_errorNum++;
		MessageBox("MSSQL删除big表数据失败！"); 
	}
	deleteYcGroupInC5DB(bigid, c5db);
	deleteYxGroupInC5DB(bigid, c5db);
	deleteDdGroupInC5DB(bigid, c5db);
	deleteYcInC5DB(bigid, c5db);
	deleteYxInC5DB(bigid, c5db);
	deleteDdInC5DB(bigid, c5db);
}
void CModbusConfigDialogDlg::deleteYcInC5DB(char *bigid, C5DB c5db){
	CString sqlStr;
	
	sqlStr.Format("delete from TB2003_YCPOINT where F2001_CODE = '%s'",bigid);
	if (!c5db.executeSQL(sqlStr))
	{
		MessageBox("MSSQL删除yc表数据失败！"); 
	}
}
void CModbusConfigDialogDlg::deleteYxInC5DB(char *bigid, C5DB c5db){
	CString sqlStr;
	
	sqlStr.Format("delete from TB2005_YXPOINT where F2001_CODE = '%s'",bigid);
	if (!c5db.executeSQL(sqlStr))
	{
		MessageBox("MSSQL删除yx表数据失败！"); 
	}
}
void CModbusConfigDialogDlg::deleteDdInC5DB(char *bigid, C5DB c5db){
	CString sqlStr;
	
	sqlStr.Format("delete from TB2007_DDPOINT where F2001_CODE = '%s'",bigid);
	if (!c5db.executeSQL(sqlStr))
	{
		m_errorNum++;
		MessageBox("MSSQL删除dd表数据失败！"); 
	}
}
void CModbusConfigDialogDlg::deleteYcGroupInC5DB(char *bigid, C5DB c5db){
	CString sqlStr;
	
	sqlStr.Format("delete from TB2002_YCGROUP where F2001_CODE = '%s'",bigid);
	if (!c5db.executeSQL(sqlStr))
	{
		m_errorNum++;
		MessageBox("MSSQL删除ycgroup表数据失败！"); 
	}
}
void CModbusConfigDialogDlg::deleteYxGroupInC5DB(char *bigid, C5DB c5db){
	CString sqlStr;
	
	sqlStr.Format("delete from TB2004_YXGROUP where F2001_CODE = '%s'",bigid);
	if (!c5db.executeSQL(sqlStr))
	{
		m_errorNum++;
		MessageBox("MSSQL删除yxgroup表数据失败！"); 
	}
}
void CModbusConfigDialogDlg::deleteDdGroupInC5DB(char *bigid, C5DB c5db){
	CString sqlStr;
	
	sqlStr.Format("delete from TB2006_DDGROUP where F2001_CODE = '%s'",bigid);
	if (!c5db.executeSQL(sqlStr))
	{
		m_errorNum++;
		MessageBox("MSSQL删除ddgroup表数据失败！"); 
	}
}

bool CModbusConfigDialogDlg::C5DBycGroupQuery(char* bigid, C5DB c5db){
	CString id = bigid;
	gYcType.ycseclen = 0;
	char* ycGroupHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2002_CODE) as F2002_CODE,RTRIM(F2002_DESC) as F2002_DESC,F2002_ADDR,F2002_RXDATALEN,F2002_DATALENHL,F2002_METHOD,F2002_RXDATALENBIT,F2002_RXDATALENBITHL,F2002_HASINVALIDVAL,F2002_INVALIDIFVAL,F2002_INVALIDREVAL";

	CString sqlStr;
	sqlStr.Format("select %s from TB2002_YCGROUP where F2001_CODE = '%s' order by CONVERT(int, F2002_CODE)", ycGroupHeadName, id);
	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			gYcType.ycseclen++;
			int ycssindex = gYcType.ycseclen-1;
			gYcType.m_ycss[ycssindex].ycslen = 0;
			varName = m_pRecordset->GetCollect ("F2002_ADDR");
			gYcType.m_ycss[ycssindex].addr = _ttoi((char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("addr");
			gYcType.m_ycss[ycssindex].startindex = 0;
			varName = m_pRecordset->GetCollect ("F2002_RXDATALEN");
			gYcType.m_ycss[ycssindex].ycdatalen = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_RXDATALENBIT");
			gYcType.m_ycss[ycssindex].cbdatalenbit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 1;
			varName = m_pRecordset->GetCollect ("F2002_METHOD");
			gYcType.m_ycss[ycssindex].method = _ttoi((char *)_bstr_t(varName));

			varName = m_pRecordset->GetCollect ("F2002_DATALENHL");
			gYcType.m_ycss[ycssindex].horl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2002_RXDATALENBITHL");
			gYcType.m_ycss[ycssindex].cbdatahorl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2002_HASINVALIDVAL");
			gYcType.m_ycss[ycssindex].hasnan = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_INVALIDIFVAL");
			gYcType.m_ycss[ycssindex].nankey = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_INVALIDREVAL");
			gYcType.m_ycss[ycssindex].nanvalue = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_DESC");
			strcpy(gYcType.m_ycss[ycssindex].desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_CODE");
			strcpy(gYcType.m_ycss[ycssindex].id, (char *)_bstr_t(varName));

			/*bigstruct bigstr;
			_variant_t varName;
			varName = m_pRecordset->GetCollect ("id");
			strcpy(bigstr.id, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("describe");
			strcpy(bigstr.desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("addrhorl");
			bigstr.addrhorl = *(char *)_bstr_t(varName);
			varName = m_pRecordset->GetCollect ("gcheckcrc");
			bigstr.checkcrc = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("version");
			bigstr.version = _ttoi((char *)_bstr_t(varName));
			vbig.push_back(bigstr);
			*/

			//cout<<strName.GetBuffer(0)<<endl;
			//OutputDebugString(strName.GetBuffer(0));
			//OutputDebugString("\n");

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}
bool CModbusConfigDialogDlg::C5DByxGroupQuery(char* bigid, C5DB c5db){
	CString id = bigid;
	char * yxgroupHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2004_CODE) as F2004_CODE,RTRIM(F2004_DESC) as F2004_DESC,F2004_ADDR,F2004_DATALENHL,F2004_METHOD,F2004_YXNUM,F2004_RXDATALENBIT,F2004_RXDATALENBITHL";

	gYxType.yxseclen = 0;
	CString sqlStr;
	sqlStr.Format("select %s from TB2004_YXGROUP where F2001_CODE = '%s' order by CONVERT(int, F2004_CODE)", yxgroupHeadName, id);
	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			gYxType.yxseclen++;
			int yxssindex = gYxType.yxseclen-1;
			gYxType.m_yxss[yxssindex].yxslen = 0;

			varName = m_pRecordset->GetCollect ("F2004_ADDR");
			gYxType.m_yxss[yxssindex].addr = _ttoi((char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("addr");
			gYxType.m_yxss[yxssindex].startindex =  0;
			
			varName = m_pRecordset->GetCollect ("F2004_RXDATALENBIT");
			gYxType.m_yxss[yxssindex].cbdatalenbit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 1;
			varName = m_pRecordset->GetCollect ("F2004_METHOD");
			gYxType.m_yxss[yxssindex].method = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2004_YXNUM");
			gYxType.m_yxss[yxssindex].yxnum = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 0;

			varName = m_pRecordset->GetCollect ("F2004_DATALENHL");
			gYxType.m_yxss[yxssindex].horl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2004_RXDATALENBITHL");
			gYxType.m_yxss[yxssindex].cbdatahorl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			
			varName = m_pRecordset->GetCollect ("F2004_DESC");
			strcpy(gYxType.m_yxss[yxssindex].desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2004_CODE");
			strcpy(gYxType.m_yxss[yxssindex].id, (char *)_bstr_t(varName));

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}
bool CModbusConfigDialogDlg::C5DBddGroupQuery(char* bigid, C5DB c5db){
	gDdType.ddseclen = 0;
	CString id = bigid;
	char * ddGroupHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2006_CODE) as F2006_CODE,RTRIM(F2006_DESC) as F2006_DESC,F2006_ADDR,F2006_RXDATALEN,F2006_DATALENHL,F2006_METHOD,F2006_RXDATALENBIT,F2006_RXDATALENBITHL,F2006_HASINVALIDVAL,F2006_INVALIDIFVAL,F2006_INVALIDREVAL";
	CString sqlStr;
	sqlStr.Format("select %s from TB2006_DDGROUP where F2001_CODE = '%s' order by CONVERT(int, F2006_CODE)",ddGroupHeadName, id);
	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			gDdType.ddseclen++;
			int ddssindex = gDdType.ddseclen-1;
			gDdType.m_ddss[ddssindex].ddslen = 0;
			varName = m_pRecordset->GetCollect ("F2006_ADDR");
			gDdType.m_ddss[ddssindex].addr = _ttoi((char *)_bstr_t(varName));
			gDdType.m_ddss[ddssindex].startindex =  0;
			varName = m_pRecordset->GetCollect ("F2006_RXDATALEN");
			gDdType.m_ddss[ddssindex].dddatalen = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_RXDATALENBIT");
			gDdType.m_ddss[ddssindex].cbdatalenbit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 1;
			varName = m_pRecordset->GetCollect ("F2006_METHOD");
			gDdType.m_ddss[ddssindex].method = _ttoi((char *)_bstr_t(varName));

			varName = m_pRecordset->GetCollect ("F2006_DATALENHL");
			gDdType.m_ddss[ddssindex].horl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2006_RXDATALENBITHL");
			gDdType.m_ddss[ddssindex].cbdatahorl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2006_HASINVALIDVAL");
			gDdType.m_ddss[ddssindex].hasnan = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_INVALIDIFVAL");
			gDdType.m_ddss[ddssindex].nankey = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_INVALIDREVAL");
			gDdType.m_ddss[ddssindex].nanvalue = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_DESC");
			strcpy(gDdType.m_ddss[ddssindex].desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_CODE");
			strcpy(gDdType.m_ddss[ddssindex].id, (char *)_bstr_t(varName));

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}
bool CModbusConfigDialogDlg::C5DBycQuery(char* bigid, char* groupid, int ycssindex, C5DB c5db){
	CString id1 = bigid;
	CString id2= groupid;
	char* ycHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2002_CODE) as F2002_CODE,RTRIM(F2003_CODE) as F2003_CODE,RTRIM(F2003_DESC) as F2003_DESC,F2003_COE,F2003_USED,F2003_POINTNO";

	//gYcType.ycseclen = 0;
	CString sqlStr;
	sqlStr.Format("select %s from TB2003_YCPOINT where F2001_CODE = '%s' and F2002_CODE = '%s' order by CONVERT(int, F2003_CODE)", ycHeadName, id1, id2);
	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			//gYcType.ycseclen++;
			//ycssindex = gYcType.ycseclen-1;
			gYcType.m_ycss[ycssindex].ycslen++;
			int ycsindex = gYcType.m_ycss[ycssindex].ycslen-1;

			varName = m_pRecordset->GetCollect ("F2003_CODE");
			strcpy(gYcType.m_ycss[ycssindex].ycs[ycsindex].id, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2003_DESC");
			strcpy(gYcType.m_ycss[ycssindex].ycs[ycsindex].desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2003_COE");
			gYcType.m_ycss[ycssindex].ycs[ycsindex].coe = (char *)_bstr_t(varName) ? atof((char *)_bstr_t(varName)) : 1.0;
			varName = m_pRecordset->GetCollect ("F2003_USED");
			gYcType.m_ycss[ycssindex].ycs[ycsindex].used = _ttoi((char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("F2003_SORT");
			//gYcType.m_ycss[ycssindex].ycs[ycsindex].sort = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 0;
			varName = m_pRecordset->GetCollect ("F2003_POINTNO");
			gYcType.m_ycss[ycssindex].ycs[ycsindex].dit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : -1;

			gYcType.m_ycss[ycssindex].ycs[ycsindex].horl = gYcType.m_ycss[ycssindex].horl;
			gYcType.m_ycss[ycssindex].ycs[ycsindex].method = gYcType.m_ycss[ycssindex].method;
			gYcType.m_ycss[ycssindex].ycs[ycsindex].length = gYcType.m_ycss[ycssindex].ycdatalen;

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}
bool CModbusConfigDialogDlg::C5DByxQuery(char* bigid, char* groupid, int yxssindex, C5DB c5db){
	CString id1 = bigid;
	CString id2= groupid;
	char * yxHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2004_CODE) as F2004_CODE,RTRIM(F2005_CODE) as F2005_CODE,RTRIM(F2005_DESC) as F2005_DESC,F2005_USED,F2005_POINTNO";

	//gYcType.ycseclen = 0;
	CString sqlStr;
	sqlStr.Format("select %s from TB2005_YXPOINT where F2001_CODE = '%s' and F2004_CODE = '%s' order by CONVERT(int, F2005_CODE)", yxHeadName, id1, id2);
	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			//gYxType.yxseclen++;
			//ycssindex = gYcType.ycseclen-1;
			gYxType.m_yxss[yxssindex].yxslen++;
			int yxsindex = gYxType.m_yxss[yxssindex].yxslen-1;

			varName = m_pRecordset->GetCollect ("F2005_CODE");
			strcpy(gYxType.m_yxss[yxssindex].yxs[yxsindex].id, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2005_DESC");
			strcpy(gYxType.m_yxss[yxssindex].yxs[yxsindex].desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2005_USED");
			gYxType.m_yxss[yxssindex].yxs[yxsindex].used = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2005_POINTNO");
			gYxType.m_yxss[yxssindex].yxs[yxsindex].dit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : -1;

			gYxType.m_yxss[yxssindex].yxs[yxsindex].horl = gYxType.m_yxss[yxssindex].horl;

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}
bool CModbusConfigDialogDlg::C5DBddQuery(char* bigid, char* groupid, int ddssindex, C5DB c5db){
	CString id1 = bigid;
	CString id2= groupid;
	//gYcType.ycseclen = 0;
	char * ddHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2006_CODE) as F2006_CODE,RTRIM(F2007_CODE) as F2007_CODE,RTRIM(F2007_DESC) as F2007_DESC,F2007_COE,F2007_USED,F2007_POINTNO";

	CString sqlStr;
	sqlStr.Format("select %s from TB2007_DDPOINT where F2001_CODE = '%s' and F2006_CODE = '%s' order by CONVERT(int, F2007_CODE)", ddHeadName, id1, id2);
	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			//gDdType.ddseclen++;
			//ycssindex = gYcType.ycseclen-1;
			gDdType.m_ddss[ddssindex].ddslen++;
			int ddsindex = gDdType.m_ddss[ddssindex].ddslen-1;

			varName = m_pRecordset->GetCollect ("F2007_CODE");
			strcpy(gDdType.m_ddss[ddssindex].dds[ddsindex].id, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2007_DESC");
			strcpy(gDdType.m_ddss[ddssindex].dds[ddsindex].desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2007_COE");
			gDdType.m_ddss[ddssindex].dds[ddsindex].coe = (char *)_bstr_t(varName) ? atof((char *)_bstr_t(varName)) : 1.0;
			varName = m_pRecordset->GetCollect ("F2007_USED");
			gDdType.m_ddss[ddssindex].dds[ddsindex].used = _ttoi((char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("F2007_SORT");
			//gDdType.m_ddss[ddssindex].dds[ddsindex].sort = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 0;
			varName = m_pRecordset->GetCollect ("F2007_POINTNO");
			gDdType.m_ddss[ddssindex].dds[ddsindex].dit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : -1;

			gDdType.m_ddss[ddssindex].dds[ddsindex].horl = gDdType.m_ddss[ddssindex].horl;
			gDdType.m_ddss[ddssindex].dds[ddsindex].method = gDdType.m_ddss[ddssindex].method;
			gDdType.m_ddss[ddssindex].dds[ddsindex].length = gDdType.m_ddss[ddssindex].dddatalen;

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}
bool CModbusConfigDialogDlg::C5DBbigQuery(C5DB c5db){
	char* bigHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2001_DESC) as F2001_DESC,F2001_ADDRHL,F2001_CRC,F2001_VERSION,F2001_USED,RTRIM(F2001_REMARK) as F2001_REMARK,RTRIM(F1102_CODE) as F1102_CODE";
	CString sqlStr;
	sqlStr.Format("select %s from TB2001_PROTOCOL order by convert(int,F2001_CODE) asc", bigHeadName);

	vbig.clear();
	//int nRes = sqlite3_exec(pDB , sqlStr.c_str() ,bigResult ,this, &cErrMsg);
	//if (nRes != SQLITE_OK)
	//{
	//	MessageBox("查询数据库big表失败");
	//	return -1;
	//}

	//return 0;
	clock_t start,finish;
	//clock_t middle1,middle2;
	int duration;

	start = clock();
	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			//middle1 = clock();

			bigstruct bigstr;
			_variant_t varName;
			varName = m_pRecordset->GetCollect ("F2001_CODE");
			strcpy(bigstr.id, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2001_DESC");
			strcpy(bigstr.desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2001_ADDRHL");
			bigstr.addrhorl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2001_CRC");
			bigstr.checkcrc = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : -1;
			varName = m_pRecordset->GetCollect ("F2001_VERSION");
			bigstr.version = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 0;
			varName = m_pRecordset->GetCollect ("F2001_USED");
			bigstr.used = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 0;
			varName = m_pRecordset->GetCollect ("F2001_REMARK");
			strcpy(bigstr.remark, varName.vt!=VT_NULL ? (char *)_bstr_t(varName) : "");
			varName = m_pRecordset->GetCollect ("F1102_CODE");
			strcpy(bigstr.type, (char *)_bstr_t(varName));
			vbig.push_back(bigstr);

			//cout<<strName.GetBuffer(0)<<endl;
			//OutputDebugString(strName.GetBuffer(0));
			//OutputDebugString("\n");

			m_pRecordset->MoveNext();
			//middle2 = clock();
			//CString strm;
			//strm.Format("一次查询BIG花费了%d毫秒\n", middle2-middle1);
			//OutputDebugString(strm.GetBuffer(0));
		}
		m_pRecordset->Close();
		m_pRecordset.Release();


		finish = clock();
		duration = finish - start;
		CString strt;
		strt.Format("查询BIG花费了%d毫秒\n", duration);
		OutputDebugString(strt.GetBuffer(0));
		return true;
	}
	
	return false;
}

void CModbusConfigDialogDlg::updateBigidInC5DB(char* oldid,char* newid, C5DB c5db){
	CString str;
//	str.Format("UPDATE big SET id = '%s' WHERE id = '%s';UPDATE yc SET bigid = '%s' WHERE bigid = '%s';UPDATE yx SET bigid = '%s' WHERE bigid = '%s';UPDATE dd SET bigid = '%s' WHERE bigid = '%s';UPDATE ycgroup SET bigid = '%s' WHERE bigid = '%s';UPDATE yxgroup SET bigid = '%s' WHERE bigid = '%s';UPDATE ddgroup SET bigid = '%s' WHERE bigid = '%s';",newid,oldid ,newid,oldid ,newid,oldid ,newid,oldid,newid,oldid ,newid,oldid ,newid,oldid);
	str.Format("UPDATE TB2001_PROTOCOL SET F2001_CODE = '%s' WHERE F2001_CODE = '%s';UPDATE TB2003_YCPOINT SET F2001_CODE = '%s' WHERE F2001_CODE = '%s';UPDATE TB2005_YXPOINT SET F2001_CODE = '%s' WHERE F2001_CODE = '%s';UPDATE TB2007_DDPOINT SET F2001_CODE = '%s' WHERE F2001_CODE = '%s';UPDATE TB2002_YCGROUP SET F2001_CODE = '%s' WHERE F2001_CODE = '%s';UPDATE TB2004_YXGROUP SET F2001_CODE = '%s' WHERE F2001_CODE = '%s';UPDATE TB2006_DDGROUP SET F2001_CODE = '%s' WHERE F2001_CODE = '%s';",newid,oldid ,newid,oldid ,newid,oldid ,newid,oldid,newid,oldid ,newid,oldid ,newid,oldid);
	if (!c5db.executeSQL(str))
	{
		m_errorNum++;
		MessageBox("MSSQL更新bigid失败！"); 
	}
}
void CModbusConfigDialogDlg::updateBigDescInC5DB(char* oldid, char* newdesc, C5DB c5db){
	CString str;
//	str.Format("UPDATE big SET describe = '%s' WHERE id = '%s';",newdesc,oldid);
	str.Format("UPDATE TB2001_PROTOCOL SET F2001_DESC = '%s' WHERE F2001_CODE = '%s';",newdesc,oldid);
	if (!c5db.executeSQL(str))
	{
		m_errorNum++;
		MessageBox("MSSQL更新big描述失败！！"); 
	}
}

void CModbusConfigDialogDlg::OnCheckBigused() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	UpdateData(TRUE);
	saveBigConfig();
}

void CModbusConfigDialogDlg::OnChangeEditRemark() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	UpdateData(TRUE);
	saveBigConfig();
}


void CModbusConfigDialogDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	//CDialog::OnOK();
}

void CModbusConfigDialogDlg::OnSelchangeComboCompany() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	UpdateData(TRUE);

	getBigTypeFromC5DBByCompany(vCompany.at(m_combo_company_val).id);

	m_combo_bigtype.ResetContent();
	int len = vBigtype.size();
	for(int i = 0; i < len; i++){
		m_combo_bigtype.AddString(vBigtype.at(i).desc.GetBuffer(0));
	}
}

void CModbusConfigDialogDlg::OnSelchangeComboBigtype() 
{
	// TODO: Add your control notification handler code here
	gIsChanged = true;
	UpdateData(TRUE);
	saveBigConfig();
}

void CModbusConfigDialogDlg::OnButtonAddcompany() 
{
	// TODO: Add your control notification handler code here
	if(m_list_type.GetCurSel()<0){
		MessageBox("请先选择规约");
		return;
	}
	CCompanyDialog companyDialog;
	if(companyDialog.DoModal()==IDOK){
		getCompanyFromC5DB();
		C5DBbigQuery(m_c5db);
		refreshTypeUI();
	}
}
