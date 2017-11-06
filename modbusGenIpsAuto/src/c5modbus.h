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

//��ȡ���ݿ�ʱ������ʶ���кţ���ʶ�ڼ���section

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
	int dit;//���
};
struct ycsection{
	sint32 addr;
	ycstruct ycs[256];
	char id[CHARLEN];
	int ycslen;
	int ycdatalen;
	int startindex;//putAYCʱ�Ŀ�ʼ���к�
	int method;
	int cbdatalenbit;
	char horl;
	char cbdatahorl;//��������ֵ�ߵ�λ
	int hasnan;//�Ƿ�����Чֵ
	int nankey;//��Чֵ
	int nanvalue;//��Чֵ������ֵ
};

struct yxstruct{
	char id[CHARLEN];
	char desc[CHARLEN];
	//int coe;
	//int length;
	char horl;
	
	int used;
	int dit;//���
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
	char cbdatahorl;//��������ֵ�ߵ�λ
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
	int dit;//���
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
	char cbdatahorl;//��������ֵ�ߵ�λ
	int hasnan;//�Ƿ�����Чֵ
	int nankey;//��Чֵ
	int nanvalue;//��Чֵ������ֵ
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

	void checkChange();//���bigid��bigname��version�仯

protected:
	virtual         ~CC5Modbus();

private:
	int m_ackDataLen;
	int m_lastSendTime;
	S_RAWCMD m_cmd;

	int m_timeOut;

public:

	int isSendBuf;//�Ƿ���ͣ���ͱ��ģ�1���ͣ�0������
	int nowBigid;//�ֹ�Լid,��c2���ݿ��ȡ
	int nowBigname;//���豸ע��id
	int nowVersion;//�ֹ�Լ�汾��
	int lastCheckChange;//�ϴμ��仯ʱ��

	void getC2RtuInfo();
	void getC5RtuInfo();

	int m_askFrameType;

	int m_CRCtype;//1 ˫�ֽ�crc16  0 ���ֽ�crc8

	int m_lastReqTime;//�ϴ�ȡ����ʱ��

	int m_lastHertTime;//�ϴ�����ʱ��
	int m_lastcbdataTime;//�ϴ�dtu�ر���ʱ��
	
	char m_devid[IDLEN];//�豸ע���
	char ghearthead[IDLEN];//������������
	char gregistehead[IDLEN];//ע�ᱨ��ͷ
	char gregistetail[IDLEN];//ע�ᱨ��β
	char gcbheartsucceed[IDLEN];//����ע��ɹ����ر���
	char gcbheartfail[IDLEN];//����ע��ʧ�ܷ��ر���

	uint8  gshellheadbuf[32];
	uint8  gshelltailbuf[32];

	int gisregiste;//�Ƿ���ע��
	int gisheartbeat;//�Ƿ�����������
	int grutInterval;//RTUʱ����
	int gcbtimeout;//���س�ʱʱ��
	int greqInterval;//����ʱ����
	char gregisterhorl;//�Ĵ�����ַ�ߵ�λ����h��λ��ǰ��l��λ��ǰ
	char gcheckhorl;//У����ߵ�λ����h��λ��ǰ��l��λ��ǰ
	int gheartbeatInterval;//�������ļ��
	int gcbheartbeat;//�������ķ���,�Ƿ��з�������
	int gdevonline;//�豸ips�Ƿ�����
	int gdtuonline;//dtu�Ƿ�����
	int gshellheadLen;//���ͷ���ݳ���
	int gshelltailLen;//���β���ݳ���

	int gcheckcrc;//�������Ƿ���crcУ��,ͬʱҲ��ʾ�˷��ر���У������
	int gcbdatalenbit;//���ر�������������ռλ��
	char gcbdatalenhorl;//���ر������ݳ��ȸߵ�λ����

	
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


	char gbigid[CHARLEN];//�ӹ�Լ�������ݿ��ȡ
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