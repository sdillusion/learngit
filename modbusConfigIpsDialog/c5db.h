#if !defined(C5DB_H_FILE)
#define C5DB_H_FILE

#include<string>
#include<vector>

using namespace std;
#import "c:\program files\common files\system\ado\msado15.dll"  no_namespace rename("EOF", "adoEOF")

class C5DB{
public:
	C5DB(){};
	C5DB(CString ip, CString password);
	void createDB();
	bool openDB();
	void closeDB();
	bool executeSQL(CString strSQL);
	bool querySQL(CString strSQL, _RecordsetPtr & m_pRecordset);

	void beginTrans();
	void commitTrans();
	void rollbackTrans();

	_ConnectionPtr  gsqlSp;
private:
	CString m_ip;
	CString m_password;
};
#endif