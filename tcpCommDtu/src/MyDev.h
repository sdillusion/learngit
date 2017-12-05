#pragma once

#include "comm/devbase.h"
#include "comm/connector.h"

class CMyDev: public CDevCore
{
public:
	CMyDev();
	virtual ~CMyDev();

public:
	virtual int Open(S_DEVCFG* pcfg);
	virtual int Close();
	virtual int Read(uchar* buf, int len, int timeout = 0);	
	virtual int Write(uchar* buf, int len, int timeout = 0);
	virtual int Check();

private:
	int OnError();

private:
	sint32	m_socketId;
	CConnector* m_connector;
};