//modbusͨ�ù�Լ
//Author:xxk
//date:2017-07
//

#pragma warning(disable : 4786)

#include <math.h>
#include <string>
#include "plat_log.h"
#include "plat_comm.h"
#include "./sqlite3.h"
#include "c5modbus.h"

#define DCV_LOG_MODBUS 8
//const int CHECKCHANGEINTERVAL = 120;//��λ:��  ���仯��ʱ������ͬʱҲ�Ƿ����������ĵ�ʱ��
//const int DTUOFFLINETIME = 60;//��λ����  dtu�豸�ڴ�ʱ���ڲ����ر�������Ϊdtu�豸������

DLLEXPORT CProtocol* CreateProtocol(char *defpara)
{
	//_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	//_CrtSetBreakAlloc(243);
	
	CProtocol *pEpv = new CC5Modbus;
	return pEpv;
}

///////-----------------------------------------
WORD
CC5Modbus::CRC16( uint8 * packet, int len )
{
	sint32 i, j;
	uint16 temp,wCRC = 0xffff;

	for ( i = 0; i < len; i++ )
	{
		wCRC ^= packet[i];
		for ( j=1; j<9; j++ )
		{
			if ((wCRC & 1) == 1 )
				wCRC = (wCRC >> 1) ^ 0xa001;
			else
				wCRC = wCRC >> 1;
		}
	}

	temp = wCRC;
	wCRC = ( (temp & 0x00ff) << 8 ) + ( (temp & 0xff00) >> 8 );
	return wCRC;
}

uint16
CC5Modbus::CRC8( uint8 * packet, int len )
{
	unsigned char i; 
    unsigned char crc=0x00; /* ����ĳ�ʼcrcֵ */ 

    while(len--)
    {
        crc ^= *packet++;  /* ÿ��������Ҫ������������,������ָ����һ���� */  
        for (i=8; i>0; --i)   /* ������μ�����������һ���ֽ�crcһ�� */  
        { 
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc = (crc << 1);
        }
    }

    return (crc); 
}

CC5Modbus::CC5Modbus()
{

//#ifdef DEBUG_MODE
	//char* tc = "test";
//	kprintf(LOG_COMM,
//			DCV_LOG_MODBUS,
//			LOG_INFORMATION,
//			"��Լ����,%s",tc);
//#endif

	pDB = NULL;

	m_ackDataLen = 0;
	m_askFrameType = FRAME_ERR;

	
}

CC5Modbus::~CC5Modbus()
{
	
}


//
void CC5Modbus::Init(S_PROTOCOLCFG * pcfg )
{
	#ifdef DEBUG_MODE
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_INFORMATION,
			"��Լ��ʼ����ʼ ");
	#endif
	PRawCtrl	=	pcfg->PRawCtrl;
	pRxBuf		=	pcfg->pRxBuf;
	pTxBuf		=	pcfg->pTxBuf;
	pCmdMem		=	pcfg->pCmdMem;
	pRtu		=	pcfg->pRtu;
	pLink		=	pcfg->pLink;
	pTable		=	pcfg->pTable;
	pHis		=	pcfg->pHis;
	pPara		=	pcfg->pPara;
	m_timeOut = pLink->GetRxTimeouts();
	nowBigid = pLink->GetAllDataScanInterval();
	greqInterval = pLink->GetKwhScanInterval();
	m_lastReqTime = m_lastSendTime = GetNowSecond();

	if(m_timeOut < 1 || m_timeOut > 1000)
		m_timeOut = 3;

	char szapipath[MAX_PATH] = {0};//

	sint32 rtuno = pLink->GetRtuNo();

	ycssindex = -1;
	yxssindex = -1;
	ddssindex = -1;
	
	setCommonConfig();

	char dbBasePath[128];
	char dbPath[128];
	sprintf(dbBasePath, "./%s.db", pPara);
	sprintf(dbPath, "./%s_%d.db", pPara, rtuno);
	CopyFile(dbBasePath, dbPath, false);


	//��·������utf-8����
    //���·���а������ģ���Ҫ���б���ת��
	int nRes = sqlite3_open(dbPath, &pDB);
	if (nRes != SQLITE_OK)
	{
	  kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"�����ݿ�%sʧ��,",dbPath);
	  return;
	}

	if(bigquery(nowBigid)){
		ycGroupQuery(gbigid);
		yxGroupQuery(gbigid);
		ddGroupQuery(gbigid);
		int i,l;
		for(i = 0,l=ycssindex; i <= l ; i++){
			if(!ycQuery(gbigid,m_ycss[i].id,i)){
				kprintf(LOG_COMM,
					DCV_LOG_MODBUS,
					LOG_ERROR,
					"��Լ����yc��ȡʧ�� rtu:%d", rtuno);
			}
		}
		for(i = 0,l=yxssindex; i <= l ; i++){
			yxQuery(gbigid,m_yxss[i].id,i);
		}
		for(i = 0,l=ddssindex; i <= l ; i++){
			ddQuery(gbigid,m_ddss[i].id,i);
		}
	}else{
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"��Լ���ö�ȡʧ�� rtu:%d", rtuno);
	}
	sqlite3_close(pDB);
	remove(dbPath);

	m_askFrameType = YC_CYCLIC;

	gcuryc = 0;
	gcuryx = 0;
	gcurdd = 0;

	#ifdef DEBUG_MODE
	kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_INFORMATION,
			"��Լ��ʼ������ rtu:%d", rtuno);
	#endif
}

//
sint32 CC5Modbus::RxProc()
{
	ParseFrame();
	//_CrtDumpMemoryLeaks();
	return 1;
}

//
sint32 CC5Modbus::TxProc()
{
	int nowtime;
	nowtime = GetNowSecond();
	
	
	E_RAW_ComStat coms = pLink->GetCommStatus();
	if( coms != CMST_TX_CNT && coms != CMST_NORMAL){
		return 0;
	}

	if(nowtime - m_lastReqTime < greqInterval){
		return 0;
	}

	RequestData();
	
	return 1;
}


void CC5Modbus::RequestData()
{
	switch (m_askFrameType)
	{
	case YC_CYCLIC:	
		if(gcuryc >= m_ycsectionNum){
			gcuryc=0;
			m_askFrameType = YX_CYCLIC; RequestYx(); break;
		}
		RequestYc();
		break;
		
	case YX_CYCLIC:	
		if(gcuryx >= m_yxsectionNum){
			gcuryx=0;
			m_askFrameType = DD_CYCLIC; RequestDD(); break;
		}
		RequestYx();
		break;
	case DD_CYCLIC:	
		if(gcurdd >= m_ddsectionNum){
			gcurdd=0;
			m_askFrameType = YC_CYCLIC; RequestYc(); break;
		}
		RequestDD();
		break;
	case FRAME_ERR:	RequestYc(); break;
	case YC_FRAME:	RequestYc(); break;
	case YX_FRAME:	RequestYx(); break;
	case DD_FRAME:	RequestDD(); break;
	}
}

