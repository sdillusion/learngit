#pragma once

#include "comm/devbase.h"

#include "comm/connector.h"
#include "myLock.h"
#include <list>

typedef struct tagConnectInfo
{	
	char peerIp [32];
	int peerPort;
	int socketId;
	int time;
	char devid[128];
	bool isonline;
}	ConnectInfo;

class CMyConnector : public CConnector
{       
public:
    CMyConnector();
    virtual ~CMyConnector();

public:
	virtual void* GetMine(char* para, FUNC_COMPAREPARA* func);
	
public:
	bool Initialize(char* para);
	void Clean();
	bool IsInitialized();
	void Run();

private:
	void checkList();
	bool ParseBuf(char*buf, int len, char*result);
	bool ParsePara(char* para);
	bool CreateSocket();
	bool Listen();
	bool Accept(int& socketId, struct sockaddr_in& peer);
	void SetSocketUnblocked(int socketId);
	int	GetNowSecond();
	int str_to_hex(char *string, unsigned char *cbuf, int len);
	void getHeadAndTailFromFile();
	int char2x(char c);
	//void int2str(const int &int_temp,std::string &string_temp);

private:
	bool m_bInitialized;
	char m_ip [32];
	int m_port;
	int m_socketId;
	std::list<ConnectInfo> m_list;
	CMyLock m_lock;
	unsigned char m_bufHead[16];
	unsigned char m_bufTail[16];

	char m_inipath[32];
};
	