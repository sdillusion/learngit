#ifndef	_C5_CONFIG_MODBUS_PROT_
#define	_C5_CONFIG_MODBUS_PROT_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

#include "c5db.h"
#include "comm/protocolbase.h"
#define DEBUG_MODE _DEBUG

#define CHARLEN 32
#define IDMAXLEN 32
const int IDLEN = 16;
const int SECTIONNUM = 32;

using namespace std;

//Frame type define
#define FRAME_ERR	-1
#define YC_FRAME	0
#define YX_FRAME	1
#define DD_FRAME	2
#define YK_RET		3
#define YT_RET		4
#define YC_CYCLIC	5
#define YX_CYCLIC	6
#define DD_CYCLIC	7

struct LOGSTRUCT{
	int init;
	int change;
	int modbus;
};

//读取数据库时用来标识序列号，标识第几个section

struct ycstruct{
	char id[CHARLEN];
	//char desc[CHARLEN];
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
	//ycstruct ycs[256];
	vector<ycstruct> ycs;
	char id[CHARLEN];
	int ycslen;
	int ycdatalen;
	int startindex;//putAYC时的开始序列号
	int method;
	int cbdatalenbit;
	char horl;
	char cbdatahorl;//返回数据值高低位
	int hasnan;//是否有无效值
	int nankey;//无效值
	int nanvalue;//无效值返回数值
};

struct yxstruct{
	char id[CHARLEN];
	//char desc[CHARLEN];
	//int coe;
	//int length;
	char horl;
	
	int used;
	int dit;//点号
	//int sort;//上报序号
};
struct yxsection{
	sint32 addr;
	//yxstruct yxs[256];
	vector<yxstruct> yxs;
	char id[CHARLEN];
	int yxslen;
	int yxdatalen;
	int startindex;
	int yxnum;
	int method;
	int offset;
	int cbdatalenbit;
	char horl;
	char cbdatahorl;//返回数据值高低位
};

struct ddstruct{
	char id[CHARLEN];
	//char desc[CHARLEN];
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
	//ddstruct dds[256];
	vector<ddstruct> dds;
	char id[CHARLEN];
	int ddslen;
	int dddatalen;
	int startindex;
	int reqnum;
	int method;
	int cbdatalenbit;
	char horl;
	char cbdatahorl;//返回数据值高低位
	int hasnan;//是否有无效值
	int nankey;//无效值
	int nanvalue;//无效值返回数值
};

class  CC5Modbus: public CProtocol
{
public:
	CC5Modbus(void);

	void ProcessDD(uint8* buf, int datalen);
	void ProcessYx(uint8* buf, int datalen);
	void ProcessYc(uint8* buf, int datalen);
	void ParseFrame();
	WORD CRC16( uint8 * packet, int len );
	WORD CRC8( uint8 * packet, int len );
	int GetNowSecond();
	void RequestData();
	void RequestDD();
	void RequestYx();
	void RequestYc();
	int MakeWord(char type, short high, short low);

	void getC2RtuInfo();

	bool C5DBycGroupQuery(char* bigid, C5DB c5db);
	bool C5DByxGroupQuery(char* bigid, C5DB c5db);
	bool C5DBddGroupQuery(char* bigid, C5DB c5db);
	bool C5DBycQuery(char* bigid, char* groupid, int ycssindex, C5DB c5db);
	bool C5DByxQuery(char* bigid, char* groupid, int yxssindex, C5DB c5db);
	bool C5DBddQuery(char* bigid, char* groupid, int ddssindex, C5DB c5db);
	bool C5DBbigQuery(int bigid, C5DB c5db);

	virtual sint32 TxProc();
	virtual sint32 RxProc();
	virtual sint32 GetZfFlag( ){return 0;};
	virtual void   Init( S_PROTOCOLCFG * pcfg );

	void   Init2();


	void interleavedInit();

	bool CheckHeartBeat();
	void SendHeartBeat();
	bool IsNeedToSendHeartBeat(int now);

	//bool CheckCbCrc();

	short int CalcYcShortVal(BYTE HighByte,BYTE LowByte);
	int makeBitYxVal(int len, char horl, unsigned char*buf);

	int addRegHead(uint8* buf,int buflen);

	void sendMSG(uint8* buf, int datalen);
	void setCommonConfig();
	int analysisBuffer(unsigned char* buf, int method, int index, char cbdatahorl);

	void ProcessBuf(uint8* buf, sint32 rtuno, int buflen, int datalen);

//	int analysisYc(unsigned char* buf, int index, char cbdatahorl);
//	int analysisYx(unsigned char* buf, int index, char cbdatahorl);
//	int analysisDd(unsigned char* buf, int index, char cbdatahorl);