void CC5Modbus::RequestYc()
{
	if(m_ycsectionNum <= 0){
		return;
	}
	if(gcuryc >= m_ycsectionNum){
		gcuryc = 0;
	}
	sint32 rtuno = pLink->GetRtuNo();
	sint32 rtuadd = pRtu->GetRtuAddr(rtuno);

	uint16  wCRC=0;
	uint16  ycnum = 0;
	ycsection ycsec = m_ycss[gcuryc];
	
	for(int i = 0,l = ycsec.ycslen; i < l; i++){
		ycstruct ycstr = ycsec.ycs[i];
		ycnum += ycstr.length;
	}
	m_ycss[gcuryc].ycdatalen = ycnum;
	ycnum = (ycnum - 1)/2 +1;

	if(ycnum <= 0 || ycnum > 0x1000)
		return;

	uint8 buf[64];
	buf[0] = rtuadd;
	buf[1] = ycsec.funcode;
	if(gregisterhorl == 'l'){
		buf[3] = ycsec.addr/256;
		buf[2] = ycsec.addr%256;
	}else{
		buf[2] = ycsec.addr/256;
		buf[3] = ycsec.addr%256;
	}
	if(ycsec.ycs[0].horl == 'h'){//Ĭ�ϵ�λ��ǰ������'h'����Ϊ��λ��ǰ�������ߵ�λ��������
		buf[4] = ycnum/256;//HIBYTE(ycnum);
		buf[5] = ycnum%256;//LOBYTE(ycnum);
	}else{
		buf[5] = ycnum/256;//HIBYTE(ycnum);
		buf[4] = ycnum%256;//LOBYTE(ycnum);
	}
	if(m_CRCtype){
		wCRC = CRC16(buf,6);
		if(gcheckhorl == 'h'){
			buf[6] = wCRC/256;//HIBYTE(wCRC);
			buf[7] = wCRC%256;//LOBYTE(wCRC);
		}else{
			buf[7] = wCRC/256;//HIBYTE(wCRC);
			buf[6] = wCRC%256;//LOBYTE(wCRC);
		}
	}else{
		buf[6] = CRC8(buf,6);
	}
	int dataLen = m_CRCtype?8:7;
	int headAndTailLen = m_CRCtype ? 4 : 3;

	sendMSG(buf,dataLen);

	m_ackDataLen = m_ycss[gcuryc].ycdatalen + headAndTailLen + m_ycss[gcuryc].cbdatalenbit;
	gcuryc++;

	m_lastReqTime = GetNowSecond();
	return;
}

void CC5Modbus::RequestYx()
{
	if(m_yxsectionNum <= 0){
		return;
	}
	if(gcuryx >= m_yxsectionNum){
		gcuryx = 0;
	}
	sint32 rtuno = pLink->GetRtuNo();
	sint32 rtuadd = pRtu->GetRtuAddr(rtuno);

	uint16  wCRC=0;
	uint16  yxnum	= 0;
	yxsection yxsec = m_yxss[gcuryx];
	yxnum = yxsec.yxnum ? yxsec.yxnum : yxsec.yxslen;
	m_yxss[gcuryx].yxdatalen = yxnum;

	if(yxnum <= 0 || yxnum > 0x1000)
		return;

	uint8	buf[64];
	buf[0] = rtuadd;
	buf[1] = yxsec.funcode;
	if(gregisterhorl == 'l'){
		buf[3] = yxsec.addr/256;
		buf[2] = yxsec.addr%256;
	}else{
		buf[2] = yxsec.addr/256;
		buf[3] = yxsec.addr%256;
	}
	if(yxsec.yxs[0].horl == 'h'){
		buf[4] = yxnum/256;//HIBYTE(yxnum);
		buf[5] = yxnum%256;//LOBYTE(yxnum);
	}else{
		buf[5] = yxnum/256;//HIBYTE(yxnum);
		buf[4] = yxnum%256;//LOBYTE(yxnum);
	}
	if(m_CRCtype) {
		wCRC = CRC16(buf,6);
		if(gcheckhorl == 'h'){
			buf[6] = wCRC/256;//HIBYTE(wCRC);
			buf[7] = wCRC%256;//LOBYTE(wCRC);
		}else{
			buf[7] = wCRC/256;//HIBYTE(wCRC);
			buf[6] = wCRC%256;//LOBYTE(wCRC);
		}
	}else{
		buf[6] = CRC8(buf,6);
	}
	int dataLen = m_CRCtype?8:7;

	sendMSG(buf,dataLen);

	int headAndTailLen = m_CRCtype ? 4 : 3;
	
	m_ackDataLen = m_yxss[gcuryx].yxdatalen + headAndTailLen + m_yxss[gcuryx].cbdatalenbit;

	gcuryx++;
	m_lastReqTime = GetNowSecond();
	return;
}

void CC5Modbus::RequestDD(){
	if(m_ddsectionNum <= 0){
		return;
	}
	if(gcurdd >= m_ddsectionNum){
		gcurdd = 0;
	}
	sint32 rtuno = pLink->GetRtuNo();
	sint32 rtuadd = pRtu->GetRtuAddr(rtuno);

	uint16  wCRC=0;
	uint16  ddnum = 0;
	ddsection ddsec = m_ddss[gcurdd];
	for(int i = 0,l = ddsec.ddslen; i < l; i++){
		ddstruct ddstr = ddsec.dds[i];
		ddnum += ddstr.length;
	}
	m_ddss[gcurdd].dddatalen = ddnum;
	ddnum = ddnum / 2;

	if(ddnum <= 0 || ddnum > 0x1000)
		return;

	uint8 buf[64];
	buf[0] = rtuadd;
	buf[1] = ddsec.funcode;
	if(gregisterhorl == 'l'){
		buf[3] = ddsec.addr/256;
		buf[2] = ddsec.addr%256;
	}else{
		buf[2] = ddsec.addr/256;
		buf[3] = ddsec.addr%256;
	}
	if(ddsec.dds[0].horl == 'h'){
		buf[4] = ddnum/256;//HIBYTE(ddnum);
		buf[5] = ddnum%256;//LOBYTE(ddnum);
	}else{
		buf[5] = ddnum/256;//HIBYTE(ddnum);
		buf[4] = ddnum%256;//LOBYTE(ddnum);
	}
	if(m_CRCtype) {
		wCRC = CRC16(buf,6);
		if(gcheckhorl == 'h'){		
			buf[6] = wCRC/256;//HIBYTE(wCRC);
			buf[7] = wCRC%256;//LOBYTE(wCRC);
		}else{
			buf[7] = wCRC/256;//HIBYTE(wCRC);
			buf[6] = wCRC%256;//LOBYTE(wCRC);
		}
	}else{
		buf[6] = CRC8(buf,6);
	}
	int dataLen = m_CRCtype?8:7;
	sendMSG(buf,dataLen);

	int headAndTailLen = m_CRCtype ? 4 : 3;
	
	m_ackDataLen = m_ddss[gcurdd].dddatalen + headAndTailLen + m_ddss[gcurdd].cbdatalenbit;
																		 
	gcurdd++;
	m_lastReqTime = GetNowSecond();
	return;
}

void CC5Modbus::sendMSG(uint8* buf, int dataLen){
	pLink->RegisterFrmCode(RAW_CODEDIR_DOWN,(char *)buf,dataLen);
	pTxBuf->Write(buf,dataLen);
	pLink->SetCommStatus(CMST_RX_CNT);
	m_lastSendTime = GetNowSecond();
}

void CC5Modbus::ParseFrame()
{
	int nowtime = GetNowSecond();

	sint32 rtuno = pLink->GetRtuNo();
	sint32 rtuaddr = pRtu->GetRtuAddr(rtuno);

	uint8  buf[2048];
	int datalen = 0 ,datanum =0;

	int buflen = pRxBuf->GetReadableSize();
	datalen = pRxBuf->Read(buf,buflen,DEF_BUFF_NOMOVE);
	
	E_RAW_ComStat coms = pLink->GetCommStatus();
	if(coms !=CMST_RX_CNT) return;
	if ( coms == CMST_RX_CNT &&
		(nowtime - m_lastReqTime) >= m_timeOut)
	{
		pRxBuf->Move(buflen);
		pLink->RegisterFrm(FRAME_RX_TIMEOUT);
		pLink->SetCommStatus(CMST_NORMAL);
	#ifdef DEBUG_MODE
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"%d���ն� rtuaddr:%d ���ݳ�ʱ,���ݳ��ȣ�%d\n",
			rtuno,rtuaddr,datalen);
	#endif
		return ;
	}

	if (buflen == 0)
		return ;
	if(buflen > 0){
		static int count = 0;
		count++;
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_INFORMATION,
			"%d���ն� rtuaddr:%d �յ�����,���ݳ��ȣ�%d,   count:%d    ",
			rtuno,rtuaddr,buflen,count);
	}

	if (buflen < m_ackDataLen)
		return ;

	if(buf[0] != rtuaddr)
	{
		pRxBuf->Move(1);
		return ;
	}

	if(buflen > 2048)
	{
	#ifdef DEBUG_MODE
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_INFORMATION,
			"%d���ն˻�����Խ��,rcv_NUM=%d,buf_len=2048\n", rtuno,buflen);
	#endif
		pRxBuf->Move(buflen);
		pLink->SetCommStatus(CMST_NORMAL);
		return ;
	}

	ProcessBuf(buf, rtuno,  buflen,  datalen);
}

