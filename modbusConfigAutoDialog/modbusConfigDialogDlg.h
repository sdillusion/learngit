// modbusConfigDialogDlg.h : header file
//

#if !defined(AFX_MODBUSCONFIGDIALOGDLG_H__B24EB65B_416E_4AAE_AF4F_78C3E8B337C5__INCLUDED_)
#define AFX_MODBUSCONFIGDIALOGDLG_H__B24EB65B_416E_4AAE_AF4F_78C3E8B337C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<string>
#include<vector>
//#include "c5db.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CModbusConfigDialogDlg dialog

typedef int		    sint32;
const int SECTIONNUM = 32;
//const int TYPENUM = 32;
const int STRUCTNUM = 64;
const int SYSC2 = 0;
const int SYSC5 = 1;

#define CHARLEN 50

struct METHODSTRUCT{
	CString desc;
	int no;
};

struct COMPANYSTRUCT{
	CString desc;
	CString id;
};

struct BIGTYPESTRUCT{
	CString desc;
	CString id;
	CString companyid;
};
/*
struct commonData{
	int reqInterval;
	char checkhorl;
	int cbcheckcrc;
	int cbdatalenbit;
	char cbdatalenhorl;

	int heartInterval;
	char heartHead[CHARLEN];
	//char heartTail[CHARLEN];
	char registeHead[CHARLEN];
	char registeTail[CHARLEN];
	char cbHeartSucceed[CHARLEN];
	char cbHeartFail[CHARLEN];
	char cbRegiste[CHARLEN];
};
*/
struct bigstruct{
	char id[CHARLEN];
	char desc[CHARLEN];

	//char checkhorl;
	int checkcrc;
	char addrhorl;
	int version;
	int used;
	char remark[CHARLEN];
	char type[CHARLEN];
	/*
	char name[CHARLEN];
	int heartInterval;
	char heartHead[CHARLEN];
	//char heartTail[CHARLEN];
	char registeHead[CHARLEN];
	char registeTail[CHARLEN];
	char cbHeartSucceed[CHARLEN];
	char cbHeartFail[CHARLEN];
	char cbRegiste[CHARLEN];
	int rtuno;
	*/
};
struct ycstruct{
	char id[CHARLEN];
	char desc[CHARLEN];
	float coe;
	int rel;
	int length;
	char horl;
	int method;
	int used;
	int sort;
	int dit;//点号
};
struct ycsection{
	sint32 addr;
	ycstruct ycs[STRUCTNUM];
	int ycslen;
	int ycdatalen;
	int startindex;//putAYC时的开始序列号
	char id[CHARLEN];
	char desc[CHARLEN];
	int cbdatalenbit;
	char cbdatahorl;
	char horl;
	int method;
	int hasnan;//是否有无效值
	int nankey;//无效值
	int nanvalue;//无效值返回数值
	int funcode;//功能编码   默认03
};

struct yctype{
	ycsection m_ycss[SECTIONNUM];
	int ycseclen;
	char desc[CHARLEN];
};

struct yxstruct{
	char id[CHARLEN];
	char desc[CHARLEN];
	//int coe;
	//int length;
	char horl;
	int used;
	int dit;//点号
};
struct yxsection{
	sint32 addr;
	yxstruct yxs[STRUCTNUM];
	int yxslen;
	int yxdatalen;
	int startindex;
	int yxnum;
	int method;
	int offset;
	char horl;
	char id[CHARLEN];
	char desc[CHARLEN];
	int cbdatalenbit;
	char cbdatahorl;
	int funcode;//功能编码   默认03
};
struct yxtype{
	yxsection m_yxss[SECTIONNUM];
	int yxseclen;
	char desc[CHARLEN];
};


struct ddstruct{
	char id[CHARLEN];
	char desc[CHARLEN];
	float coe;
	int rel;
	int length;
	char horl;
	int method;
	int used;
	int sort;
	int dit;//点号
};
struct ddsection{
	sint32 addr;
	ddstruct dds[STRUCTNUM];
	int ddslen;
	int dddatalen;
	int startindex;
	int reqnum;
	char id[CHARLEN];
	char desc[CHARLEN];
	int cbdatalenbit;
	char cbdatahorl;
	char horl;
	int method;
	int hasnan;//是否有无效值
	int nankey;//无效值
	int nanvalue;//无效值返回数值
	int funcode;//功能编码   默认03
};
struct ddtype{
	ddsection m_ddss[SECTIONNUM];
	int ddseclen;
	char desc[CHARLEN];
};