	int Int2(unsigned char* buf, int index, char cbdatahorl);
	//int Int2H(unsigned char* buf, int index, char cbdatahorl);
	int Int4(unsigned char* buf, int index, char cbdatahorl);
	//int Int4H(unsigned char* buf, int index, char cbdatahorl);
	int UInt2(unsigned char* buf, int index, char cbdatahorl);
	//int UInt2H(unsigned char* buf, int index, char cbdatahorl);
	int UInt4(unsigned char* buf, int index, char cbdatahorl);
	//int UInt4H(unsigned char* buf, int index, char cbdatahorl);
	//float Float4L(unsigned char* buf, int index, char cbdatahorl);
	//float Float4H(unsigned char* buf, int index, char cbdatahorl);
	float Float4(unsigned char* buf, int index, char cbdatahorl);
	//float Float4_DD1L(unsigned char* buf, int index, char cbdatahorl);
	//float Float4_DD1H(unsigned char* buf, int index, char cbdatahorl);
	float Float4_DD1(unsigned char* buf, int index, char cbdatahorl);
	//int EX_INT8_PM800L(unsigned char* buf, int index, char cbdatahorl);
	//int EX_INT8_PM800H(unsigned char* buf, int index, char cbdatahorl);
	int EX_INT8_PM800(unsigned char* buf, int index, char cbdatahorl);
	int yxbit(unsigned char*buf, int index, char cbdatahorl);
	int yxchar(unsigned char* buf, int index, char cbdatahorl);

	int IpsYc(unsigned char* buf, int index, char cbdatahorl);
	int IpsYx(unsigned char* buf, int index, char cbdatahorl);
	int IpsDd(unsigned char* buf, int index, char cbdatahorl);

	int NewIpsYc(unsigned char* pData, int index, char cbdatahorl);

	void checkChange();//检查bigid、bigname、version变化
	void checkC5Change();//检查version变化
	void ChanDown();//通道下线

	

protected:
	virtual         ~CC5Modbus();

private:
	int m_ackDataLen;
	int m_lastSendTime;
	S_RAWCMD m_cmd;

	int m_timeOut;

public:
	bool m_isInited;//是否已经初始化
	int m_systemInitTime;//系统初始化时间

	//int m_isSendBuf;//是否暂停发送报文，1发送，0不发送
	int m_nowBigid;//现规约id,从c2数据库读取
	int m_nowBigname;//现设备注册id
	char m_nowBigName[CHARLEN];//现设备注册id
	int m_nowVersion;//现规约版本号
	int m_lastCheckChange;//上次检查变化时间

	int m_askFrameType;

	int m_CRCtype;//1 双字节crc16  0 单字节crc8

	int m_lastReqTime;//上次取报文时间

	int m_lastHertTime;//上次心跳时间
	int m_lastcbdataTime;//上次dtu回报文时间
	
	//char m_devid[IDLEN];//设备注册号
	char m_ghearthead[IDLEN];//心跳报文内容
	char m_gregistehead[IDLEN];//注册报文头
	char m_gregistetail[IDLEN];//注册报文尾
	char m_gcbheartsucceed[IDLEN];//心跳注册成功返回报文
	char m_gcbheartfail[IDLEN];//心跳注册失败返回报文

	//uint8  m_gshellheadbuf[32];
	//uint8  m_gshelltailbuf[32];

	int m_gisregiste;//是否有注册
	int m_gisheartbeat;//是否有心跳报文
	//int m_grutInterval;//RTU时间间隔
	//int m_gcbtimeout;//返回超时时间
	int m_greqInterval;//请求时间间隔
	char m_gregisterhorl;//寄存器地址高低位规则：h高位在前、l低位在前
	char m_gcheckhorl;//校验码高低位规则：h高位在前、l低位在前
	int m_gheartbeatInterval;//心跳报文间隔
	int m_gcbheartbeat;//心跳报文反馈,是否有反馈回来
	int m_gdevonline;//设备ips是否在线
	int m_gdtuonline;//dtu是否在线
	//int m_gshellheadLen;//外壳头内容长度
	//int m_gshelltailLen;//外壳尾内容长度

	int m_gcheckcrc;//请求报文是否有crc校验,同时也表示了返回报文校验类型
	//int m_gcbdatalenbit;//返回报文数据数量所占位数
	//char m_gcbdatalenhorl;//返回报文数据长度高低位规则

	
	ycsection m_ycss[SECTIONNUM];
	yxsection m_yxss[SECTIONNUM];
	ddsection m_ddss[SECTIONNUM];

	int m_ycssindex;
	int m_yxssindex;
	int m_ddssindex;

	int m_ycsectionNum;
	int m_ddsectionNum;
	int m_yxsectionNum;


	char m_gbigid[CHARLEN];//从规约配置数据库读取
	char m_gbigname[CHARLEN];

	int m_gcuryc;
	int m_gcuryx;
	int m_gcurdd;

	int m_iResult;
	float m_fResult;

	int m_run;//规约运行状态
	int m_hasChanged;//c2和C5是否已改变

	int m_firstSendHeart;//1第一次心跳已发送，0没发送

	int m_lastInitTime;

	int m_constructorTime;

	sqlite3 * m_pDB;
	C5DB m_c5db;

	LOGSTRUCT m_log;
};
	static int str_to_hex(char *string, unsigned char *cbuf, int len);
	float sixteen2float(uint8* buf,int len, char horl);
	int sixteen2int(uint8* buf,int len, char horl);
	int sixteen2uint(uint8* buf,int len, char horl);


	

#endif