void CC5Modbus::ProcessBuf(uint8* buf, sint32 rtuno, int buflen, int datalen){
	switch(m_askFrameType)
	{
	case YC_FRAME:
	case YC_CYCLIC:
		ProcessYc(buf, datalen);
		gcuryc++;
		break;
	case YX_FRAME:
	case YX_CYCLIC:
		ProcessYx(buf, datalen);
		gcuryx++;
		break;
	case DD_FRAME:
	case DD_CYCLIC:
		ProcessDD(buf, datalen);
		gcurdd++;
		break;
	default:
		break;
	}

	pRxBuf->Move(buflen);
	pLink->SetCommStatus(CMST_NORMAL);
	
	return ;
}


void CC5Modbus::ProcessYc(uint8 *buf, int datalen)
{
	gcuryc--;
	if(gcuryc < 0){
		gcuryc = 0;
		return;
	}
	ycsection ycsec = m_ycss[gcuryc];
	if(ycsec.ycdatalen != sixteen2uint(buf+2, ycsec.cbdatalenbit, 'h') && ycsec.ycdatalen != sixteen2uint(buf+2, ycsec.cbdatalenbit, 'l')){
		return;
	}
	int rcvCRC1, rcvCRC2;
	int CRC;
	if(m_CRCtype == 1){
		CRC = CRC16(buf, 2 + ycsec.cbdatalenbit + ycsec.ycdatalen);
		rcvCRC1 = buf[2 + ycsec.cbdatalenbit + ycsec.ycdatalen] * 256 +  buf[2 + ycsec.cbdatalenbit + ycsec.ycdatalen + 1];
		rcvCRC2 = buf[2 + ycsec.cbdatalenbit + ycsec.ycdatalen] +  buf[2 + ycsec.cbdatalenbit + ycsec.ycdatalen + 1] * 256;
		if(CRC != rcvCRC1 && CRC != rcvCRC2){
			kprintf(LOG_COMM,
						DCV_LOG_MODBUS,
						LOG_ERROR,
						"CRCУ��ʧ�ܣ�%d",ycsec.addr);
			return;
		}
	}else{
		CRC = CRC8(buf, 2 + ycsec.cbdatalenbit + ycsec.ycdatalen);
		rcvCRC1 = buf[2 + ycsec.cbdatalenbit + ycsec.ycdatalen];
		if(CRC != rcvCRC1){
			return;
		}
	}
	
	sint32 rtuno = pLink->GetRtuNo();
	int valLen = ycsec.ycslen;
	int ycval=0;
	float ycvalf=0.0;
	int putvalindex = ycsec.startindex;//�ϱ����������

	int method = ycsec.method;
	
	for(int i = 0; i < valLen; i++){
		ycstruct ycstr = ycsec.ycs[i];
		
		if(analysisBuffer(buf + 2 + ycsec.cbdatalenbit, ycstr.method, i, ycsec.cbdatahorl) == -1){
			return;
		}
		ycval = iResult;
		ycvalf = fResult;
		
		if(ycstr.used){
			if(ycsec.hasnan){
				if(ycsec.nankey == iResult || ycsec.nankey == (int)fResult){
					ycval = ycsec.nanvalue;
					ycvalf = ycsec.nanvalue;
				}
			}
			if(ycstr.coe && ycstr.coe > 0){
				ycval = ycval * ycstr.coe;
				ycvalf = ycvalf * ycstr.coe;
			}
			if(ycstr.dit>=0){
				PRawCtrl->PutAYc(rtuno, ycstr.dit, ycval ? ycval: (int)ycvalf);
				putvalindex = ycstr.dit;
			}else{
				PRawCtrl->PutAYc(rtuno, putvalindex, ycval ? ycval: (int)ycvalf);
				kprintf(LOG_COMM,
						DCV_LOG_MODBUS,
						LOG_VIOLATION,
						"yc���û����д�� %d, %d ", gcuryc, i);
			}
			putvalindex++;
		}
	}
	pLink->RegisterFrmCode(RAW_CODEDIR_UP, (char*)buf, datalen);
	pLink->RegisterFrm(FRAME_RX_SUC);
}

void CC5Modbus::ProcessYx(uint8 *buf, int datalen)
{
	gcuryx--;
	if(gcuryx < 0){
		gcuryx = 0;
		return;
	}
	yxsection yxsec = m_yxss[gcuryx];
	if(yxsec.yxdatalen != sixteen2uint(buf+2, yxsec.cbdatalenbit, 'h') && yxsec.yxdatalen != sixteen2uint(buf+2, yxsec.cbdatalenbit, 'l')){
		kprintf(LOG_COMM,
						DCV_LOG_MODBUS,
						LOG_ERROR,
						"YX������У��ʧ�ܣ�%d ,%d ,%d ,%d ,%d",yxsec.addr,yxsec.yxdatalen,
						sixteen2uint(buf+2, yxsec.cbdatalenbit, 'h'),sixteen2uint(buf+2, yxsec.cbdatalenbit, 'l'),
						yxsec.cbdatalenbit);
		return;
	}
	int rcvCRC1, rcvCRC2;
	int CRC;
	if(m_CRCtype == 1){
		CRC = CRC16(buf, 2 + yxsec.cbdatalenbit + yxsec.yxdatalen);
		rcvCRC1 = buf[2 + yxsec.cbdatalenbit + yxsec.yxdatalen] * 256 +  buf[2 + yxsec.cbdatalenbit + yxsec.yxdatalen + 1];
		rcvCRC2 = buf[2 + yxsec.cbdatalenbit + yxsec.yxdatalen] +  buf[2 + yxsec.cbdatalenbit + yxsec.yxdatalen + 1] * 256;
		if(CRC != rcvCRC1 && CRC != rcvCRC2){
			kprintf(LOG_COMM,
						DCV_LOG_MODBUS,
						LOG_ERROR,
						"CRCУ��ʧ�ܣ�%d",yxsec.addr);
			return;
		}
	}else{
		CRC = CRC8(buf, 2 + yxsec.cbdatalenbit + yxsec.yxdatalen);
		rcvCRC1 = buf[2 + yxsec.cbdatalenbit + yxsec.yxdatalen];
		if(CRC != rcvCRC1){
			return;
		}
	}
	sint32 rtuno = pLink->GetRtuNo();
	int yxNum = 0;
	int startindex = 0;

	int putvalindex = 0;//�ϱ����������
	yxNum = yxsec.yxslen;
	startindex = yxsec.startindex;
	for(int i = 0; i < yxNum; i++){
		uint8 oldyxval = 0;
		uint8 newyxval = 0;
		yxstruct yxstr;
		if(analysisBuffer(buf + 2 + yxsec.cbdatalenbit, yxsec.method, i, yxsec.cbdatahorl) == -1){
			return;
		}
		newyxval = iResult;
		try{
			yxstr = yxsec.yxs[i];
		}catch(exception& e){
			kprintf(LOG_COMM,
						DCV_LOG_MODBUS,
						LOG_ERROR,
						"ң�ű��Ľ�������%s,  ",e.what());
			yxstr = yxsec.yxs[0];
		}
		
		PRawCtrl->GetAYx(rtuno, i, &oldyxval);
		if(oldyxval != newyxval){
			S_RAWSOE rawsoe;
			CJskTime	JTime;
			SJSK_CLOCK NowTime;
			JTime.GetNow(&NowTime);//��õ�ǰʱ��
			rawsoe.Yxno=i;
			rawsoe.Val=newyxval;
			rawsoe.Source=1;
			rawsoe.Second=NowTime.second;
			rawsoe.Ms=NowTime.msecond;
			rawsoe.Rtuno=pLink->GetRtuNo();
			rawsoe.Year=NowTime.year;
			rawsoe.Month=NowTime.month;
			rawsoe.Day=NowTime.day;
			rawsoe.Hour=NowTime.hour;
			rawsoe.Minute=NowTime.minute;
			if(yxstr.used){
				PRawCtrl->PutASoe(rawsoe);
			}
		}
		if(yxstr.used){
			if(yxstr.dit>=0){
				PRawCtrl->PutAYx(rtuno, yxstr.dit, newyxval);
				putvalindex = yxstr.dit;
			}else{
				PRawCtrl->PutAYx(rtuno, putvalindex, newyxval);
				kprintf(LOG_COMM,
						DCV_LOG_MODBUS,
						LOG_VIOLATION,
						"���û����д�� %d, %d ", gcuryx, i);
			}
			putvalindex++;
		}
	}
	pLink->RegisterFrmCode(RAW_CODEDIR_UP, (char*)buf, datalen);
	pLink->RegisterFrm(FRAME_RX_SUC);
}

