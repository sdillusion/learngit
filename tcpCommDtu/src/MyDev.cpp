#include <assert.h>
#include "MyDev.h"
#include "plat_comm.h"

#ifdef WIN32
#include <process.h>

#define close	closesocket
#define getpid	_getpid
#endif

CMyDev::CMyDev()
{
	m_socketId = -1;
	m_connector = NULL;
}

CMyDev::~CMyDev()
{
	Close();
}

const int share_mem_id = 5239105;
const int share_mem_size = 4096;

static bool compare(char* para, char* devid, int isonline)
{
	if (para == NULL || devid == NULL || !isonline)
		return false;
	if (strstr(para, devid) != NULL)
		return true;
	return false;
}

int CMyDev::Open(S_DEVCFG* pcfg)
{
	if (m_connector == NULL)
	{
		CShareMem shareMem;
		void ** pMem = (void **) shareMem.MappingShm(share_mem_id, share_mem_size);
		if (pMem == NULL)
			return DEF_DEV_OPENERROR;

		bool ok = pMem[0] == (void *) getpid();

		if (ok)
			m_connector = (CConnector *) pMem[1];

//printf("CMyDev::Open() pMem: %p, m_connector: %p\n", pMem, m_connector);

		int rc = shareMem.UnmapShm(pMem, shareMem.GetHandle());
		assert(rc == 0);

		if (!ok)
			return DEF_DEV_OPENERROR;
	}

	void* ret = m_connector->GetMine(pcfg->DevAddr, compare);
	if (ret == NULL)
		return DEF_DEV_OPENERROR;
	
	kprintf(2,8,1,"获取到socket devid:%s  ", pcfg->DevAddr);
	m_socketId = (int) ret;
//printf("tcpComm Open: %s -> %d\n", pcfg->DevAddr, m_socketId);

	return DEF_DEV_STATE_OPENED;
}

int CMyDev::Close()
{
	if (m_socketId < 0)
		return 1;

	close(m_socketId);

	m_socketId = -1;

	return 1;
}

int CMyDev::Read(uchar* buf, int len, int timeout)
{
	if (m_socketId < 0)
		return DEF_READDEV_STATEERR;

	if (buf == NULL || len <= 0)
		return DEF_READDEV_PARAERR;

	fd_set rset;
	FD_ZERO(&rset);

	FD_SET(m_socketId, &rset);

	struct timeval tv;
	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000)*1000;

	int ret = select(m_socketId + 1, &rset, NULL, NULL, &tv);

	if (ret == 0)
		return 0;
	if (ret < 0)
		return OnError();

	//ret > 0

	if (FD_ISSET(m_socketId, &rset) == 0)
		return 0;

	int num = recv(m_socketId, (char*)buf, len, 0);

	if (num == 0)
		return DEF_READDEV_ERROR;
	if (num < 0)
		return OnError();

	//num > 0

	return num;
}

int CMyDev::Write(uchar* buf, int len, int timeout)
{
	if (m_socketId < 0)
		return DEF_READDEV_STATEERR;

	if (buf == NULL || len <= 0)
		return DEF_READDEV_PARAERR;

	timeval tv;
	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000)*1000;

	fd_set wset;
	FD_ZERO(&wset);
	FD_SET(m_socketId, &wset);

	int ret = select(m_socketId + 1, NULL, &wset, NULL, &tv);

	if (ret == 0)
		return 0;
	if (ret < 0)
		return OnError();

	//ret > 0

	if( FD_ISSET( m_socketId, &wset )== 0 )
		return 0;

	int txn = send(m_socketId, (char*)buf, len, 0);

	if (txn < 0)
		return OnError();

	return txn;
}

int CMyDev::Check()
{
	return 0;
}

int CMyDev::OnError()
{
	int err = MyGetLastError();
	if (err == EINTR || err == EWOULDBLOCK)
		return 0;
	else
		return DEF_READDEV_ERROR;
}

//设备对象创建函数
DLLEXPORT
CDevCore* CreateDevDrive()
{
	//printf("tcpComm::CreateDevDrive...\n");
    return new CMyDev;
}