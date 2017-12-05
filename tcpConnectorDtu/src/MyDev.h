#pragma once

#include "comm/devbase.h"
#include "tcp.h"

class CMyDev : public CDevCore
{       
public:
    CMyDev();
    virtual ~CMyDev();
	
public:
    virtual int Open(S_DEVCFG* pcfg);
    virtual int Close();
    virtual int Read(uchar* buf, int len, int TimeOut = 0);	
    virtual int Write(uchar* buf, int len, int TimeOut = 0);
    virtual int Check();

private:
	CMyConnector m_connector;
};