void CC5Modbus::ProcessDD(uint8 *buf, int datalen)
{
	gcurdd--;
	if(gcurdd < 0){
		gcurdd = 0;
		return;
	}
	ddsection ddsec = m_ddss[gcurdd];
	if(ddsec.dddatalen != sixteen2uint(buf+2, ddsec.cbdatalenbit, 'h') && ddsec.dddatalen != sixteen2uint(buf+2, ddsec.cbdatalenbit, 'l')){
		return;
	}
	int rcvCRC1, rcvCRC2;
	int CRC;
	if(m_CRCtype == 1){
		CRC = CRC16(buf, 2 + ddsec.cbdatalenbit + ddsec.dddatalen);
		rcvCRC1 = buf[2 + ddsec.cbdatalenbit + ddsec.dddatalen] * 256 +  buf[2 + ddsec.cbdatalenbit + ddsec.dddatalen + 1];
		rcvCRC2 = buf[2 + ddsec.cbdatalenbit + ddsec.dddatalen] +  buf[2 + ddsec.cbdatalenbit + ddsec.dddatalen + 1] * 256;
		if(CRC != rcvCRC1 && CRC != rcvCRC2){
			kprintf(LOG_COMM,
						DCV_LOG_MODBUS,
						LOG_ERROR,
						"CRCУ��ʧ�ܣ�%d",ddsec.addr);
			return;
		}
	}else{
		CRC = CRC8(buf, 2 + ddsec.cbdatalenbit + ddsec.dddatalen);
		rcvCRC1 = buf[2 + ddsec.cbdatalenbit + ddsec.dddatalen];
		if(CRC != rcvCRC1){
			return;
		}
	}
	sint32 rtuno = pLink->GetRtuNo();
	int valLen = ddsec.ddslen;
	int ddval = 0;
	float ddvalf = 0.0;
	int putvalindex = 0;//�ϱ����������
	for(int i = 0; i < valLen; i++){
		ddstruct ddstr = ddsec.dds[i];
		
		if(analysisBuffer(buf + 2 + ddsec.cbdatalenbit, ddstr.method, i, ddsec.cbdatahorl) == -1){
			return;
		}
		ddval = iResult;
		ddvalf = fResult;
		
		if(ddstr.used){
			if(ddsec.hasnan){
				if(ddsec.nankey == iResult || ddsec.nankey == (int)fResult){
					ddval = ddsec.nanvalue;
					ddvalf = ddsec.nanvalue;
				}
			}
			if(ddstr.coe && ddstr.coe > 0){
				ddval = ddval * ddstr.coe;
				ddvalf = ddvalf * ddstr.coe;
			}
			if(ddstr.dit>=0){
				PRawCtrl->PutAKwh(rtuno, ddstr.dit, ddval? ddval : (int)ddvalf);
				putvalindex = ddstr.dit;
			}else{
				PRawCtrl->PutAKwh(rtuno, putvalindex, ddval? ddval : (int)ddvalf);
				kprintf(LOG_COMM,
						DCV_LOG_MODBUS,
						LOG_VIOLATION,
						"���û����д�� %d, %d ", gcurdd, i);
			}
			putvalindex++;
		}
	}
	pLink->RegisterFrmCode(RAW_CODEDIR_UP, (char*)buf, datalen);
	pLink->RegisterFrm(FRAME_RX_SUC);
}

//����Ϊ���׺�����������ѯsqlite���ݿ�

void CC5Modbus::setCommonConfig(){
	gisregiste = 0;
	gisheartbeat = 0;//�Ƿ�����������

	gregisterhorl = 'h';//�Ĵ�����ַ�ߵ�λ����
}

std::string CC5Modbus::bigmakeSqlStr(int rtuno){
	std::string sqlStr =  "select id,desc,sort,addrhorl,gcheckcrc,version from big where id = ";
	char s[12];
    itoa(rtuno,s,10);
	std::string strTmp = s;
	sqlStr += strTmp;
	return sqlStr;
}

bool CC5Modbus::bigquery(int bigid)
{
	char* cErrMsg;
	int nrownum = 0, ncolnum = 0;  
	char **azResult;
	char* bigHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2001_DESC) as F2001_DESC,F2001_ADDRHL,F2001_CRC,F2001_VERSION,F2001_USED,RTRIM(F2001_REMARK) as F2001_REMARK";
	CString sqlStr;
	sqlStr.Format("select %s from TB2001_PROTOCOL where F2001_CODE = '%d' order by cast(F2001_CODE as int) asc", bigHeadName, bigid);

	int nRes = sqlite3_get_table(pDB, sqlStr.GetBuffer(0), &azResult, &nrownum, &ncolnum, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"��ѯ���ݿ�big��ʧ�ܣ�%s",cErrMsg);
		return false;
	}else if(nrownum < 1){
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"��ѯ���ݿ�big��û������");
		return false;
	}else{
		bigResultFormat(nrownum, ncolnum, azResult);
	}
	return true;
}
void CC5Modbus::bigResultFormat(int nrownum, int ncolnum, char **argv){
	
	for(int iRow = 1; iRow <= nrownum; iRow++){
		int offset = ncolnum * iRow;
		
		nowVersion = argv[offset + 4] ? atoi(argv[offset + 4]) : 0;
		
		gregisterhorl = *argv[offset + 2];
		strcpy(gbigid, argv[offset + 0]);
		gcheckcrc = argv[offset + 3] ? atoi(argv[offset + 3]) : 0;
		if(gcheckcrc == 0 || gcheckcrc == 2){
			gcheckhorl = 'h';
		}else{
			gcheckhorl = 'l';
		}
		if(gcheckcrc == 2 || gcheckcrc == 3){
			m_CRCtype = 0;
		}else{
			m_CRCtype = 1;
		}
	}
}

