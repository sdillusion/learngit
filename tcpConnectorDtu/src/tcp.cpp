#include <assert.h>
#include "tcp.h"
#include "plat_comm.h"
//#include <string>

#ifdef __unix
#include <netdb.h>
#include <sys/socket.h>        /* for AF_INET */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#ifdef WIN32
#include <process.h>

//using namespace std;

#define close	closesocket
#define getpid	_getpid
#endif

const int share_mem_id = 5239105;
const int share_mem_size = 4096;

const int BUFLEN = 512;

CMyConnector::CMyConnector()
{   
	m_bInitialized = false;
	m_socketId = -1;

	int fCreate;
	CShareMem shareMem;
	void ** pMem = (void **) shareMem.CreateAndMapping(share_mem_id, share_mem_size, &fCreate);
	assert (pMem != NULL);

	pMem[0] = (void *) getpid();
	pMem[1] = (void *) this;

	m_lock.init();

	strcpy(m_inipath,"..\\cfg\\registerCfg.ini");

	getHeadAndTailFromFile();
}

CMyConnector::~CMyConnector()
{
	Clean();

	m_lock.end();
}

void* CMyConnector::GetMine(char* para, FUNC_COMPAREPARA* func)
{
	if (para == NULL || func == NULL)
		return NULL;

	if (m_list.size() <= 0)
		return NULL;

	m_lock.lock();

	std::list<ConnectInfo>::iterator i = m_list.begin();
	for (; i != m_list.end(); ++i)
	{
		
		if (func(para, i->devid, i->isonline))
		{
			int socketId = i->socketId;
			m_list.erase(i);
			m_lock.unlock();
			return (void *) socketId;
		}
	}

	m_lock.unlock();

	return NULL;
}

bool CMyConnector::Initialize(char* para)
{
	if (!ParsePara(para))
		return false;

	if (!CreateSocket())
		return false;
				
	for (int i = 0; i < 2; i++) {

		if (Listen()) 
		{
			m_bInitialized = true;
			return true;
		}
		
		JskMicroSleep(1000000);
	}
	
	return false;
}

void CMyConnector::Clean()
{
	m_bInitialized = false;

	if (m_socketId < 0)
		return;

	close(m_socketId);

	m_socketId = -1;
}

bool CMyConnector::IsInitialized()
{
	return m_bInitialized;
}

void CMyConnector::Run()
{
	if (!m_bInitialized)
		return;

	checkList();

	int socketId;
	sockaddr_in peer;
	
	if (!Accept(socketId, peer))
		return;

	SetSocketUnblocked(socketId);

	sockaddr_in sin;
	memcpy(&sin, &peer, sizeof(sin));

	ConnectInfo info;	
	strcpy(info.peerIp, inet_ntoa(sin.sin_addr));
	info.peerPort = sin.sin_port;
	info.socketId = socketId;
	info.isonline = false;

	m_list.push_back(info);
	kprintf(2,8,1,"客户端ip = %s,端口 = %d  ",  info.peerIp, info.peerPort);
	checkList();
	//printf("New client: %s - %d\n", info.peerIp, ntohs(info.peerPort));
}

void CMyConnector::checkList(){
	std::list<ConnectInfo>::iterator i = m_list.begin();
	for (;i != m_list.end(); ++i)
	{
		char buf[BUFLEN];
		char devid[BUFLEN];
		int socketId = i->socketId;
		int num = recv(socketId, (char*)buf, 512, 0);
		int now = GetNowSecond();
		if(num>0){
			if(ParseBuf(buf,strlen(buf),devid)){
				strcpy(i->devid, devid);
				i->time = now;
				i->isonline = true;
				kprintf(2,8,1,"注册成功ip = %s,端口 =%d  ",  i->peerIp, i->peerPort);
			}
		}
		if(now - i->time > 30){
			kprintf(2,8,1,"超时ip = %s,端口 =%d  ",  i->peerIp, i->peerPort);
			m_list.erase(i);
			
			if(m_list.size() == 0){
				break;
			}
			i--;
		}
	}
}

bool CMyConnector::ParseBuf(char* buf,int len, char*result){
	char* headAddr = strstr(buf, (char*)(m_bufHead));
	int headIndex = headAddr ? buf - headAddr : -1;
	int headlen = strlen((char*)m_bufHead);
	buf += headlen;
	char* tailAddr = strstr(buf, (char*)(m_bufTail));
	int tailIndex = tailAddr ? tailAddr - buf : -1;
	if(tailIndex && headIndex >= 0 && tailIndex < BUFLEN){
		strncpy(result, buf, tailIndex); 
		result[tailIndex] = '\0';
		return true;
	}
	
	return false;
}


bool CMyConnector::ParsePara(char* para)
{
	char tmp [256];
	
	assert (para);
	assert (sizeof(tmp) > strlen(para));
	strcpy(tmp, para);

	char* p = strchr(tmp, ':');
	if (p == NULL)
		return false;

	*p = 0;
	strcpy(m_ip, tmp);

	p++;	
	if (strlen(p) <= 0)
		return false;

	m_port = atoi(p);
	if (m_port <= 0)
		return false;

	return true;
}

