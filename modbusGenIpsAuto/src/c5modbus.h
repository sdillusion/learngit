#ifndef	_C5_CONFIG_MODBUS_PROT_
#define	_C5_CONFIG_MODBUS_PROT_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

//读取数据库时用来标识序列号，标识第几个section

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
	ycstruct ycs[256];
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
	char desc[CHARLEN];
	//int coe;
	//int length;
	char horl;
	
	int used;
	int dit;//点号
};
struct yxsection{
	sint32 addr;
	yxstruct yxs[256];
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
	ddstruct dds[256];
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
	int calculatePoint(short* valArray, int i);
	int MakeWord(char type, short high, short low);

	std::string bigmakeSqlStr(int type);
	std::string ycmakeSqlStr(char* type);
	std::string yxmakeSqlStr(char* type);
	std::string ddmakeSqlStr(char* type);
	std::string ytmakeSqlStr(char* type);
	std::string ykmakeSqlStr(char* type);

	bool bigquery(const std::string& sqlStr);
	bool ycquery(const std::string& sqlStr);
	bool yxquery(const std::string& sqlStr);
	bool ddquery(const std::string& sqlStr);
	bool ytquery(const std::string& sqlStr);
	bool ykquery(const std::string& sqlStr);

	void bigResultFormat(int nrownum, int ncolnum, char **azResult);
	void ycResultFormat(int nrownum, int ncolnum, char **azResult);
	void yxResultFormat(int nrownum, int ncolnum, char **azResult);
	void ddResultFormat(int nrownum, int ncolnum, char **azResult);
	void ytResultFormat(int nrownum, int ncolnum, char **azResult);
	void ykResultFormat(int nrownum, int ncolnum, char **azResult);


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

	bool CheckHeartBeat();
	void SendHeartBeat();
	bool IsNeedToSendHeartBeat(int now);

	bool CheckCbCrc();

	short int CalcYcShortVal(BYTE HighByte,BYTE LowByte);
	int makeBitYxVal(int len, char horl, unsigned char*buf);

	int addRegHead(uint8* buf,int buflen);

	void sendMSG(uint8* buf, int datalen);
	void setCommonConfig();
	int analysisBuffer(unsigned char* buf, int method, int index, char cbdatahorl);

	void ProcessBuf(uint8* buf, sint32 rtuno, int buflen, int datalen);

	int analysisYc(unsigned char* buf, int index, char cbdatahorl);
	int analysisYx(unsigned char* buf, int index, char cbdatahorl);
	int analysisDd(unsigned char* buf, int index, char cbdatahorl);

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
	int yx4bit(unsigned char*buf, int index, char cbdatahorl);
	int yx4char(unsigned char* buf, int index, char cbdatahorl);

	int IpsYc(unsigned char* buf, int index, char cbdatahorl);
	int IpsYx(unsigned char* buf, int index, char cbdatahorl);
	int IpsDd(unsigned char* buf, int index, char cbdatahorl);

	int NewIpsYc(unsigned char* pData, int index, char cbdatahorl);

	void checkChange();//检查bigid、bigname、version变化

protected:
	virtual         ~CC5Modbus();

private:
	int m_ackDataLen;
	int m_lastSendTime;
	S_RAWCMD m_cmd;

	int m_timeOut;

public:

	int isSendBuf;//是否暂停发送报文，1发送，0不发送
	int nowBigid;//现规约id,从c2数据库读取
	int nowBigname;//现设备注册id
	int nowVersion;//现规约版本号
	int lastCheckChange;//上次检查变化时间

	void getC2RtuInfo();
	void getC5RtuInfo();

	int m_askFrameType;

	int m_CRCtype;//1 双字节crc16  0 单字节crc8

	int m_lastReqTime;//上次取报文时间

	int m_lastHertTime;//上次心跳时间
	int m_lastcbdataTime;//上次dtu回报文时间
	
	char m_devid[IDLEN];//设备注册号
	char ghearthead[IDLEN];//心跳报文内容
	char gregistehead[IDLEN];//注册报文头
	char gregistetail[IDLEN];//注册报文尾
	char gcbheartsucceed[IDLEN];//心跳注册成功返回报文
	char gcbheartfail[IDLEN];//心跳注册失败返回报文

	uint8  gshellheadbuf[32];
	uint8  gshelltailbuf[32];

	int gisregiste;//是否有注册
	int gisheartbeat;//是否有心跳报文
	int grutInterval;//RTU时间间隔
	int gcbtimeout;//返回超时时间
	int greqInterval;//请求时间间隔
	char gregisterhorl;//寄存器地址高低位规则：h高位在前、l低位在前
	char gcheckhorl;//校验码高低位规则：h高位在前、l低位在前
	int gheartbeatInterval;//心跳报文间隔
	int gcbheartbeat;//心跳报文反馈,是否有反馈回来
	int gdevonline;//设备ips是否在线
	int gdtuonline;//dtu是否在线
	int gshellheadLen;//外壳头内容长度
	int gshelltailLen;//外壳尾内容长度

	int gcheckcrc;//请求报文是否有crc校验,同时也表示了返回报文校验类型
	int gcbdatalenbit;//返回报文数据数量所占位数
	char gcbdatalenhorl;//返回报文数据长度高低位规则

	
	ycsection m_ycss[SECTIONNUM];
	yxsection m_yxss[SECTIONNUM];
	ddsection m_ddss[SECTIONNUM];

	int ycssindex;
	int yxssindex;
	int ddssindex;

	int ycsindex;
	int yxsindex;
	int ddsindex;

	int m_ycsectionNum;
	int m_ddsectionNum;
	int m_yxsectionNum;


	char gbigid[CHARLEN];//从规约配置数据库读取
	char gbigname[CHARLEN];

	int gcuryc;
	int gcuryx;
	int gcurdd;

	int iResult;
	float fResult;

	sqlite3 * pDB;
	C5DB m_c5db;

};
	static int str_to_hex(char *string, unsigned char *cbuf, int len);
	float sixteen2float(uint8* buf,int len, char horl);
	int sixteen2int(uint8* buf,int len, char horl);
	int sixteen2uint(uint8* buf,int len, char horl);


	

#endif