bool CC5Modbus::ycGroupQuery(char* bigid){
	CString id = bigid;
	m_ycsectionNum = 0;
	char* ycGroupHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2002_CODE) as F2002_CODE,RTRIM(F2002_DESC) as F2002_DESC,F2002_ADDR,F2002_RXDATALEN,F2002_DATALENHL,F2002_METHOD,F2002_RXDATALENBIT,F2002_RXDATALENBITHL,F2002_HASINVALIDVAL,F2002_INVALIDIFVAL,F2002_INVALIDREVAL";

	CString sqlStr;
	sqlStr.Format("select %s from TB2002_YCGROUP where F2001_CODE = '%s' order by cast(F2002_CODE as int)", ycGroupHeadName, id);

	char* cErrMsg = 0;
	int nrownum = 0, ncolnum = 0;  
	char **azResult;
	int nRes = sqlite3_get_table(pDB, sqlStr.GetBuffer(0), &azResult, &nrownum, &ncolnum, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"��ѯ���ݿ�ycGroup��ʧ�ܣ�%s",cErrMsg);
		return false;
	}else if(nrownum < 1){
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"��ѯ���ݿ�ycGroup��û������");
		return false;
	}else{
		ycGroupResultFormat(nrownum, ncolnum, azResult);
	}
	return true;
}

void CC5Modbus::ycGroupResultFormat(int nrownum, int ncolnum, char** argv){
	for(int iRow = 1; iRow <= nrownum; iRow++){
		int offset = ncolnum * iRow;

		m_ycsectionNum++;
		ycssindex = m_ycsectionNum-1;
		m_ycss[ycssindex].ycslen = 0;
		//���vector
		vector<ycstruct>().swap(m_ycss[ycssindex].ycs);

		m_ycss[ycssindex].funcode = atoi(argv[offset + 12]);
		m_ycss[ycssindex].addr = atoi(argv[offset + 3]);
		m_ycss[ycssindex].startindex = 0;
		m_ycss[ycssindex].ycdatalen = atoi(argv[offset + 4]);
		m_ycss[ycssindex].cbdatalenbit = (argv[offset + 7]) ? atoi(argv[offset + 7]) : 1;
		m_ycss[ycssindex].method = atoi(argv[offset + 6]);

		m_ycss[ycssindex].horl = argv[offset + 5] ? *argv[offset + 5] : 'h';
		m_ycss[ycssindex].cbdatahorl = argv[offset + 8] ? *argv[offset + 8] : 'h';
		m_ycss[ycssindex].hasnan = atoi(argv[offset + 9]);
		m_ycss[ycssindex].nankey = atoi(argv[offset + 10]);
		m_ycss[ycssindex].nanvalue = atoi(argv[offset + 11]);
		strcpy(m_ycss[ycssindex].id, argv[offset + 1]);
	}
}

bool CC5Modbus::yxGroupQuery(char* bigid){
	m_yxsectionNum = 0;
	CString id = bigid;
	char * yxgroupHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2004_CODE) as F2004_CODE,RTRIM(F2004_DESC) as F2004_DESC,F2004_ADDR,F2004_DATALENHL,F2004_METHOD,F2004_YXNUM,F2004_RXDATALENBIT,F2004_RXDATALENBITHL";
	
	CString sqlStr;
	sqlStr.Format("select %s from TB2004_YXGROUP where F2001_CODE = '%s' order by cast(F2004_CODE as int)", yxgroupHeadName, id);

	char* cErrMsg = 0;
	int nrownum = 0, ncolnum = 0;  
	char **azResult;
	int nRes = sqlite3_get_table(pDB, sqlStr.GetBuffer(0), &azResult, &nrownum, &ncolnum, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"��ѯ���ݿ�yxGroup��ʧ�ܣ�%s",cErrMsg);
		return false;
	}else if(nrownum < 1){
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"��ѯ���ݿ�yxGroup��û������");
		return false;
	}else{
		yxGroupResultFormat(nrownum, ncolnum, azResult);
	}
	return true;
}

void CC5Modbus::yxGroupResultFormat(int nrownum, int ncolnum, char** argv){
	for(int iRow = 1; iRow <= nrownum; iRow++){
		int offset = ncolnum * iRow;

		m_yxsectionNum++;
		yxssindex = m_yxsectionNum-1;
		m_yxss[yxssindex].yxslen = 0;
		//���vector
		vector<yxstruct>().swap(m_yxss[yxssindex].yxs);

		m_yxss[yxssindex].funcode = atoi(argv[offset + 9]);
		m_yxss[yxssindex].addr = atoi(argv[offset + 3]);
		m_yxss[yxssindex].startindex =  0;
		
		m_yxss[yxssindex].cbdatalenbit = argv[offset + 7] ? atoi(argv[offset + 7]) : 1;
		m_yxss[yxssindex].method = atoi(argv[offset + 5]);
		m_yxss[yxssindex].yxnum = (argv[offset + 6]) ? atoi(argv[offset + 6]) : 0;

		m_yxss[yxssindex].horl = argv[offset + 4] ? *argv[offset + 4] : 'h';
		m_yxss[yxssindex].cbdatahorl = argv[offset + 8] ? *argv[offset + 8] : 'h';
		
		strcpy(m_yxss[yxssindex].id, argv[offset + 1]);
	}
}

bool CC5Modbus::ddGroupQuery(char* bigid){
	m_ddsectionNum = 0;
	CString id = bigid;
	char * ddGroupHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2006_CODE) as F2006_CODE,RTRIM(F2006_DESC) as F2006_DESC,F2006_ADDR,F2006_RXDATALEN,F2006_DATALENHL,F2006_METHOD,F2006_RXDATALENBIT,F2006_RXDATALENBITHL,F2006_HASINVALIDVAL,F2006_INVALIDIFVAL,F2006_INVALIDREVAL";
	CString sqlStr;
	sqlStr.Format("select %s from TB2006_DDGROUP where F2001_CODE = '%s' order by cast(F2006_CODE as int)",ddGroupHeadName, id);
	
	char* cErrMsg = 0;
	int nrownum = 0, ncolnum = 0;  
	char **azResult;
	int nRes = sqlite3_get_table(pDB, sqlStr.GetBuffer(0), &azResult, &nrownum, &ncolnum, &cErrMsg);
	if (nRes != SQLITE_OK){
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"��ѯ���ݿ�ddGroup��ʧ�ܣ�%s",cErrMsg);
		return false;
	}else if(nrownum < 1){
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"��ѯ���ݿ�ddGroup��û������");
		return false;
	}else{
		ddGroupResultFormat(nrownum, ncolnum, azResult);
	}

	return true;
}

void CC5Modbus::ddGroupResultFormat(int nrownum, int ncolnum, char** argv){
	for(int iRow = 1; iRow <= nrownum; iRow++){
		int offset = ncolnum * iRow;

		m_ddsectionNum++;
		ddssindex = m_ddsectionNum-1;
		m_ddss[ddssindex].ddslen = 0;
		//���vector
		vector<ddstruct>().swap(m_ddss[ddssindex].dds);

		m_ddss[ddssindex].funcode = atoi(argv[offset + 12]);
		m_ddss[ddssindex].addr = atoi(argv[offset + 3]);
		m_ddss[ddssindex].startindex =  0;
		m_ddss[ddssindex].dddatalen = atoi(argv[offset + 4]);
		m_ddss[ddssindex].cbdatalenbit =(argv[offset + 7]) ? atoi(argv[offset + 7]) : 1;
		m_ddss[ddssindex].method = atoi(argv[offset + 6]);

		m_ddss[ddssindex].horl = argv[offset + 5] ? *argv[offset + 5] : 'h';
		m_ddss[ddssindex].cbdatahorl = argv[offset + 8] ? *argv[offset + 8] : 'h';
		m_ddss[ddssindex].hasnan = atoi(argv[offset + 9]);
		m_ddss[ddssindex].nankey = atoi(argv[offset + 10]);
		m_ddss[ddssindex].nanvalue =atoi(argv[offset + 11]);
		strcpy(m_ddss[ddssindex].id, argv[offset + 1]);
	}
}