bool CMyConnector::CreateSocket()
{	
	m_socketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socketId < 0)
		return false;

	int temp = 150*1000;
	setsockopt(m_socketId, SOL_SOCKET, SO_RCVBUF, (char*)&temp, sizeof(temp));
	setsockopt(m_socketId, SOL_SOCKET, SO_SNDBUF, (char*)&temp, sizeof(temp));
	
	temp = 1;
	setsockopt(m_socketId, SOL_SOCKET, SO_REUSEADDR, (char*)&temp, sizeof(temp));
    
	SetSocketUnblocked(m_socketId);

	return true;
}

bool CMyConnector::Listen()
{
	in_addr addr;
	addr.s_addr = inet_addr(m_ip);
		
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(m_port);
	sin.sin_addr = addr;

	if (bind(m_socketId, (struct sockaddr*)&sin, sizeof(sin)) != 0)
	{
		Clean();
		return false;
	}

	if (listen(m_socketId, SOMAXCONN) != 0)
	{
		Clean();
		return false;
	}

	return true;
}

bool CMyConnector::Accept(int& socketId, struct sockaddr_in& peer)
{
	fd_set rset;
	FD_ZERO(&rset);

	FD_SET(m_socketId, &rset);	

	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if (select(m_socketId + 1, &rset, NULL, NULL, &tv) <= 0)
		return false;

	#ifdef __unix
		socklen_t addrlen;
	#else 
		int addrlen;
	#endif
		
	addrlen = sizeof(peer);

	socketId = accept(m_socketId, (sockaddr*)&peer, &addrlen);
	if (socketId < 0)
		return false;

	return true;
}

void CMyConnector::SetSocketUnblocked(int socketId)
{
	#if defined(__unix)

        fcntl(socketId, F_SETFL, O_NONBLOCK);
        sigset(SIGPIPE, SIG_IGN);

    #elif defined(WIN32)

        unsigned long block_flag = 0;
        ioctlsocket(socketId, FIONBIO, &block_flag);
		
    #endif
}

int CMyConnector::GetNowSecond()
{
	CJskTime     JSK_T;
	TCriterionTime tmptime;
	JSK_T.GetNow(&tmptime);
	return (int)tmptime;
}

/**************************************************************************** 
函数名称: str_to_hex 
函数功能: 字符串转换为十六进制 
输入参数: string 字符串 cbuf 十六进制 len 字符串的长度。 
输出参数: 无 
*****************************************************************************/   
int CMyConnector::str_to_hex(char *string, unsigned char *cbuf, int len)  
{  
    BYTE high, low;  
    int idx, ii=0;  
    for (idx=0; idx<len; idx+=2)   
    {  
        high = string[idx];  
        low = string[idx+1];  
          
        if(high>='0' && high<='9')  
            high = high-'0';  
        else if(high>='A' && high<='F')  
            high = high - 'A' + 10;  
        else if(high>='a' && high<='f')  
            high = high - 'a' + 10;  
        else  
            return -1;  
          
        if(low>='0' && low<='9')  
            low = low-'0';  
        else if(low>='A' && low<='F')  
            low = low - 'A' + 10;  
        else if(low>='a' && low<='f')  
            low = low - 'a' + 10;  
        else  
            return -1;  
          
        cbuf[ii++] = high<<4 | low;  
    }  
    return 0;  
}

int CMyConnector::char2x(char c){
	if(c-'0'<10){
		return c-'0';
	}else{
		return c-'A'+10;
	}
	return -1;
}

void CMyConnector::getHeadAndTailFromFile(){
	int methodnum = ::GetPrivateProfileInt("para","num",-1,m_inipath);
	if(methodnum == -1){
		kprintf(2,8,1,"读取registerCfg.ini文件失败");
		return;
	}
	for(int i =0; i < methodnum; i++){
		char intstr[8];
		itoa(i, intstr, 10);
		
		CString head;
		::GetPrivateProfileString(intstr,"head","Error",head.GetBuffer(20),20,m_inipath);
		head.MakeUpper();
		CString tail;
		::GetPrivateProfileString(intstr,"tail","Error",tail.GetBuffer(20),20,m_inipath);
		tail.MakeUpper();
		if(head != "Error" && tail != "Error"){
			head.ReleaseBuffer();
			int headL = head.GetLength();
			tail.ReleaseBuffer();
			int tailL = tail.GetLength();
			if(headL%2 || tailL%2){
				kprintf(2,8,1,"registerCfg.ini文件配置错误");
				return;
			}
			int i = 0;
			int l = headL/2;
			char stem[16];
			strcpy(stem, head.GetBuffer(0));
			for(i = 0; i < l; i++){
				
				m_bufHead[i] = char2x(stem[2*i]) *16 + char2x(stem[2*i+1]);
			}
			m_bufHead[i] = 0;

			i=0;
			l=tailL/2;
			strcpy(stem, tail.GetBuffer(0));
			for(i = 0; i < l; i++){
				m_bufTail[i] = char2x(stem[2*i]) *16 + char2x(stem[2*i+1]);
			}
			m_bufTail[i] = 0;
		}else{
			kprintf(2,8,1,"读取registerCfg.ini文件内容失败");
			return;
		}
	}
}
