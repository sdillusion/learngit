#include "StdAfx.h"
#include "c5db.h"
#include <iostream>
#include <Windows.h>
//#include <sstream>
//#include <stdio.h>


using namespace std;

//const char* dbname = "WJCPROTOCOL";
const char* dbname = "JSKWJC";

C5DB::C5DB(CString ip, CString password){
	m_ip = ip;
	m_password = password;
}

void C5DB::createDB(){
	CoInitialize(NULL);
    _ConnectionPtr  sqlSp;
    HRESULT hr=sqlSp.CreateInstance(_uuidof(Connection));
    if(FAILED(hr))
    {
        cout<<"_ConnectionPtr����ָ��ʵ����ʧ�ܣ�����"<<endl;
		OutputDebugString("_ConnectionPtr����ָ��ʵ����ʧ�ܣ�����\n");
        return;
    }
    else {
        try {
			CString strCon;
			strCon.Format("Driver={sql server};server=%s,1433;uid=sa;pwd=%s;", m_ip, m_password);
            //_bstr_t strConnect="Driver={sql server};server=127.0.0.1,1433;uid=sa;pwd=Qwertyuiop1;";
			_bstr_t strConnect = (_bstr_t)strCon;
            sqlSp->Open(strConnect,"","",adModeUnknown);
        }
        catch(_com_error &e) {
			cout<<"��ʧ��"<<endl;
            cout<<(char*)(e.Description())<<endl;
			OutputDebugString("��ʧ��\n");
			OutputDebugString((char*)(e.Description()));
			OutputDebugString("\n");
        }
        
		CString strSQL = "create database WJCPROTOCOL";
		_variant_t varSQL(strSQL);
		COleVariant vtOptional((long)DISP_E_PARAMNOTFOUND,VT_ERROR);
		try{
			sqlSp->Execute(_bstr_t(strSQL),&vtOptional,-1);
			cout<<"�������ݿ�ɹ�"<<endl;
			OutputDebugString("�������ݿ�ɹ�\n");
		}catch(_com_error &e)
        {
			cout<<"�������ݿ�ʧ��"<<endl;
            cout << (LPCSTR)e.Description()<<endl;

			OutputDebugString("�������ݿ�ʧ��\n");
			OutputDebugString((char*)(e.Description()));
			OutputDebugString("\n");

        }

		//strSQL = "create table table1 (LastName varchar)"; 
		//try{
		//	sqlSp->Execute(_bstr_t(strSQL),&vtOptional,-1);
		//	cout<<"������ɹ�"<<endl;
		//}catch(_com_error &e)
        //{
		//	cout<<"������ʧ��"<<endl;
        //    cout << (LPCSTR)e.Description()<<endl;
        //}

		sqlSp.Release();
		cout<<"OK!"<<endl;
	}
}

bool C5DB::executeSQL(CString strSQL){
	_variant_t varSQL(strSQL);
	COleVariant vtOptional((long)DISP_E_PARAMNOTFOUND,VT_ERROR);
	try{
		gsqlSp->Execute(_bstr_t(strSQL),&vtOptional,-1);
		cout<<strSQL.GetBuffer(0)<<endl;
		cout<<"SQLִ�гɹ�"<<endl;


		
		OutputDebugString(strSQL.GetBuffer(0));
		OutputDebugString("\n");
		OutputDebugString("SQLִ�гɹ�\n");
	}catch(_com_error &e)
    {
		cout<<"SQLִ��ʧ��"<<endl;
        cout << (LPCSTR)e.Description()<<endl;
		
		OutputDebugString(strSQL.GetBuffer(0));
		OutputDebugString("SQLִ��ʧ��\n");
		OutputDebugString((char*)(e.Description()));
		OutputDebugString("\n");
		return false;
    }
	return true;
}

bool C5DB::openDB(){
	CoInitialize(NULL);
    HRESULT hr=gsqlSp.CreateInstance(_uuidof(Connection));
    if(FAILED(hr))
    {
        cout<<"_ConnectionPtr����ָ��ʵ����ʧ�ܣ�����"<<endl;
        return false;
    }else {
        try {
			CString strCon;
			strCon.Format("Driver={sql server};server=%s,1433;uid=sa;pwd=%s;database=%s", m_ip, m_password,dbname);
			_bstr_t strConnect = (_bstr_t)strCon;
            //_bstr_t strConnect="Driver={sql server};server=127.0.0.1,1433;uid=sa;pwd=Qwertyuiop1;database=WJCPROTOCOL";
            gsqlSp->Open(strConnect,"","",adModeUnknown);
        }
        catch(_com_error &e) {
			cout<<"��ʧ��"<<endl;
            cout<<(char*)(e.Description())<<endl;

			OutputDebugString("��ʧ��\n");
			OutputDebugString((char*)(e.Description()));
			OutputDebugString("\n");
			return false;
        }
		return true;
	}
}

bool C5DB::querySQL(CString strSQL, _RecordsetPtr & m_pRecordset){
	//_RecordsetPtr m_pRecordset;
    if(FAILED(m_pRecordset.CreateInstance( _uuidof( Recordset ))))
    {
        cout<<"��¼������ָ��ʵ����ʧ�ܣ�"<<endl;
		OutputDebugString("��¼������ָ��ʵ����ʧ�ܣ�\n");
        return false;
    }
	_variant_t varSQL(strSQL);
    try {
        m_pRecordset->Open(varSQL,(IDispatch*)gsqlSp,adOpenForwardOnly,adLockReadOnly, adCmdText);
    }
    catch (_com_error &e)
    {
		cout<<"��ѯʧ��"<<endl;
        cout << e.Description().length()<<endl;

		OutputDebugString(strSQL.GetBuffer(0));
		OutputDebugString("\n");
		OutputDebugString("��ѯʧ��\n");
		OutputDebugString((char*)(e.Description()));
		OutputDebugString("\n");
		return false;
    }
	return true;
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

void C5DB::closeDB(){
	try{
		gsqlSp->Close();
		gsqlSp.Release();
		
	}catch(_com_error &e) {
		cout<<"�ر�����ʧ��"<<endl;
        cout<<(char*)(e.Description())<<endl;

		OutputDebugString("�ر�����ʧ��\n");
		OutputDebugString((char*)(e.Description()));
		OutputDebugString("\n");
    }
}

void C5DB::beginTrans(){
	gsqlSp->BeginTrans();
}

void C5DB::commitTrans(){
	gsqlSp->CommitTrans();
}

void C5DB::rollbackTrans(){
	gsqlSp->RollbackTrans();
}