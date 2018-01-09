//modbus通用规约
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
#include "plat_realdb.h"

#define Random(x) (rand() % x)
#define DCV_LOG_MODBUS 8
const int INITFINESHEDTIME = 120;//初始化需要时间
const int CHANLINKWAITTIME = 120;
const int CHECKCHANGEINTERVAL = 15;//单位:秒  检查变化的时间间隔
const int DTUOFFLINETIME = 60;//单位：秒  dtu设备在此时间内不返回报文则认为dtu设备不在线

DLLEXPORT CProtocol* CreateProtocol(char *defpara)
{
	//_CrtSetBreakAlloc(211);
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
    unsigned char crc=0x00; /* 计算的初始crc值 */ 

    while(len--)
    {
        crc ^= *packet++;  /* 每次先与需要计算的数据异或,计算完指向下一数据 */  
        for (i=8; i>0; --i)   /* 下面这段计算过程与计算一个字节crc一样 */  
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
	m_pDB = NULL;
	
	m_ackDataLen = 0;
	m_askFrameType = FRAME_ERR;

	m_lastInitTime = 0;

	m_systemInitTime = m_constructorTime = GetNowSecond();
	//m_systemInitTime = GetNowSecond();
	
}

CC5Modbus::~CC5Modbus()
{
	
}


//
void CC5Modbus::Init(S_PROTOCOLCFG * pcfg )
{
	PRawCtrl	=	pcfg->PRawCtrl;
	pRxBuf		=	pcfg->pRxBuf;
	pTxBuf		=	pcfg->pTxBuf;
	pCmdMem		=	pcfg->pCmdMem;
	pRtu		=	pcfg->pRtu;
	pLink		=	pcfg->pLink;
	pTable		=	pcfg->pTable;
	pHis		=	pcfg->pHis;
	pPara		=	pcfg->pPara;

	m_log.change = 0;
	m_log.init = 0;
	m_log.modbus = 0;

	kprintf(2, 3, 2, "c5modbus init");
	Init2();
}

void CC5Modbus::Init2(){
	if(GetNowSecond() - m_lastInitTime < 2){
		return;
	}
	m_lastInitTime = GetNowSecond();
	m_timeOut = pLink->GetRxTimeouts();
	m_lastcbdataTime = m_lastCheckChange = m_lastReqTime = m_lastSendTime = GetNowSecond();
	//m_systemInitTime = GetNowSecond();
	m_lastHertTime = 0;
	m_gdevonline = 0;
	m_gcbheartbeat = 0;
	m_gdtuonline = 0;

	m_hasChanged = 0;

	m_isInited = false;
	if(m_timeOut < 100 || m_timeOut > 10000)
		m_timeOut = 303;

	m_greqInterval = m_timeOut/100 * 10;//前两位为周期
	m_timeOut = m_timeOut % 100 * 10;//后两位为超时时间
	char szapipath[MAX_PATH] = {0};//

	sint32 rtuno = pLink->GetRtuNo();

	m_ycssindex = -1;
	m_yxssindex = -1;
	m_ddssindex = -1;
	
	setCommonConfig();
	m_askFrameType = YC_CYCLIC;

	m_gcuryc = 0;
	m_gcuryx = 0;
	m_gcurdd = 0;

	m_firstSendHeart = 0;

	int yxnum = pRtu->GetYxNum(rtuno);
	PRawCtrl->PutAYx(rtuno, yxnum-4, 1);

	PRawCtrl->PutAYx(rtuno, yxnum-2, 0);
	PRawCtrl->PutAYx(rtuno, yxnum-3, 0);
}

void CC5Modbus::interleavedInit(){
	if(!m_isInited){
		sint32 rtuno = pLink->GetRtuNo();
		if(GetNowSecond() - m_systemInitTime < rtuno % 10){
			return;
		}

		getC2RtuInfo();
		m_hasChanged = 0;

		m_c5db = C5DB("10.154.238.187","wontex@1");
		if(m_c5db.openDB()){

try{
			C5DBbigQuery(m_nowBigid, m_c5db);
			
			C5DBycGroupQuery(m_gbigid, m_c5db);
			C5DByxGroupQuery(m_gbigid, m_c5db);
			C5DBddGroupQuery(m_gbigid, m_c5db);
			
			int i,l;
			for(i = 0,l=m_ycssindex; i <= l ; i++){
				C5DBycQuery(m_gbigid,m_ycss[i].id,i,m_c5db);
			}
			for(i = 0,l=m_yxssindex; i <= l ; i++){
				C5DByxQuery(m_gbigid,m_yxss[i].id,i,m_c5db);
			}
			for(i = 0,l=m_ddssindex; i <= l ; i++){
				C5DBddQuery(m_gbigid,m_ddss[i].id,i,m_c5db);
			}
			
}catch(...){
}
			
try{
			m_c5db.closeDB();
}catch(...){
}
		}
		m_isInited = true;
		m_run = 1;
	}
}
//
sint32 CC5Modbus::RxProc()
{
	if(!m_run){
		return 0;
	}
	ParseFrame();
	return 1;
}

//
sint32 CC5Modbus::TxProc()
{
	interleavedInit();
	if(!m_run){
		int nowtime = GetNowSecond();
		if(nowtime - m_lastCheckChange >= CHANLINKWAITTIME){
			Init2();
			//m_run = 1;
		}
		return 0;
	}
	
	if(!m_isInited){
		return 0;
	}
	int nowtime;
	nowtime = GetNowSecond();
	
	if(nowtime - m_lastCheckChange > CHECKCHANGEINTERVAL+Random(CHECKCHANGEINTERVAL) && nowtime - m_constructorTime > INITFINESHEDTIME){
		checkChange();
	}

	if(IsNeedToSendHeartBeat(nowtime)){
		SendHeartBeat();
	}
	
	E_RAW_ComStat coms = pLink->GetCommStatus();
	if( coms != CMST_TX_CNT && coms != CMST_NORMAL){
		return 0;
	}

	if(!m_gdtuonline || !m_gcbheartbeat){
		return 0;
	}

	if(nowtime - m_lastReqTime < m_greqInterval){
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
		if(m_gcuryc >= m_ycsectionNum){
			m_gcuryc=0;
			m_askFrameType = YX_CYCLIC; RequestYx(); break;
		}
		RequestYc();
		break;
		
	case YX_CYCLIC:	
		if(m_gcuryx >= m_yxsectionNum){
			m_gcuryx=0;
			m_askFrameType = DD_CYCLIC; RequestDD(); break;
		}
		RequestYx();
		break;
	case DD_CYCLIC:	
		if(m_gcurdd >= m_ddsectionNum){
			m_gcurdd=0;
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
	if(m_gcuryc >= m_ycsectionNum){
		m_gcuryc = 0;
	}
	sint32 rtuno = pLink->GetRtuNo();
	sint32 rtuadd = pRtu->GetRtuAddr(rtuno);

	uint16  wCRC=0;
	uint16  ycnum = 0;
	ycsection ycsec = m_ycss[m_gcuryc];
	
	for(int i = 0,l = ycsec.ycslen; i < l; i++){
		ycstruct ycstr = ycsec.ycs[i];
		ycnum += ycstr.length;
	}
	m_ycss[m_gcuryc].ycdatalen = ycnum;
	ycnum = (ycnum - 1)/2 +1;

	if(ycnum <= 0 || ycnum > 0x1000)
		return;

	uint8 buf[64];
	buf[0] = rtuadd;
	buf[1] = 0x03;
	if(m_gregisterhorl == 'l'){
		buf[3] = ycsec.addr/256;
		buf[2] = ycsec.addr%256;
	}else{
		buf[2] = ycsec.addr/256;
		buf[3] = ycsec.addr%256;
	}
	if(ycsec.ycs[0].horl == 'h'){//默认低位在前，不是'h'即认为低位在前，其他高低位规则类似
		buf[4] = ycnum/256;//HIBYTE(ycnum);
		buf[5] = ycnum%256;//LOBYTE(ycnum);
	}else{
		buf[5] = ycnum/256;//HIBYTE(ycnum);
		buf[4] = ycnum%256;//LOBYTE(ycnum);
	}
	if(m_CRCtype){
		wCRC = CRC16(buf,6);
		if(m_gcheckhorl == 'h'){
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

	m_ackDataLen = m_ycss[m_gcuryc].ycdatalen + headAndTailLen + m_ycss[m_gcuryc].cbdatalenbit;
	m_gcuryc++;

	m_lastReqTime = GetNowSecond();
	return;
}

void CC5Modbus::RequestYx()
{
	if(m_yxsectionNum <= 0){
		return;
	}
	if(m_gcuryx >= m_yxsectionNum){
		m_gcuryx = 0;
	}
	sint32 rtuno = pLink->GetRtuNo();
	sint32 rtuadd = pRtu->GetRtuAddr(rtuno);

	uint16  wCRC=0;
	uint16  yxnum	= 0;
	yxsection yxsec = m_yxss[m_gcuryx];
	yxnum = yxsec.yxnum ? yxsec.yxnum : yxsec.yxslen;
	m_yxss[m_gcuryx].yxdatalen = yxnum * 2;

	if(yxnum <= 0 || yxnum > 0x1000)
		return;

	uint8	buf[64];
	buf[0] = rtuadd;
	buf[1] = 0x03;
	if(m_gregisterhorl == 'l'){
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
		if(m_gcheckhorl == 'h'){
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
	
	m_ackDataLen = m_yxss[m_gcuryx].yxdatalen + headAndTailLen + m_yxss[m_gcuryx].cbdatalenbit;

	m_gcuryx++;
	m_lastReqTime = GetNowSecond();
	return;
}

void CC5Modbus::RequestDD(){
	if(m_ddsectionNum <= 0){
		return;
	}
	if(m_gcurdd >= m_ddsectionNum){
		m_gcurdd = 0;
	}
	sint32 rtuno = pLink->GetRtuNo();
	sint32 rtuadd = pRtu->GetRtuAddr(rtuno);

	uint16  wCRC=0;
	uint16  ddnum = 0;
	ddsection ddsec = m_ddss[m_gcurdd];
	for(int i = 0,l = ddsec.ddslen; i < l; i++){
		ddstruct ddstr = ddsec.dds[i];
		ddnum += ddstr.length;
	}
	m_ddss[m_gcurdd].dddatalen = ddnum;
	ddnum = ddnum / 2;

	if(ddnum <= 0 || ddnum > 0x1000)
		return;

	uint8 buf[64];
	buf[0] = rtuadd;
	buf[1] = 0x03;
	if(m_gregisterhorl == 'l'){
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
		if(m_gcheckhorl == 'h'){		
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
	
	m_ackDataLen = m_ddss[m_gcurdd].dddatalen + headAndTailLen + m_ddss[m_gcurdd].cbdatalenbit;
																		 
	m_gcurdd++;
	m_lastReqTime = GetNowSecond();
	return;
}

void CC5Modbus::sendMSG(uint8* buf, int dataLen){
	if(m_gisregiste){
		int buflen = addRegHead(buf, dataLen);
		pLink->RegisterFrmCode(RAW_CODEDIR_DOWN,(char *)buf,buflen);
		pTxBuf->Write(buf,buflen);
	}else{
		pLink->RegisterFrmCode(RAW_CODEDIR_DOWN,(char *)buf,dataLen);
		pTxBuf->Write(buf,dataLen);
	}
	pLink->SetCommStatus(CMST_RX_CNT);
	m_lastSendTime = GetNowSecond();
}

int CC5Modbus::addRegHead(uint8 *buf, int buflen){
	int gregisteheadlen = strlen(m_gregistehead);
	int gregistetaillen = strlen(m_gregistetail);
	int gRegHeadLen = (gregisteheadlen-1)/2+1 + (gregistetaillen-1)/2+1 + 1 + IDMAXLEN;
	int i = 0;
	for(i = buflen - 1; i >= 0; i--){
		buf[gRegHeadLen + i] = buf[i];
	}

	
	str_to_hex(m_gregistehead,buf,gregisteheadlen);

	int strLen = strlen(m_gbigname);
	buf[(gregisteheadlen-1)/2+1] = strLen;

	for(i = 0; i < IDMAXLEN; i++){
		buf[i + (gregisteheadlen-1)/2+1 + 1]=0;
		if(i<strLen)
		{
			buf[i + (gregisteheadlen-1)/2+1 + 1]= m_gbigname[i];
		}
	}

	str_to_hex(m_gregistetail,buf + (gregisteheadlen-1)/2+1 + IDMAXLEN + 1, gregistetaillen);

	return buflen + gRegHeadLen;
}


bool CC5Modbus::CheckHeartBeat(){
	bool result = false;

	uint8  buf[2048] = {0};
	uint8  gcbheartsucceedbuf[2048];
	uint8  gcbheartfailbuf[2048];
	int datalen = 0 ,datanum =0;

	int buflen = pRxBuf->GetReadableSize();
	int nowtime = GetNowSecond();

	if (buflen < 4){
		if(nowtime-m_lastHertTime>m_gheartbeatInterval){
			pRxBuf->Move(buflen);
		}
		return false;
	}
	datalen = pRxBuf->Read(buf,buflen,DEF_BUFF_NOMOVE);
	if(datalen != 4){
		pRxBuf->Move(buflen);
		//m_gdevonline = 0;
		m_gdtuonline = 0;
		return false;
	}

	int gcbheartsucceedlen = strlen(m_gcbheartsucceed);
	str_to_hex(m_gcbheartsucceed,gcbheartsucceedbuf,gcbheartsucceedlen);
	int gcbheartfaillen = strlen(m_gcbheartfail);
	str_to_hex(m_gcbheartfail,gcbheartfailbuf,gcbheartfaillen);
	
	int i = 0;
	int l = 0;
	for(i = 0, l = gcbheartfaillen/2; i < l; i++){
		result = true;
		if(buf[i] != gcbheartfailbuf[i]){
			result = false;
			break;
		}
	}
	if(result){
		pLink->RegisterFrm(FRAME_RX_SUC);
		pLink->RegisterFrmCode(RAW_CODEDIR_UP,(char *)buf,datalen);
		pRxBuf->Move(buflen);
		m_gcbheartbeat = 1;
		m_gdtuonline = 0;
		return true;//返回发现心跳报文
	}
	
	for(i = 0, l = gcbheartsucceedlen/2; i < l; i++){
		result = true;
		if(buf[i] != gcbheartsucceedbuf[i]){
			result = false;
			break;
		}
	}
	pLink->RegisterFrm(FRAME_RX_SUC);
	pLink->RegisterFrmCode(RAW_CODEDIR_UP,(char *)buf,datalen);
	pRxBuf->Move(buflen);
	if(result){
		m_gcbheartbeat = 1;
		m_gdtuonline = 1;
		return true;//返回发现心跳报文
	}

	return false;//返回没有发现心跳报文
}

bool CC5Modbus::IsNeedToSendHeartBeat(int now){
	if(!m_gisheartbeat){
		return false;
	}
	if(now - m_lastcbdataTime > m_gheartbeatInterval  && now - m_lastHertTime > m_gheartbeatInterval && now - m_lastSendTime > 1){
		return true;
	}

	//if(now - m_lastSendTime > m_gheartbeatInterval){
	//	return true;
	//}
	return false;
}

void CC5Modbus::SendHeartBeat(){
	int nowtime = GetNowSecond();

	sint32 rtuno = pLink->GetRtuNo();
	sint32 rtuadd = pRtu->GetRtuAddr(rtuno);

	uint8	buf[64];
	int gheartheadlen = strlen(m_ghearthead);
	str_to_hex(m_ghearthead,buf,gheartheadlen);

	int dataLen = gheartheadlen/2;
	sendMSG(buf,dataLen);

	m_gcbheartbeat = 0;

	m_lastHertTime = nowtime;

	return;
}

void CC5Modbus::ParseFrame()
{
	if(m_gisheartbeat && !m_gcbheartbeat){
		CheckHeartBeat();
		sint32 rtuno = pLink->GetRtuNo();
		int yxnum = pRtu->GetYxNum(rtuno);
		//PRawCtrl->PutAYx(rtuno, yxnum-2, m_gdevonline);
		PRawCtrl->PutAYx(rtuno, yxnum-2, m_gdtuonline);	
		
		return;
	}

	if(!m_gdtuonline){
		return;
	}

	int nowtime = GetNowSecond();

	sint32 rtuno = pLink->GetRtuNo();
	sint32 rtuaddr = pRtu->GetRtuAddr(rtuno);

	if(nowtime - m_lastcbdataTime > DTUOFFLINETIME){
		int yxnum = pRtu->GetYxNum(rtuno);
		PRawCtrl->PutAYx(rtuno, yxnum-3, 0);
	}

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
		return ;
	}

	if (buflen == 0)
		return ;
	if(buflen > 0){
		static int count = 0;
		count++;
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
		pRxBuf->Move(buflen);
		pLink->SetCommStatus(CMST_NORMAL);
		return ;
	}

	
	ProcessBuf(buf, rtuno,  buflen,  datalen);
	m_lastcbdataTime = nowtime;
	int yxnum = pRtu->GetYxNum(rtuno);
	PRawCtrl->PutAYx(rtuno, yxnum-3, 1);
	if(!m_log.modbus){
		kprintf(2, 3, 2, "c5modbus 解析成功");
		m_log.modbus = 1;
	}
}

void CC5Modbus::ProcessBuf(uint8* buf, sint32 rtuno, int buflen, int datalen){
	switch(m_askFrameType)
	{
	case YC_FRAME:
	case YC_CYCLIC:
		ProcessYc(buf, datalen);
		m_gcuryc++;
		break;
	case YX_FRAME:
	case YX_CYCLIC:
		ProcessYx(buf, datalen);
		m_gcuryx++;
		break;
	case DD_FRAME:
	case DD_CYCLIC:
		ProcessDD(buf, datalen);
		m_gcurdd++;
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
	m_gcuryc--;
	if(m_gcuryc < 0){
		m_gcuryc = 0;
		return;
	}
	ycsection ycsec = m_ycss[m_gcuryc];
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
	int putvalindex = ycsec.startindex;//上报点的索引号

	int method = ycsec.method;
	
	for(int i = 0; i < valLen; i++){
		ycstruct ycstr = ycsec.ycs[i];
		
		if(analysisBuffer(buf + 2 + ycsec.cbdatalenbit, ycstr.method, i, ycsec.cbdatahorl) == -1){
			return;
		}
		ycval = m_iResult;
		ycvalf = m_fResult;
		
		if(ycstr.used){
			if(ycsec.hasnan){
				if(ycsec.nankey == m_iResult || ycsec.nankey == (int)m_fResult){
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
			}
			putvalindex++;
		}
	}
	pLink->RegisterFrmCode(RAW_CODEDIR_UP, (char*)buf, datalen);
	pLink->RegisterFrm(FRAME_RX_SUC);
}

void CC5Modbus::ProcessYx(uint8 *buf, int datalen)
{
	m_gcuryx--;
	if(m_gcuryx < 0){
		m_gcuryx = 0;
		return;
	}
	yxsection yxsec = m_yxss[m_gcuryx];
	if(yxsec.yxdatalen != sixteen2uint(buf+2, yxsec.cbdatalenbit, 'h') && yxsec.yxdatalen != sixteen2uint(buf+2, yxsec.cbdatalenbit, 'l')){
		return;
	}
	int rcvCRC1, rcvCRC2;
	int CRC;
	if(m_CRCtype == 1){
		CRC = CRC16(buf, 2 + yxsec.cbdatalenbit + yxsec.yxdatalen);
		rcvCRC1 = buf[2 + yxsec.cbdatalenbit + yxsec.yxdatalen] * 256 +  buf[2 + yxsec.cbdatalenbit + yxsec.yxdatalen + 1];
		rcvCRC2 = buf[2 + yxsec.cbdatalenbit + yxsec.yxdatalen] +  buf[2 + yxsec.cbdatalenbit + yxsec.yxdatalen + 1] * 256;
		if(CRC != rcvCRC1 && CRC != rcvCRC2){
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

	int putvalindex = 0;//上报点的索引号
	yxNum = yxsec.yxslen;
	startindex = yxsec.startindex;
	for(int i = 0; i < yxNum; i++){
		uint8 oldyxval = 0;
		uint8 newyxval = 0;
		yxstruct yxstr;
		try{
			yxstr = yxsec.yxs[i];
		}catch(...){
			return;
		}
		if(analysisBuffer(buf + 2 + yxsec.cbdatalenbit, yxsec.method, i, yxsec.cbdatahorl) == -1){
			return;
		}
		newyxval = m_iResult;
		
		if(yxstr.used){
			if(yxstr.dit>=0){

				PRawCtrl->GetAYx(rtuno, yxstr.dit, &oldyxval);
				if(oldyxval != newyxval){
					S_RAWSOE rawsoe;
					CJskTime	JTime;
					SJSK_CLOCK NowTime;
					JTime.GetNow(&NowTime);//获得当前时间
					rawsoe.Yxno=yxstr.dit;
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

				PRawCtrl->PutAYx(rtuno, yxstr.dit, newyxval);
				putvalindex = yxstr.dit;
			}else{
				PRawCtrl->PutAYx(rtuno, putvalindex, newyxval);
			}
			putvalindex++;
		}
	}
	pLink->RegisterFrmCode(RAW_CODEDIR_UP, (char*)buf, datalen);
	pLink->RegisterFrm(FRAME_RX_SUC);
}

void CC5Modbus::ProcessDD(uint8 *buf, int datalen)
{
	m_gcurdd--;
	if(m_gcurdd < 0){
		m_gcurdd = 0;
		return;
	}
	ddsection ddsec = m_ddss[m_gcurdd];
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
	int putvalindex = 0;//上报点的索引号
	for(int i = 0; i < valLen; i++){
		ddstruct ddstr = ddsec.dds[i];
		
		if(analysisBuffer(buf + 2 + ddsec.cbdatalenbit, ddstr.method, i, ddsec.cbdatahorl) == -1){
			return;
		}
		ddval = m_iResult;
		ddvalf = m_fResult;
		
		if(ddstr.used){
			if(ddsec.hasnan){
				if(ddsec.nankey == m_iResult || ddsec.nankey == (int)m_fResult){
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
			}
			putvalindex++;
		}
	}
	pLink->RegisterFrmCode(RAW_CODEDIR_UP, (char*)buf, datalen);
	pLink->RegisterFrm(FRAME_RX_SUC);
}

//以下为配套函数，包含查询sqlite数据库

void CC5Modbus::setCommonConfig(){
	m_gisregiste = 1;
	m_gisheartbeat = 1;//是否有心跳报文
	
	
	m_gheartbeatInterval = 12;//心跳报文间隔
	
	strcpy(m_ghearthead,"eeeeeeee");//心跳报文内容
	strcpy(m_gregistehead,"eb90eb90");//注册报文头
	strcpy(m_gregistetail,"ea80ea80");//注册报文尾
	strcpy(m_gcbheartsucceed,"eeee01ee");//心跳注册成功返回报文
	strcpy(m_gcbheartfail,"eeee00ee");//心跳注册失败返回报文


	m_gregisterhorl = 'h';//寄存器地址高低位规则
}


bool CC5Modbus::C5DBycGroupQuery(char* bigid, C5DB c5db){
	CString id = bigid;

	char* ycGroupHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2002_CODE) as F2002_CODE,RTRIM(F2002_DESC) as F2002_DESC,F2002_ADDR,F2002_RXDATALEN,F2002_DATALENHL,F2002_METHOD,F2002_RXDATALENBIT,F2002_RXDATALENBITHL,F2002_HASINVALIDVAL,F2002_INVALIDIFVAL,F2002_INVALIDREVAL";
	CString sqlStr;
	sqlStr.Format("select %s from TB2002_YCGROUP where F2001_CODE = '%s' order by CONVERT(int, F2002_CODE)", ycGroupHeadName, id);

	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			m_ycssindex++;
			m_ycss[m_ycssindex].ycslen = 0;
			//清空vector
			vector<ycstruct>().swap(m_ycss[m_ycssindex].ycs);

			varName = m_pRecordset->GetCollect ("F2002_ADDR");
			m_ycss[m_ycssindex].addr = _ttoi((char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("addr");
			m_ycss[m_ycssindex].startindex =  0;
			varName = m_pRecordset->GetCollect ("F2002_RXDATALEN");
			m_ycss[m_ycssindex].ycdatalen = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_RXDATALENBIT");
			m_ycss[m_ycssindex].cbdatalenbit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 1;
			varName = m_pRecordset->GetCollect ("F2002_METHOD");
			m_ycss[m_ycssindex].method = _ttoi((char *)_bstr_t(varName));

			varName = m_pRecordset->GetCollect ("F2002_DATALENHL");
			m_ycss[m_ycssindex].horl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2002_RXDATALENBITHL");
			m_ycss[m_ycssindex].cbdatahorl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2002_HASINVALIDVAL");
			m_ycss[m_ycssindex].hasnan = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_INVALIDIFVAL");
			m_ycss[m_ycssindex].nankey = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_INVALIDREVAL");
			m_ycss[m_ycssindex].nanvalue = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_CODE");
			strcpy(m_ycss[m_ycssindex].id, (char *)_bstr_t(varName));

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		m_ycsectionNum = m_ycssindex + 1;
		return true;
	}else{
		m_ycsectionNum = 0;
	}
	return false;
}
bool CC5Modbus::C5DByxGroupQuery(char* bigid, C5DB c5db){
	CString id = bigid;

	char * yxgroupHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2004_CODE) as F2004_CODE,RTRIM(F2004_DESC) as F2004_DESC,F2004_ADDR,F2004_DATALENHL,F2004_METHOD,F2004_YXNUM,F2004_RXDATALENBIT,F2004_RXDATALENBITHL";
	CString sqlStr;
	sqlStr.Format("select %s from TB2004_YXGROUP where F2001_CODE = '%s' order by CONVERT(int, F2004_CODE)", yxgroupHeadName, id);

	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			m_yxssindex++;
			m_yxss[m_yxssindex].yxslen = 0;
			//清空vector
			vector<yxstruct>().swap(m_yxss[m_yxssindex].yxs);

			varName = m_pRecordset->GetCollect ("F2004_ADDR");
			m_yxss[m_yxssindex].addr = _ttoi((char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("addr");
			m_yxss[m_yxssindex].startindex =  0;
			
			varName = m_pRecordset->GetCollect ("F2004_RXDATALENBIT");
			m_yxss[m_yxssindex].cbdatalenbit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 1;
			varName = m_pRecordset->GetCollect ("F2004_METHOD");
			m_yxss[m_yxssindex].method = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2004_YXNUM");
			m_yxss[m_yxssindex].yxnum = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 0;

			varName = m_pRecordset->GetCollect ("F2004_DATALENHL");
			m_yxss[m_yxssindex].horl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2004_RXDATALENBITHL");
			m_yxss[m_yxssindex].cbdatahorl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			
			varName = m_pRecordset->GetCollect ("F2004_CODE");
			strcpy(m_yxss[m_yxssindex].id, (char *)_bstr_t(varName));

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		m_yxsectionNum = m_yxssindex + 1;
		return true;
	}else{
		m_yxsectionNum = 0;
	}
	return false;
}
bool CC5Modbus::C5DBddGroupQuery(char* bigid, C5DB c5db){
	CString id = bigid;

	char * ddGroupHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2006_CODE) as F2006_CODE,RTRIM(F2006_DESC) as F2006_DESC,F2006_ADDR,F2006_RXDATALEN,F2006_DATALENHL,F2006_METHOD,F2006_RXDATALENBIT,F2006_RXDATALENBITHL,F2006_HASINVALIDVAL,F2006_INVALIDIFVAL,F2006_INVALIDREVAL";
	CString sqlStr;
	sqlStr.Format("select %s from TB2006_DDGROUP where F2001_CODE = '%s' order by CONVERT(int, F2006_CODE)",ddGroupHeadName, id);

	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			m_ddssindex++;
			m_ddss[m_ddssindex].ddslen = 0;
			//清空vector
			vector<ddstruct>().swap(m_ddss[m_ddssindex].dds);

			varName = m_pRecordset->GetCollect ("F2006_ADDR");
			m_ddss[m_ddssindex].addr = _ttoi((char *)_bstr_t(varName));
			m_ddss[m_ddssindex].startindex =  0;
			varName = m_pRecordset->GetCollect ("F2006_RXDATALEN");
			m_ddss[m_ddssindex].dddatalen = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_RXDATALENBIT");
			m_ddss[m_ddssindex].cbdatalenbit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 1;
			varName = m_pRecordset->GetCollect ("F2006_METHOD");
			m_ddss[m_ddssindex].method = _ttoi((char *)_bstr_t(varName));

			varName = m_pRecordset->GetCollect ("F2006_DATALENHL");
			m_ddss[m_ddssindex].horl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2006_RXDATALENBITHL");
			m_ddss[m_ddssindex].cbdatahorl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2006_HASINVALIDVAL");
			m_ddss[m_ddssindex].hasnan = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_INVALIDIFVAL");
			m_ddss[m_ddssindex].nankey = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_INVALIDREVAL");
			m_ddss[m_ddssindex].nanvalue = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_CODE");
			strcpy(m_ddss[m_ddssindex].id, (char *)_bstr_t(varName));

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		m_ddsectionNum = m_ddssindex + 1;
		return true;
	}else{
		m_ddsectionNum = 0;
	}
	return false;
}
bool CC5Modbus::C5DBycQuery(char* bigid, char* groupid, int ycssindex, C5DB c5db){
	CString id1 = bigid;
	CString id2= groupid;

	char* ycHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2002_CODE) as F2002_CODE,RTRIM(F2003_CODE) as F2003_CODE,RTRIM(F2003_DESC) as F2003_DESC,F2003_COE,F2003_USED,F2003_POINTNO";
	CString sqlStr;
	sqlStr.Format("select %s from TB2003_YCPOINT where F2001_CODE = '%s' and F2002_CODE = '%s' order by CONVERT(int, F2003_CODE)", ycHeadName, id1, id2);

	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			m_ycss[ycssindex].ycslen++;
			ycstruct ycstr;
			m_ycss[ycssindex].ycs.push_back(ycstr);
			int ycsindex = m_ycss[ycssindex].ycslen-1;

			varName = m_pRecordset->GetCollect ("F2003_CODE");
			strcpy(m_ycss[ycssindex].ycs[ycsindex].id, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2003_COE");
			m_ycss[ycssindex].ycs[ycsindex].coe = (char *)_bstr_t(varName) ? atof((char *)_bstr_t(varName)) : 1.0;
			varName = m_pRecordset->GetCollect ("F2003_USED");
			m_ycss[ycssindex].ycs[ycsindex].used = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2003_POINTNO");
			m_ycss[ycssindex].ycs[ycsindex].dit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : -1;

			m_ycss[ycssindex].ycs[ycsindex].horl = m_ycss[ycssindex].horl;
			m_ycss[ycssindex].ycs[ycsindex].method = m_ycss[ycssindex].method;
			m_ycss[ycssindex].ycs[ycsindex].length = m_ycss[ycssindex].ycdatalen;

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}
bool CC5Modbus::C5DByxQuery(char* bigid, char* groupid, int yxssindex, C5DB c5db){
	CString id1 = bigid;
	CString id2= groupid;
	char * yxHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2004_CODE) as F2004_CODE,RTRIM(F2005_CODE) as F2005_CODE,RTRIM(F2005_DESC) as F2005_DESC,F2005_USED,F2005_POINTNO,F2005_SORT";
	CString sqlStr;
	sqlStr.Format("select %s from TB2005_YXPOINT where F2001_CODE = '%s' and F2004_CODE = '%s' order by CONVERT(int, F2005_CODE)", yxHeadName, id1, id2);
	
	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;

			m_yxss[yxssindex].yxslen++;
			yxstruct yxstr;
			m_yxss[yxssindex].yxs.push_back(yxstr);
			int yxsindex = m_yxss[yxssindex].yxslen-1;

			varName = m_pRecordset->GetCollect ("F2005_CODE");
			strcpy(m_yxss[yxssindex].yxs[yxsindex].id, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2005_USED");
			m_yxss[yxssindex].yxs[yxsindex].used = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2005_POINTNO");
			m_yxss[yxssindex].yxs[yxsindex].dit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : -1;

			//varName = m_pRecordset->GetCollect ("F2005_SORT");
			//if(varName.vt == VT_NULL){
			//	m_yxss[yxssindex].yxs[yxsindex].sort = -1;
			//}else{
			//	m_yxss[yxssindex].yxs[yxsindex].sort = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : -1;
			//}

			m_yxss[yxssindex].yxs[yxsindex].horl = m_yxss[yxssindex].horl;

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}
bool CC5Modbus::C5DBddQuery(char* bigid, char* groupid, int ddssindex, C5DB c5db){
	CString id1 = bigid;
	CString id2= groupid;
	char * ddHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2006_CODE) as F2006_CODE,RTRIM(F2007_CODE) as F2007_CODE,RTRIM(F2007_DESC) as F2007_DESC,F2007_COE,F2007_USED,F2007_POINTNO";

	CString sqlStr;
	sqlStr.Format("select %s from TB2007_DDPOINT where F2001_CODE = '%s' and F2006_CODE = '%s' order by CONVERT(int, F2007_CODE)", ddHeadName, id1, id2);
	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;

			m_ddss[ddssindex].ddslen++;
			ddstruct ddstr;
			m_ddss[ddssindex].dds.push_back(ddstr);
			int ddsindex = m_ddss[ddssindex].ddslen-1;

			varName = m_pRecordset->GetCollect ("F2007_CODE");
			strcpy(m_ddss[ddssindex].dds[ddsindex].id, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2007_COE");
			m_ddss[ddssindex].dds[ddsindex].coe = (char *)_bstr_t(varName) ? atof((char *)_bstr_t(varName)) : 1.0;
			varName = m_pRecordset->GetCollect ("F2007_USED");
			m_ddss[ddssindex].dds[ddsindex].used = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2007_POINTNO");
			m_ddss[ddssindex].dds[ddsindex].dit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : -1;

			m_ddss[ddssindex].dds[ddsindex].horl = m_ddss[ddssindex].horl;
			m_ddss[ddssindex].dds[ddsindex].method = m_ddss[ddssindex].method;
			m_ddss[ddssindex].dds[ddsindex].length = m_ddss[ddssindex].dddatalen;

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}
bool CC5Modbus::C5DBbigQuery(int bigid, C5DB c5db){
	char s[12];
    itoa(bigid,s,10);
	CString ss = s;

	char* bigHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2001_DESC) as F2001_DESC,F2001_ADDRHL,F2001_CRC,F2001_VERSION,F2001_USED,RTRIM(F2001_REMARK) as F2001_REMARK,RTRIM(F1102_CODE) as F1102_CODE";
	CString sqlStr;
	sqlStr.Format("select %s from TB2001_PROTOCOL where F2001_CODE = %s", bigHeadName, ss);

	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			
			_variant_t varName;
			varName = m_pRecordset->GetCollect ("F2001_CODE");
			strcpy(m_gbigid, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2001_ADDRHL");
			m_gregisterhorl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2001_CRC");
			m_gcheckcrc = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2001_VERSION");
			m_nowVersion = _ttoi((char *)_bstr_t(varName));
			if(m_gcheckcrc == 0 || m_gcheckcrc == 2){
				m_gcheckhorl = 'h';
			}else{
				m_gcheckhorl = 'l';
			}
			if(m_gcheckcrc == 2 || m_gcheckcrc == 3){
				m_CRCtype = 0;
			}else{
				m_CRCtype = 1;
			}
			
			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}

void CC5Modbus::ChanDown(){
	sint32 rtuno = pLink->GetRtuNo();
	sint32 channo =pLink->GetChanNo();
	//sint32 rtuadd = pRtu->GetRtuAddr(rtuno);

	CRdbTable RDB_Chan;

	int ret = RDB_Chan.Login("sa","5i5j");
	ret = RDB_Chan.OpenTableByName("dcchan");

	S_RDB_Chan	* pChan = (S_RDB_Chan *)RDB_Chan.GetRcdAddr(0);

	uint16 key = (uint16)channo;
	sint32 chanrcdno = RDB_Chan.FindRcdByKey(&key);
	pChan[chanrcdno].State = EDEV_DOWN_STATE;
	kprintf(2, 3, 2, "c5modbus set down");
}

void CC5Modbus::checkChange(){
	
	m_lastCheckChange = GetNowSecond();
	if(!m_log.change){
		kprintf(2, 3, 2, "c5modbus 检查配置");
		m_log.change = 1;
	}
	checkC5Change();
	getC2RtuInfo();
	if(m_hasChanged){
		m_run = 0;//规约暂停运行
		ChanDown();
		sint32 rtuno = pLink->GetRtuNo();
		int yxnum = pRtu->GetYxNum(rtuno);
		PRawCtrl->PutAYx(rtuno, yxnum-2, 0);//上报dtu离线
		PRawCtrl->PutAYx(rtuno, yxnum-3, 0);//上报dev离线
		PRawCtrl->PutAYx(rtuno, yxnum-4, 0);
	}
}

void CC5Modbus::checkC5Change(){
	int version;

	if(m_c5db.openDB()){
		
		char s[12];
		itoa(m_nowBigid,s,10);
		CString ss = s;
		char* bigHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2001_DESC) as F2001_DESC,F2001_ADDRHL,F2001_CRC,F2001_VERSION,F2001_USED,RTRIM(F2001_REMARK) as F2001_REMARK,RTRIM(F1102_CODE) as F1102_CODE";
		CString sqlStr;
		sqlStr.Format("select %s from TB2001_PROTOCOL where F2001_CODE = %s ", bigHeadName, ss);

		_RecordsetPtr m_pRecordset;
		if(m_c5db.querySQL(sqlStr, m_pRecordset)){
			while(!m_pRecordset->GetadoEOF()){
				
				_variant_t varName;
				
				varName = m_pRecordset->GetCollect ("F2001_VERSION");
				version = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 0;

				m_pRecordset->MoveNext();
			}
			m_pRecordset->Close();
			m_pRecordset.Release();
		}
		m_c5db.closeDB();
	}

	if(m_nowVersion != version){
		m_hasChanged = 1;
	}
	m_nowVersion = version;
}

//获取RTU信息，并检查 bigid、bigname 是否有改变
void CC5Modbus::getC2RtuInfo(){
	int bigid;
	//int name;
	
	char dbPath[128];
	char dbBasePath[128];
	char* envvar = getenv("C2PLAT");
	sint32 rtuno = pLink->GetRtuNo();
	if( envvar == NULL ){
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"获取环境变量C2PLAT失败！   ");
		return;
	}
	sprintf(dbBasePath, "%s/db/jskpara.db", envvar);
	sprintf(dbPath, "%s/db/jskpara_%d.db", envvar, rtuno);
	CopyFile(dbBasePath, dbPath, false);

try{
	int nRes = sqlite3_open(dbPath, &m_pDB);
	if (nRes != SQLITE_OK)
	{
	  kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"打开数据库%s失败,",dbPath);
	  remove(dbPath);
	  return;
	}

	char* cErrMsg;
	int nrownum = 0, ncolnum = 0;  
	char **azResult;
	char str[10];
	sprintf(str, "%d", rtuno); //将100转为16进制表示的字符串。
	std::string sqlStr = "select 装置序号,召全数据间隔,终端号码 from T004_装置表 where 装置序号=";
	sqlStr += str;
	nRes = sqlite3_get_table(m_pDB , sqlStr.c_str() , &azResult, &nrownum, &ncolnum, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"查询数据库T004_装置表失败，%s   ",cErrMsg);
	}else if(nrownum < 1){
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"查询数据库T004_装置表没有相关数据  ");
	}else{
		for(int iRow = 1; iRow <= nrownum; iRow++){
			int offset = ncolnum * iRow;
			bigid = azResult[offset + 1] ? atoi(azResult[offset + 1]) : 0;

			strcpy(m_gbigname, azResult[offset + 2]);
			if(m_nowBigid != bigid || strcmp(m_nowBigName , m_gbigname)){
				kprintf(LOG_COMM,
					DCV_LOG_MODBUS,
					LOG_VIOLATION,
					"C2配置改变：规约号 %d =》 %d, 设备名 %s =》 %s  ", m_nowBigid,bigid,m_nowBigName,m_gbigname);

				m_hasChanged = 1;
			}
			strcpy(m_nowBigName, m_gbigname);
			m_nowBigid = bigid;
		}
	}
	sqlite3_close(m_pDB);
}catch(...){
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"打开数据库%s抛出错误,",dbPath);
}
	remove(dbPath);
	m_pDB=NULL;
}


//16进制转float
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

//16进制转int
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
		if(val>(tem-1)/2){
			val = val - tem;
		}
	}else{
		while(index<len){
			tem *= 256;
			val = val *256 + buf[index];
			index++;
		}
		if(val>(tem-1)/2){
			val = val - tem;
		}
	}
	return val;
}

//16进制转非负
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

int CC5Modbus::MakeWord(char type, short high, short low) {//生成测点数量
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
函数名称: str_to_hex 
函数功能: 字符串转换为十六进制 
输入参数: string 字符串 cbuf 十六进制 len 字符串的长度。 
输出参数: 无 
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

//其中的method需要与method.ini严格匹配，ini中的第一个，method0与case 0对应，以此类推
//且ini文件中method0就是第一个，method1就是第二个，不可乱序
//return 0为正常，return -1为异常
int CC5Modbus::analysisBuffer(unsigned char* buf, int method, int index, char cbdatahorl){
	m_iResult = 0;
	m_fResult = 0.0;
	switch(method){
		/*
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
		*/
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
		return yxbit(buf, index, cbdatahorl);
		break;
	case 15:
		return yxchar(buf, index, cbdatahorl);
		break;
	case 16:
		break;
	case 17:
		break;
	case 18:
		break;
	default:
		return -1;
	}
	return 0;
}
int CC5Modbus::Int2(unsigned char* pData, int index, char cbdatahorl){
	m_iResult = sixteen2int(pData + 2*index, 2, cbdatahorl);
	return 0;
}
int CC5Modbus::Int4(unsigned char* buf, int index, char cbdatahorl){
	m_iResult = sixteen2int(buf + 4*index, 4, cbdatahorl);
	return 0;
}
int CC5Modbus::UInt2(unsigned char* buf, int index, char cbdatahorl){
	m_iResult = sixteen2uint(buf + 2*index, 2, cbdatahorl);
	return 0;
}
int CC5Modbus::UInt4(unsigned char* buf, int index, char cbdatahorl){
	m_iResult = sixteen2uint(buf + 4*index, 4, cbdatahorl);
	return 0;
}
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
	m_fResult = total;
	return 0;
}
//IEEE-754浮点型数据格式
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
	m_fResult = YC_1;
	return 0;
}
int CC5Modbus::EX_INT8_PM800(unsigned char* buf, int index, char cbdatahorl){
	int t1=buf[0+8*index]*0x100+buf[1+8*index];
	int t2=buf[2+8*index]*0x100+buf[3+8*index];
	int t3=buf[4+8*index]*0x100+buf[5+8*index];
	int t4=buf[6+8*index]*0x100+buf[7+8*index];
	float val;
	if(cbdatahorl == 'h'){
		val=(t4+t3*10000+t2*100000000+(float)t1*1000000000000.0)/1000;//单位变成MW
	}else{
		val=(t1+t2*10000+t3*100000000+(float)t4*1000000000000.0)/1000;//单位变成MW
	}
	m_fResult = val;
	return 0;
}
//每一位代表一个遥信
int CC5Modbus::yxbit(unsigned char*buf, int index, char cbdatahorl){
	//int i = index / 8;
	sint16 value=buf[1]+buf[0]*0x100;//这里注意高低字节的顺序
	if(cbdatahorl == 'h'){
		value=buf[1]*0x100+buf[0];
	}
	value = value>>index;
	if (value&0x0001)
	    m_iResult = 1;
	else
	    m_iResult = 0;
	return 0;
}
//每个字节代表一个遥信
int CC5Modbus::yxchar(unsigned char* buf, int index, char cbdatahorl){
	sint16 value = buf[index/2];
	if(cbdatahorl == 'h'){
		if(index % 2 == 0){
			m_iResult = value & 0x0001;
		}else{
			m_iResult = value & 0x0100;
		}
	}else{
		if(index % 2 == 1){
			m_iResult = value & 0x0001;
		}else{
			m_iResult = value & 0x0100;
		}
	}
	return 0;
}

int CC5Modbus::NewIpsYc(unsigned char* pData, int index, char cbdatahorl){
	int t1;
	pData += 2 * index;
	t1=pData[0]*0x100+pData[1];
	
    m_iResult = t1;//单位kwh

	return 0;
}