std::string CC5Modbus::ycmakeSqlStr(char* bigid){
	std::string sqlStr =  "select bigid,id,desc,addr,coe,relation,length,horl,method,used,sort,startindex,cbdatalenbit,cbdatahorl,hasnan,nankey,nanvalue from yc where bigid = ";
	std::string strTmp = bigid;
	sqlStr += strTmp;
	return sqlStr;
}

bool CC5Modbus::ycQuery(char* bigid, char* groupid, int ycssindex)
{
	char* cErrMsg = 0;

	int nrownum = 0, ncolnum = 0;  
	char **azResult;
	char* ycHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2002_CODE) as F2002_CODE,RTRIM(F2003_CODE) as F2003_CODE,RTRIM(F2003_DESC) as F2003_DESC,F2003_COE,F2003_USED,F2003_POINTNO";

	CString id1 = bigid;
	CString id2= groupid;
	CString sqlStr;
	sqlStr.Format("select %s from TB2003_YCPOINT where F2001_CODE = '%s' and F2002_CODE = '%s' order by cast(F2003_CODE as int)", ycHeadName, id1, id2);

	int nRes = sqlite3_get_table(pDB, sqlStr.GetBuffer(0), &azResult, &nrownum, &ncolnum, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
	 kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"��ѯ���ݿ�yc��ʧ�ܣ�%s",cErrMsg);
	return false;
	}else{
	  ycResultFormat(nrownum, ncolnum, azResult, ycssindex);
	}
	return true;
}
void CC5Modbus::ycResultFormat(int nrownum, int ncolnum, char **argv, int ycssindex){
	if(nrownum < 1){
		m_ycsectionNum = 0;
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"��ѯ���ݿ�yc��û������");
		return;
	}
	for(int iRow = 1; iRow <= nrownum; iRow++){
		int offset = ncolnum * iRow;

		m_ycss[ycssindex].ycslen++;
		ycstruct ycstr;
		m_ycss[ycssindex].ycs.push_back(ycstr);
		int ycsindex = m_ycss[ycssindex].ycslen-1;

		strcpy(m_ycss[ycssindex].ycs[ycsindex].id, argv[offset + 2]);
		m_ycss[ycssindex].ycs[ycsindex].coe = argv[offset + 4] ? atof(argv[offset + 4]) : 1.0;
		m_ycss[ycssindex].ycs[ycsindex].used = atoi(argv[offset + 5]);
		m_ycss[ycssindex].ycs[ycsindex].dit = (argv[offset + 6]) ? atoi(argv[offset + 6]) : -1;

		m_ycss[ycssindex].ycs[ycsindex].horl = m_ycss[ycssindex].horl;
		m_ycss[ycssindex].ycs[ycsindex].method = m_ycss[ycssindex].method;
		m_ycss[ycssindex].ycs[ycsindex].length = m_ycss[ycssindex].ycdatalen;
	}
}

std::string CC5Modbus::yxmakeSqlStr(char* bigid){
	std::string sqlStr =  "select bigid,id,desc,addr,horl,method,used,sort,startindex,offset,yxnum,cbdatalenbit,cbdatahorl from yx where bigid = ";
	std::string strTmp = bigid;
	sqlStr += strTmp;
	return sqlStr;
}

bool CC5Modbus::yxQuery(char* bigid, char* groupid, int yxssindex)
{
	CString id1 = bigid;
	CString id2= groupid;
	char * yxHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2004_CODE) as F2004_CODE,RTRIM(F2005_CODE) as F2005_CODE,RTRIM(F2005_DESC) as F2005_DESC,F2005_USED,F2005_POINTNO";

	CString sqlStr;
	sqlStr.Format("select %s from TB2005_YXPOINT where F2001_CODE = '%s' and F2004_CODE = '%s' order by cast(F2005_CODE as int)", yxHeadName, id1, id2);

	char* cErrMsg = 0;
	int nrownum = 0, ncolnum = 0;  
	char **azResult;
	int nRes = sqlite3_get_table(pDB, sqlStr.GetBuffer(0), &azResult, &nrownum, &ncolnum, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"��ѯ���ݿ�yx��ʧ�ܣ�%s",cErrMsg);
		return false;
	}else{
		yxResultFormat(nrownum, ncolnum, azResult, yxssindex);
	}
	return true;
}
void CC5Modbus::yxResultFormat(int nrownum, int ncolnum, char **argv, int yxssindex){
	if(nrownum < 1){
		m_yxsectionNum = 0;
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"��ѯ���ݿ�yx��û������");
		return;
	}
	for(int iRow = 1; iRow <= nrownum; iRow++){
		int offset = ncolnum * iRow;

		m_yxss[yxssindex].yxslen++;
		yxstruct yxstr;
		m_yxss[yxssindex].yxs.push_back(yxstr);
		int yxsindex = m_yxss[yxssindex].yxslen-1;

		strcpy(m_yxss[yxssindex].yxs[yxsindex].id, argv[offset + 2]);
		m_yxss[yxssindex].yxs[yxsindex].used = atoi(argv[offset + 4]);
		m_yxss[yxssindex].yxs[yxsindex].dit = (argv[offset + 5]) ? atoi(argv[offset + 5]) : -1;

		m_yxss[yxssindex].yxs[yxsindex].horl = m_yxss[yxssindex].horl;
	}
}

std::string CC5Modbus::ddmakeSqlStr(char* bigid){
	std::string sqlStr =  "select bigid,id,desc,addr,coe,relation,length,horl,method,used,sort,startindex,reqnum,cbdatalenbit,cbdatahorl,hasnan,nankey,nanvalue from dd where bigid = ";
	std::string strTmp = bigid;
	sqlStr += strTmp;
	return sqlStr;
}

bool CC5Modbus::ddQuery(char* bigid, char* groupid, int ddssindex)
{
	CString id1 = bigid;
	CString id2= groupid;
	char * ddHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2006_CODE) as F2006_CODE,RTRIM(F2007_CODE) as F2007_CODE,RTRIM(F2007_DESC) as F2007_DESC,F2007_COE,F2007_USED,F2007_POINTNO";

	CString sqlStr;
	sqlStr.Format("select %s from TB2007_DDPOINT where F2001_CODE = '%s' and F2006_CODE = '%s' order by cast(F2007_CODE as int)", ddHeadName, id1, id2);
	char* cErrMsg = 0;
	int nrownum = 0, ncolnum = 0;  
	char **azResult;
	int nRes = sqlite3_get_table(pDB, sqlStr.GetBuffer(0), &azResult, &nrownum, &ncolnum, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"��ѯ���ݿ�dd��ʧ�ܣ�%s",cErrMsg);
		return false;
	}else{
		ddResultFormat(nrownum, ncolnum, azResult, ddssindex);
	}	
	return true;
}
void CC5Modbus::ddResultFormat(int nrownum, int ncolnum, char **argv, int ddssindex){
	if(nrownum < 1){
		m_ddsectionNum = 0;
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"��ѯ���ݿ�dd��û������");
		return;
	}
	for(int iRow = 1; iRow <= nrownum; iRow++){
		int offset = ncolnum * iRow;

		m_ddss[ddssindex].ddslen++;
		ddstruct ddstr;
		m_ddss[ddssindex].dds.push_back(ddstr);
		int ddsindex = m_ddss[ddssindex].ddslen-1;

		strcpy(m_ddss[ddssindex].dds[ddsindex].id, argv[offset + 2]);
		m_ddss[ddssindex].dds[ddsindex].coe = argv[offset + 4] ? atof(argv[offset + 4]) : 1.0;
		m_ddss[ddssindex].dds[ddsindex].used = atoi(argv[offset + 5]);
		m_ddss[ddssindex].dds[ddsindex].dit = (argv[offset + 6]) ? atoi(argv[offset + 6]) : -1;

		m_ddss[ddssindex].dds[ddsindex].horl = m_ddss[ddssindex].horl;
		m_ddss[ddssindex].dds[ddsindex].method = m_ddss[ddssindex].method;
		m_ddss[ddssindex].dds[ddsindex].length = m_ddss[ddssindex].dddatalen;
	}
}

