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

//��ȡ���ݿ�ʱ������ʶ���кţ���ʶ�ڼ���section

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
	int dit;//���
};
struct ycsection{
	sint32 addr;
	//ycstruct ycs[256];
	vector<ycstruct> ycs;
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
	//char desc[CHARLEN];
	//int coe;
	//int length;
	char horl;
	
	int used;
	int dit;//���
	//int sort;//�ϱ����
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
	char cbdatahorl;//��������ֵ�ߵ�λ
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
	int dit;//���
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

	void checkChange();//���bigid��bigname��version�仯
	void checkC5Change();//���version�仯
	void ChanDown();//ͨ������

	

protected:
	virtual         ~CC5Modbus();

private:
	int m_ackDataLen;
	int m_lastSendTime;
	S_RAWCMD m_cmd;

	int m_timeOut;

public:
	bool m_isInited;//�Ƿ��Ѿ���ʼ��
	int m_systemInitTime;//ϵͳ��ʼ��ʱ��

	//int m_isSendBuf;//�Ƿ���ͣ���ͱ��ģ�1���ͣ�0������
	int m_nowBigid;//�ֹ�Լid,��c2���ݿ��ȡ
	int m_nowBigname;//���豸ע��id
	char m_nowBigName[CHARLEN];//���豸ע��id
	int m_nowVersion;//�ֹ�Լ�汾��
	int m_lastCheckChange;//�ϴμ��仯ʱ��

	int m_askFrameType;

	int m_CRCtype;//1 ˫�ֽ�crc16  0 ���ֽ�crc8

	int m_lastReqTime;//�ϴ�ȡ����ʱ��

	int m_lastHertTime;//�ϴ�����ʱ��
	int m_lastcbdataTime;//�ϴ�dtu�ر���ʱ��
	
	//char m_devid[IDLEN];//�豸ע���
	char m_ghearthead[IDLEN];//������������
	char m_gregistehead[IDLEN];//ע�ᱨ��ͷ
	char m_gregistetail[IDLEN];//ע�ᱨ��β
	char m_gcbheartsucceed[IDLEN];//����ע��ɹ����ر���
	char m_gcbheartfail[IDLEN];//����ע��ʧ�ܷ��ر���

	//uint8  m_gshellheadbuf[32];
	//uint8  m_gshelltailbuf[32];

	int m_gisregiste;//�Ƿ���ע��
	int m_gisheartbeat;//�Ƿ�����������
	//int m_grutInterval;//RTUʱ����
	//int m_gcbtimeout;//���س�ʱʱ��
	int m_greqInterval;//����ʱ����
	char m_gregisterhorl;//�Ĵ�����ַ�ߵ�λ����h��λ��ǰ��l��λ��ǰ
	char m_gcheckhorl;//У����ߵ�λ����h��λ��ǰ��l��λ��ǰ
	int m_gheartbeatInterval;//�������ļ��
	int m_gcbheartbeat;//�������ķ���,�Ƿ��з�������
	int m_gdevonline;//�豸ips�Ƿ�����
	int m_gdtuonline;//dtu�Ƿ�����
	//int m_gshellheadLen;//���ͷ���ݳ���
	//int m_gshelltailLen;//���β���ݳ���

	int m_gcheckcrc;//�������Ƿ���crcУ��,ͬʱҲ��ʾ�˷��ر���У������
	//int m_gcbdatalenbit;//���ر�������������ռλ��
	//char m_gcbdatalenhorl;//���ر������ݳ��ȸߵ�λ����

	
	ycsection m_ycss[SECTIONNUM];
	yxsection m_yxss[SECTIONNUM];
	ddsection m_ddss[SECTIONNUM];

	int m_ycssindex;
	int m_yxssindex;
	int m_ddssindex;

	int m_ycsectionNum;
	int m_ddsectionNum;
	int m_yxsectionNum;


	char m_gbigid[CHARLEN];//�ӹ�Լ�������ݿ��ȡ
	char m_gbigname[CHARLEN];

	int m_gcuryc;
	int m_gcuryx;
	int m_gcurdd;

	int m_iResult;
	float m_fResult;

	int m_run;//��Լ����״̬
	int m_hasChanged;//c2��C5�Ƿ��Ѹı�

	int m_firstSendHeart;//1��һ�������ѷ��ͣ�0û����

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