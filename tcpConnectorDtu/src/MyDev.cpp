#include <stdio.h>
#include "MyDev.h"

CMyDev::CMyDev()
{
}

CMyDev::~CMyDev()
{
	Close();
}

int CMyDev::Open(S_DEVCFG* pcfg)
{
    m_connector.Clean();
	
    return m_connector.Initialize(pcfg->DevAddr) ? DEF_DEV_STATE_OPENED : DEF_DEV_OPENERROR;
}

int CMyDev::Close()
{
    m_connector.Clean();
    return 1;
}

int CMyDev::Check()
{
	return m_connector.IsInitialized() ? DEF_DEV_STATE_OPENED : DEF_DEV_STATE_CLOSED;
}

int CMyDev::Read(uchar *buf, int len, int TimeOut)
{    
	m_connector.Run();
    return 0;
}

int CMyDev::Write(uchar *buf, int len, int TimeOut)
{
    return len;
}

//设备对象创建函数
DLLEXPORT
CDevCore* CreateDevDrive()
{
	//printf("tcpConnector::CreateDevDrive...\n");
    return new CMyDev;
}