//16����תfloat
float sixteen2float(uint8* buf, int len, char horl){

	float val = 0;
	while(len>0){
		if(horl == 'l'){
			val = val * 10000 + buf[len-1]*256 + buf[len-2];
		}else{
			val = val * 10000 + buf[len-1] + buf[len-2]*256;
		}
		len = len - 2;
	}
	return val/1000;
}

//16����תint
int sixteen2int(uint8* buf, int len, char horl){
	int index = 0;
	int val = 0;
	int tem = 1;
	if(horl == 'l'){
		while(len>0){
			tem *= 256;
			val = val * 256 + buf[len-1];
			len--;
		}
		if(val>(tem+1)/2){
			val = val - tem;
		}
	}else{
		while(index<len){
			tem *= 256;
			val = val *256 + buf[index];
			index++;
		}
		if(val>(tem+1)/2){
			val = val - tem;
		}
	}
	return val;
}

//16����ת�Ǹ�
int sixteen2uint(uint8* buf,int len, char horl){
	int index = 0;
	int val = 0;
	if(horl == 'l'){
		while(len>0){
			val = val * 256 + buf[len-1];
			len--;
		}
	}else{
		while(index<len){
			val = val *256 + buf[index];
			index++;
		}
	}
	return val;
}

int CC5Modbus::GetNowSecond()
{
	CJskTime     JSK_T;
	TCriterionTime tmptime;
	JSK_T.GetNow(&tmptime);
	return (int)tmptime;
}

int CC5Modbus::MakeWord(char type, short high, short low) {//���ɲ������
	int result = 0;
	if (type == 'h') {
		result = high * 256 + low;
	} else {
		result = low * 256 + high;
	}
	return result;
}

short int CC5Modbus::CalcYcShortVal(BYTE HighByte,BYTE LowByte)
{
	short int YcValue=0;
	if((HighByte&0x80)==0)
		YcValue=LowByte+HighByte*256;
	else
		YcValue=-(LowByte+(HighByte&0x7f)*256);
	return YcValue;
}

int CC5Modbus::makeBitYxVal(int len, char horl, unsigned char*buf){
	int val = 0;
	int index = 0;
	if(horl =='h'){
		while(len){
			val = buf[index] + val*256;
			index++;
		}
	}else{
		index = len - 1;
		while(len){
			val = buf[index] + val*256;
			index--;
		}
	}
	return val;
}

/**************************************************************************** 
��������: str_to_hex 
��������: �ַ���ת��Ϊʮ������ 
�������: string �ַ��� cbuf ʮ������ len �ַ����ĳ��ȡ� 
�������: �� 
*****************************************************************************/   
static int str_to_hex(char *string, unsigned char *cbuf, int len)  
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

//���е�method��Ҫ��method.ini�ϸ�ƥ�䣬ini�еĵ�һ����method0��case 0��Ӧ���Դ�����
//��ini�ļ���method0���ǵ�һ����method1���ǵڶ�������������
//return 0Ϊ������return -1Ϊ�쳣
int CC5Modbus::analysisBuffer(unsigned char* buf, int method, int index, char cbdatahorl){
	iResult = 0;
	fResult = 0.0;
	switch(method){
	case 0:
		return analysisYc(buf, index, cbdatahorl);
		break;
	case 1:
		return analysisYx(buf, index, cbdatahorl);
		break;
	case 2:
		return analysisDd(buf, index, cbdatahorl);
		break;
	case 3:
		return IpsYc(buf, index, cbdatahorl);
		break;
	case 4:
		return IpsYx(buf, index, cbdatahorl);
		break;
	case 5:
		return IpsDd(buf, index, cbdatahorl);
		break;
	case 6:
		return NewIpsYc(buf, index, cbdatahorl);
		break;
	case 7:
		return Int2(buf, index, cbdatahorl);
		break;
	case 8:
		return Int4(buf, index, cbdatahorl);
		break;
	case 9:
		return UInt2(buf, index, cbdatahorl);
		break;
	case 10:
		return UInt4(buf, index, cbdatahorl);
		break;
	case 11:
		return Float4(buf, index, cbdatahorl);
		break;
	case 12:
		return Float4_DD1(buf, index, cbdatahorl);
		break;
	case 13:
		return EX_INT8_PM800(buf, index, cbdatahorl);
		break;
	case 14:
		return yx4bit(buf, index, cbdatahorl);
		break;
	case 15:
		return yx4char(buf, index, cbdatahorl);
		break;
	case 16:
		return yx4word(buf, index, cbdatahorl);
		break;
	case 17:
		break;
	case 18:
		break;
	default:
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"����Ľ�����������");
		return -1;
	}
	return 0;
}

int CC5Modbus::analysisYc(unsigned char* buf, int index, char cbdatahorl){
	iResult = buf[2*index] + buf[2*index+1]*256;
	return 0;
}

int CC5Modbus::analysisYx(unsigned char* buf, int index, char cbdatahorl){
	iResult = buf[index];
	return 0;
}

int CC5Modbus::analysisDd(unsigned char* buf, int index, char cbdatahorl){
	iResult = sixteen2uint(buf + 4*index, 4, 'h');
	return 0;
}