struct ykstruct{
	char id[CHARLEN];
	char desc[CHARLEN];
	int length;
	char horl;
	int used;
	int index;
	int sort;
};
struct yksection{
	sint32 addr;
	ykstruct yks[STRUCTNUM];
	int ykslen;
};
struct yktype{
	yksection m_ykss[SECTIONNUM];
	int ykseclen;
};

struct ytstruct{
	char id[CHARLEN];
	char desc[CHARLEN];
	char horl;
	int index;
	int used;
	int sort;
	int length;
};
struct ytsection{
	sint32 addr;
	ytstruct yts[STRUCTNUM];
	int ytslen;
};
struct yttype{
	ytsection m_ytss[SECTIONNUM];
	int ytseclen;
};




class CModbusConfigDialogDlg : public CDialog
{
// Construction
public:
	CModbusConfigDialogDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CModbusConfigDialogDlg)
	enum { IDD = IDD_MODBUSCONFIGDIALOG_DIALOG };
	CComboBox	m_combo_funcode;
	CComboBox	m_combo_bigtype;
	CComboBox	m_combo_company;
	CButton	m_check_bigused;
	CEdit	m_edit_nanvalue;
	CEdit	m_edit_nankey;
	CButton	m_check_hasnan;
	CEdit	m_edit_bigid;
	CEdit	m_edit_cbdatalenbit;
	CEdit	m_edit_reqnum;
	CButton	m_radio_yc;
	CStatic	m_static_typeindex;
	CEdit	m_edit_groupdesc;
	CEdit	m_edit_typedesc;
	CEdit	m_edit_offset;
	CComboBox	m_combo_horl;
	CComboBox	m_combo_rtuinfo;
	CListBox	m_list_type;
	CComboBox	m_combo_datalen;
	CComboBox	m_combo_method;
	CListBox	m_list_group;
	CEdit	m_edit_tmp;
	CListCtrl	m_list_attr;
	CString	m_combo_addr_val;
	int		m_combo_horl_val;
	int		m_combo_crc_val;
	int		m_combo_method_val;
	int		m_combo_datalen_val;
	int		m_edit_datanum_val;
	int		m_edit_startindex_val;
	int		m_combo_addrhorl_val;
	int		m_edit_reqnum_val;
	int		m_edit_cbdatalenbit_val;
	int		m_edit_bigid_val;
	int		m_combo_cbdatahorl_val;
	BOOL	m_check_hasnan_val;
	int		m_edit_nankey_val;
	int		m_edit_nanvalue_val;
	BOOL	m_check_bigused_val;
	CString	m_edit_remark_val;
	int		m_combo_company_val;
	int		m_combo_bigtype_val;
	CString	m_combo_funcode_val;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModbusConfigDialogDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
	
	void setComboOption();//设置部分下拉框选项

	void clearUI();//清空右侧属性界面，主要是属性值。

	void refreshUI();//刷新右侧属性界面，主要是属性值。
	void refreshUIyc();
	void refreshUIyx();
	void refreshUIdd();
	void refreshUIcommon();
	//void refreshUIbig();
	void refreshTypeUI();

	void refreshGroupListBox();//刷新组列表显示内容
	//void refreshTypeListBox();//刷新类列表显示内容

	
	void deleteGroup(int index);
	void deleteType(int index);

	void saveConfig();
	void saveYcConfig();
	void saveYxConfig();
	void saveDdConfig();
	void saveCommonConfig();
	void saveBigConfig();

	void initTypes();//初始化规约300个

	void changeListCtrlColumn();//改变控制列表的列数
	void initListCtrl(int num);//初始化控制列表
	void updateListCtrlData();//更新控制列表数据

	bool checkBigid(int id, int index);//检查bigid是否有重复
	bool checkPointNumLimit(int num);//检查测点个数是否超限

	void creatBigTable();
	void creatCommonTable();
	void creatYcTable();
	void creatYcGroupTable();
	void creatYxTable();
	void creatYxGroupTable();
	void creatDdTable();
	void creatDdGroupTable();


	void deleteTypeInDB(char *id);
	void deleteYcGroupInDB(char *bigid);
	void deleteYxGroupInDB(char *bigid);
	void deleteDdGroupInDB(char *bigid);
	void deleteYcInDB(char *bigid);
	void deleteYxInDB(char *bigid);
	void deleteDdInDB(char *bigid);

	//void insertDataToDB();//数据库插入数据
	void insertDataYcToDB();
	void insertDataYxToDB();
	void insertDataDdToDB();
	void insertDataBigToDB(bigstruct bigstr);

	void updateBigidInDB(char* oldid,char* newid);
	void updateBigDescInDB(char* oldid, char* newdesc);

	void dropAllTable();//删除所有表

	void checkIsChanged();
	void checkDit();
	void autoAddDit();

	int openDB();
	int closeDB();
	int sqlExec(char* sql);
	int ycGroupQuery(char* bigid);
	int yxGroupQuery(char* bigid);
	int ddGroupQuery(char* bigid);
	int ycQuery(char* bigid, char* groupid, int ycssindex);
	int yxQuery(char* bigid, char* groupid, int yxssindex);
	int ddQuery(char* bigid, char* groupid, int ddssindex);
	int bigQuery();
	int ykQuery();
	int ytQuery();

	void bigResultFormat(int nrownum, int ncolnum, char **argv);
	void ycGroupResultFormat(int nrownum, int ncolnum, char **argv);
	void ycResultFormat(int nrownum, int ncolnum, char **argv, int ycssindex);
	void yxResultFormat(int nrownum, int ncolnum, char **argv, int yxssindex);
	void yxGroupResultFormat(int nrownum, int ncolnum, char **argv);
	void ddResultFormat(int nrownum, int ncolnum, char **argv, int ddssindex);
	void ddGroupResultFormat(int nrownum, int ncolnum, char **argv);

	int getDataFromDB();

	void getMethodFromFile();
	void getCompanyFromC5DB();
	void getBigTypeFromC5DBBySQL(CString sqlStr);//通过SQL获取规约
	void getBigTypeFromC5DBByCompany(CString companyid);
	void getBigTypeFromC5DBByBig(CString bigid);//获取与此规约id厂家相同的规约
	void int2str(const int &int_temp,std::string &string_temp);
	
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CModbusConfigDialogDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDblclkListAttr(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEditTmp();
	afx_msg void OnButtonAddgroup();
	afx_msg void OnSelchangeListGroup();
	afx_msg void OnEditchangeComboAddr();
	afx_msg void OnSelchangeComboAddr();
	afx_msg void OnSelchangeComboHorl();
	afx_msg void OnSelchangeComboDatalen();
	afx_msg void OnSelchangeComboCrc();
	afx_msg void OnSelchangeComboMethod();
	afx_msg void OnButtonAddType();
	afx_msg void OnSelchangeListType();
	afx_msg void OnButtonOpen();
	afx_msg void OnDblclkListGroup();
	afx_msg void OnDblclkListType();
	afx_msg void OnButtonChangetypedesc();
	afx_msg void OnButtonChangegroupdesc();
	afx_msg void OnButtonNewdb();
	afx_msg void OnButtonSaveconfigtodb();
	afx_msg void OnButtonAddlistctrl();
	afx_msg void OnButtonDeletelistctrl();
	afx_msg void OnRadioYc();
	afx_msg void OnRadioDd();
	afx_msg void OnRadioYx();
	afx_msg void OnButtonDeletetype();
	afx_msg void OnButtonDeletegroup();
	afx_msg void OnSelchangeComboAddrhorl();
	afx_msg void OnChangeEditReqnum();
	afx_msg void OnCheckHasnan();
	afx_msg void OnChangeEditNankey();
	afx_msg void OnChangeEditNanvalue();
	afx_msg void OnButtonMethodExplain();
	afx_msg void OnCheckBigused();
	afx_msg void OnChangeEditRemark();
	virtual void OnOK();
	afx_msg void OnSelchangeComboCompany();
	afx_msg void OnSelchangeComboBigtype();
	afx_msg void OnButtonAddcompany();
	afx_msg void OnChangeEditCbdatalenbit();
	afx_msg void OnEditchangeComboFuncode();
	afx_msg void OnSelchangeComboFuncode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	int m_Row;
	int m_Col;

	int m_state;//0yc 1yx 2dd 3yk 4yt

	int m_ycsectionNum;
	int m_ddsectionNum;
	int m_yxsectionNum;
	int m_yksectionNum;
	int m_ytsectionNum;

	int m_group_sel;
	int m_type_sel;

	int m_yctypeNum;
	int m_yxtypeNum;
	int m_ddtypeNum;
	int m_yktypeNum;
	int m_yttypeNum;

	int m_typeNum;

	//C5DB m_c5db;
	int m_errorNum;
};
static int CALLBACK listCtrlCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CopyFile(char *SourceFile,char *NewFile);
int FindIndexInMethodVector(vector<METHODSTRUCT> v, int no);
int FindIndexInCompanyVector(vector<COMPANYSTRUCT> v, CString id);
int FindIndexInBigtypeVector(vector<BIGTYPESTRUCT> v, CString id);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODBUSCONFIGDIALOGDLG_H__B24EB65B_416E_4AAE_AF4F_78C3E8B337C5__INCLUDED_)