int CC5Modbus::Int2(unsigned char* pData, int index, char cbdatahorl){
	if(cbdatahorl == 'h'){
		iResult = pData[2*index+1] + pData[2*index]*256;
	}else{
		iResult = pData[2*index+1]*256 + pData[2*index];
	}
	return 0;
}
/*
int CC5Modbus::Int2H(unsigned char* pData, int index, char cbdatahorl){
	iResult = pData[2*index] + pData[2*index+1]*256;
	return 0;
}
*/
int CC5Modbus::Int4(unsigned char* buf, int index, char cbdatahorl){
	//iResult = sixteen2int(buf + 4*index, 4, 'l');
	iResult = sixteen2int(buf + 4*index, 4, cbdatahorl);
	return 0;
}
/*
int CC5Modbus::Int4H(unsigned char* buf, int index, char cbdatahorl){
	//iResult = sixteen2int(buf + 4*index, 4, 'h');
	iResult = sixteen2int(buf + 4*index, 4, cbdatahorl);
	return 0;
}
*/
int CC5Modbus::UInt2(unsigned char* buf, int index, char cbdatahorl){
	//iResult = sixteen2uint(buf + 2*index, 2, 'l');
	iResult = sixteen2uint(buf + 2*index, 2, cbdatahorl);
	return 0;
}
/*
int CC5Modbus::UInt2H(unsigned char* buf, int index, char cbdatahorl){
	//iResult = sixteen2uint(buf + 2*index, 2, 'h');
	iResult = sixteen2uint(buf + 2*index, 2, cbdatahorl);
	return 0;
}
*/
int CC5Modbus::UInt4(unsigned char* buf, int index, char cbdatahorl){
	//iResult = sixteen2uint(buf + 4*index, 4, 'l');
	iResult = sixteen2uint(buf + 4*index, 4, cbdatahorl);
	return 0;
}
/*
int CC5Modbus::UInt4H(unsigned char* buf, int index, char cbdatahorl){
	//iResult = sixteen2uint(buf + 4*index, 4, 'h');
	iResult = sixteen2uint(buf + 4*index, 4, cbdatahorl);
	return 0;
}
float CC5Modbus::Float4L(unsigned char* buf, int index, char cbdatahorl){
	float total;
	unsigned char *r;
	r=(unsigned char*)&total;
	*r=buf[3+4*index];r++;
	*r=buf[2+4*index];r++;
	*r=buf[1+4*index];r++;
	*r=buf[0+4*index];
	fResult = total;
	return 0;
}
float CC5Modbus::Float4H(unsigned char* buf, int index, char cbdatahorl){
	float total;
	unsigned char *r;
	r=(unsigned char*)&total;
	*r=buf[0+4*index];r++;
	*r=buf[1+4*index];r++;
	*r=buf[2+4*index];r++;
	*r=buf[3+4*index];
	fResult = total;
	return 0;
}*/
float CC5Modbus::Float4(unsigned char* buf, int index, char cbdatahorl){
	float total;
	unsigned char *r;
	r=(unsigned char*)&total;
	if(cbdatahorl == 'h'){
		*r=buf[0+4*index];r++;
		*r=buf[1+4*index];r++;
		*r=buf[2+4*index];r++;
		*r=buf[3+4*index];
	}else{
		*r=buf[3+4*index];r++;
		*r=buf[2+4*index];r++;
		*r=buf[1+4*index];r++;
		*r=buf[0+4*index];
	}
	fResult = total;
	return 0;
}
/*
float CC5Modbus::Float4_DD1L(unsigned char* buf, int index, char cbdatahorl){
	float  YC_1,F;
	uint32 YC_32, E_1;
	short  S,E;
	YC_32 = buf[2+4*index] * 256 * 256 * 256 + buf[3+4*index] * 256 * 256 + buf[0+4*index] * 256 + buf[1+4*index];
	S=(YC_32>>31)&0x00000001;
	E=(YC_32>>23)&0x000000ff;
	F=YC_32 & 0x007fffff;
	if((E-127)<0)
		E_1=1/pow(2.0,abs(E-127));
	else
		E_1=pow(2.0,(E-127));
	YC_1=(1-2*S)*(1+F/pow(2.0,23))*E_1;
	fResult = YC_1;
	return 0;
}
float CC5Modbus::Float4_DD1H(unsigned char* buf, int index, char cbdatahorl){
	return 0;
}
*/
//IEEE-754���������ݸ�ʽ
float CC5Modbus::Float4_DD1(unsigned char* buf, int index, char cbdatahorl){
	float  YC_1,F;
	uint32 YC_32, E_1;
	short  S,E;
	if(cbdatahorl == 'h'){
		YC_32 = buf[0+4*index] * 256 * 256 * 256 + buf[1+4*index] * 256 * 256 + buf[2+4*index] * 256 + buf[3+4*index];
	}else{
		YC_32 = buf[2+4*index] * 256 * 256 * 256 + buf[3+4*index] * 256 * 256 + buf[0+4*index] * 256 + buf[1+4*index];
	}
	S=(YC_32>>31)&0x00000001;
	E=(YC_32>>23)&0x000000ff;
	F=YC_32 & 0x007fffff;
	if((E-127)<0)
		E_1=1/pow(2.0,abs(E-127));
	else
		E_1=pow(2.0,(E-127));
	YC_1=(1-2*S)*(1+F/pow(2.0,23))*E_1;
	fResult = YC_1;
	return 0;
}
/*
int CC5Modbus::EX_INT8_PM800L(unsigned char* buf, int index, char cbdatahorl){
	int t1=buf[0]*0x100+buf[1];
	int t2=buf[2]*0x100+buf[3];
	int t3=buf[4]*0x100+buf[5];
	int t4=buf[6]*0x100+buf[7];
	float val=(t1+t2*10000+t3*100000000+(float)t4*1000000000000.0)/1000;//��λ���MW
	fResult = val;
	return 0;
}
int CC5Modbus::EX_INT8_PM800H(unsigned char* buf, int index, char cbdatahorl){
	return 0;
}
*/
int CC5Modbus::EX_INT8_PM800(unsigned char* buf, int index, char cbdatahorl){
	int t1=buf[0]*0x100+buf[1];
	int t2=buf[2]*0x100+buf[3];
	int t3=buf[4]*0x100+buf[5];
	int t4=buf[6]*0x100+buf[7];
	float val;
	if(cbdatahorl == 'h'){
		val=(t4+t3*10000+t2*100000000+(float)t1*1000000000000.0)/1000;//��λ���MW
	}else{
		val=(t1+t2*10000+t3*100000000+(float)t4*1000000000000.0)/1000;//��λ���MW
	}
	fResult = val;
	return 0;
}
//ÿһλ����һ��ң��
int CC5Modbus::yx4bit(unsigned char*buf, int index, char cbdatahorl){
	//int i = index / 8;
	sint16 value=buf[1]+buf[0]*0x100;//����ע��ߵ��ֽڵ�˳��
	if(cbdatahorl == 'h'){
		value=buf[1]*0x100+buf[0];
	}
	value = value>>index;
	if (value&0x0001)
	    iResult = 1;
	else
	    iResult = 0;
	return 0;
}
//ÿ���ֽڴ���һ��ң��
int CC5Modbus::yx4char(unsigned char* buf, int index, char cbdatahorl){
	sint16 value = buf[index/2];
	if(cbdatahorl == 'h'){
		if(index % 2 == 0){
			iResult = value & 0x0001;
		}else{
			iResult = value & 0x0100;
		}
	}else{
		if(index % 2 == 1){
			iResult = value & 0x0001;
		}else{
			iResult = value & 0x0100;
		}
	}
	
	return 0;
}

int CC5Modbus::yx4word(unsigned char* buf, int index, char cbdatahorl){
	sint16 value = buf[index];
	iResult = value;
	return 0;
}


int CC5Modbus::IpsYc(unsigned char* pData, int index, char cbdatahorl){
	sint16 value=0;
	int len_num=0;

	switch(gcuryc)
	{
	case 0://����
		len_num=4;
		break;
	case 1://��ѹ
		len_num=7;
		break;
	case 2://����
		len_num=12;
		break;
	case 3://��������
		len_num=5;
		break;
	case 4://Ƶ��
		len_num=1;
		break;

	default:
		return -1;
	}
	uint16 u16;
	short int s16;

	pData+=2 * index;
	if ((gcuryc)==0||(gcuryc)==1||(gcuryc)==4){
		u16=pData[0]*0x100+pData[1];
		s16=pData[0]*0x100+pData[1];
		if(s16==-32768)
			iResult = 0;
		else
			iResult = u16;
	}else if ((gcuryc)==2){
		s16=pData[0]*0x100+pData[1];
		if(s16==-32768)
			iResult = 0;
		else
			iResult = s16;
	}else if ((gcuryc)==3){
		s16=CalcYcShortVal(pData[0],pData[1]);
		if(s16==-32768)
			iResult = 0;
		else
			iResult = s16;
	}
	return 0;
}

int CC5Modbus::IpsYx(unsigned char* pData, int index, char cbdatahorl){

	sint16 value=pData[1]+pData[0]*0x100;//����ע��ߵ��ֽڵ�˳��
	
	if (value&0x0002)
	    iResult = 1;
	else
	    iResult = 0;

	return 0;
}

int CC5Modbus::IpsDd(unsigned char* pData, int index, char cbdatahorl){
	float t1,t2,t3,t4;
	pData += 8 * index;
	t1=pData[0]*0x100+pData[1];
	t2=pData[2]*0x100+pData[3];
	t3=pData[4]*0x100+pData[5];
	t4=pData[6]*0x100+pData[7];
    fResult = t1/1000+t2*10+t3*100000+t4*1000000000;//��λkwh

	return 0;
}

int CC5Modbus::NewIpsYc(unsigned char* pData, int index, char cbdatahorl){
	int t1;
	pData += 2 * index;
	t1=pData[0]*0x100+pData[1];
	
    iResult = t1;//��λkwh
	return 0;
}
