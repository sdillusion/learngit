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

#define DCV_LOG_MODBUS 8
const int CHECKCHANGEINTERVAL = 120;//单位:秒  检查变化的时间间隔，同时也是发送心跳报文的时间
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

//#ifdef DEBUG_MODE
	//char* tc = "test";
//	kprintf(LOG_COMM,
//			DCV_LOG_MODBUS,
//			LOG_INFORMATION,
//			"规约构造,%s",tc);
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
	//char* tc = "test";
	kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_INFORMATION,
			"规约初始化开始 ");
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
	m_lastcbdataTime = lastCheckChange = m_lastReqTime = m_lastSendTime = GetNowSecond();
	m_lastHertTime = 0;
	gdevonline = 0;
	gcbheartbeat = 0;
	gdtuonline = 0;

	
	if(m_timeOut < 100 || m_timeOut > 10000)
		m_timeOut = 303;

	greqInterval = m_timeOut/100 * 10;//前两位为周期
	m_timeOut = m_timeOut % 100 * 10;//后两位为超时时间

	getC2RtuInfo();
	isSendBuf = 1;//初始化时候不做检查判断，改为默认的发送报文

	char szapipath[MAX_PATH] = {0};//

	sint32 rtuno = pLink->GetRtuNo();

	ycssindex = -1;
	yxssindex = -1;
	ddssindex = -1;
	
	setCommonConfig();
/*
	char dbBasePath[128];
	char dbPath[128];
	sprintf(dbBasePath, "./%s.db", pPara);
	//sprintf(dbPath, "./%s%d.db", pPara, nowBigid);
	sprintf(dbPath, "./%s_%d.db", pPara, rtuno);
	CopyFile(dbBasePath, dbPath, false);


	//打开路径采用utf-8编码
    //如果路径中包含中文，需要进行编码转换
	int nRes = sqlite3_open(dbPath, &pDB);
	if (nRes != SQLITE_OK)
	{
	  kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"打开数据库%s失败,",dbPath);
	  return;
	}

	std::string bigsqlStr = bigmakeSqlStr(nowBigid);
	if(bigquery(bigsqlStr)){
		std::string ycsqlStr = ycmakeSqlStr(gbigid);
		ycquery(ycsqlStr);
		std::string yxsqlStr = yxmakeSqlStr(gbigid);
		yxquery(yxsqlStr);
		std::string ddsqlStr = ddmakeSqlStr(gbigid);
		ddquery(ddsqlStr);
	}
	sqlite3_close(pDB);
	remove(dbPath);
*/
	m_c5db = C5DB("111.231.135.99","wontex@1");
	//m_c5db = C5DB("127.0.0.1","Qwertyuiop1");
	if(m_c5db.openDB()){
		if(!C5DBbigQuery(nowBigid, m_c5db)){
			kprintf(LOG_COMM,
				DCV_LOG_MODBUS,
				LOG_ERROR,
				"规约配置big读取失败 rtu:%d", rtuno);
		}
		C5DBycGroupQuery(gbigid, m_c5db);
		C5DByxGroupQuery(gbigid, m_c5db);
		C5DBddGroupQuery(gbigid, m_c5db);
		int i,l;
		for(i = 0,l=ycssindex; i <= l ; i++){
			if(!C5DBycQuery(gbigid,m_ycss[i].id,i,m_c5db)){
				kprintf(LOG_COMM,
					DCV_LOG_MODBUS,
					LOG_ERROR,
					"规约配置yc读取失败 rtu:%d", rtuno);
			}
		}
		for(i = 0,l=yxssindex; i <= l ; i++){
			C5DByxQuery(gbigid,m_yxss[i].id,i,m_c5db);
		}
		for(i = 0,l=ddssindex; i <= l ; i++){
			C5DBddQuery(gbigid,m_ddss[i].id,i,m_c5db);
		}
		m_c5db.closeDB();
	}else{
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"规约配置读取失败 rtu:%d", rtuno);
	}

	m_askFrameType = YC_CYCLIC;

	gcuryc = 0;
	gcuryx = 0;
	gcurdd = 0;

	#ifdef DEBUG_MODE
	kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_INFORMATION,
			"规约初始化结束 rtu:%d", rtuno);
	#endif
}


//
sint32 CC5Modbus::RxProc()
{
	ParseFrame();
	return 1;
}

//
sint32 CC5Modbus::TxProc()
{
	int nowtime;
	//如果有改变则暂停发送报文
	if(isSendBuf == 0){
		nowtime = GetNowSecond();
		//当超过时限时发送心跳激活通道
		if(nowtime - lastCheckChange >= CHECKCHANGEINTERVAL){
			if(IsNeedToSendHeartBeat(nowtime)){
				SendHeartBeat();
			}
		}
		return 0;
	}
	nowtime = GetNowSecond();
	
	if(nowtime - lastCheckChange > CHECKCHANGEINTERVAL){
		checkChange();
	}

	if(IsNeedToSendHeartBeat(nowtime)){
		SendHeartBeat();
	}
	
	E_RAW_ComStat coms = pLink->GetCommStatus();
	if( coms != CMST_TX_CNT && coms != CMST_NORMAL){
		return 0;
	}

	if(!gdevonline || !gcbheartbeat){
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
	buf[1] = 0x03;
	if(gregisterhorl == 'l'){
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
	m_yxss[gcuryx].yxdatalen = yxnum * 2;

	if(yxnum <= 0 || yxnum > 0x1000)
		return;

	uint8	buf[64];
	buf[0] = rtuadd;
	buf[1] = 0x03;
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
	buf[1] = 0x03;
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
	if(gisregiste){
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
	int gregisteheadlen = strlen(gregistehead);
	int gregistetaillen = strlen(gregistetail);
	int gRegHeadLen = (gregisteheadlen-1)/2+1 + (gregistetaillen-1)/2+1 + 1 + IDMAXLEN;
	int i = 0;
	for(i = buflen - 1; i >= 0; i--){
		buf[gRegHeadLen + i] = buf[i];
	}

	
	str_to_hex(gregistehead,buf,gregisteheadlen);

	int strLen = strlen(gbigname);
	buf[(gregisteheadlen-1)/2+1] = strLen;

	for(i = 0; i < IDMAXLEN; i++){
		buf[i + (gregisteheadlen-1)/2+1 + 1]=0;
		if(i<strLen)
		{
			buf[i + (gregisteheadlen-1)/2+1 + 1]= gbigname[i];
		}
	}

	str_to_hex(gregistetail,buf + (gregisteheadlen-1)/2+1 + IDMAXLEN + 1, gregistetaillen);

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
		if(nowtime-m_lastHertTime>gheartbeatInterval){
			pRxBuf->Move(buflen);
			//pLink->SetCommStatus(CMST_TX_CNT);
		}
		return false;
	}
	datalen = pRxBuf->Read(buf,buflen,DEF_BUFF_NOMOVE);
	if(datalen != 4){
		pRxBuf->Move(buflen);
		//pLink->SetCommStatus(CMST_NORMAL);
		gdevonline = 0;
		return false;
	}

	int gcbheartsucceedlen = strlen(gcbheartsucceed);
	str_to_hex(gcbheartsucceed,gcbheartsucceedbuf,gcbheartsucceedlen);
	int gcbheartfaillen = strlen(gcbheartfail);
	str_to_hex(gcbheartfail,gcbheartfailbuf,gcbheartfaillen);
	
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
		//pLink->SetCommStatus(CMST_NORMAL);
		gcbheartbeat = 1;
		gdevonline = 0;
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
	//pLink->SetCommStatus(CMST_NORMAL);
	if(result){
		gcbheartbeat = 1;
		gdevonline = 1;
		return true;//返回发现心跳报文
	}

	return false;//返回没有发现心跳报文
}

bool CC5Modbus::IsNeedToSendHeartBeat(int now){
	if(!gisheartbeat){
		return false;
	}
	if(now - m_lastcbdataTime > gheartbeatInterval  && now - m_lastHertTime > gheartbeatInterval && now - m_lastSendTime > 1){
		return true;
	}

	//if(now - m_lastSendTime > gheartbeatInterval){
	//	return true;
	//}
	return false;
}

void CC5Modbus::SendHeartBeat(){
	int nowtime = GetNowSecond();

	sint32 rtuno = pLink->GetRtuNo();
	sint32 rtuadd = pRtu->GetRtuAddr(rtuno);

	uint8	buf[64];
	int gheartheadlen = strlen(ghearthead);
	str_to_hex(ghearthead,buf,gheartheadlen);

	int dataLen = gheartheadlen/2;
	sendMSG(buf,dataLen);

	gcbheartbeat = 0;

	m_lastHertTime = nowtime;

	return;
}

void CC5Modbus::ParseFrame()
{
	if(gisheartbeat && !gcbheartbeat){
		CheckHeartBeat();
		sint32 rtuno = pLink->GetRtuNo();
		int yxnum = pRtu->GetYxNum(rtuno);
		PRawCtrl->PutAYx(rtuno, yxnum-2, gdevonline);
		//pLink->SetCommStatus(CMST_TX_CNT);
		return;
	}

	int nowtime = GetNowSecond();

	sint32 rtuno = pLink->GetRtuNo();
	sint32 rtuaddr = pRtu->GetRtuAddr(rtuno);

	if(gdtuonline == 1 && nowtime - m_lastcbdataTime > DTUOFFLINETIME){
		gdtuonline = 0;
		int yxnum = pRtu->GetYxNum(rtuno);
		PRawCtrl->PutAYx(rtuno, yxnum-3, gdtuonline);
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
	#ifdef DEBUG_MODE
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"%d号终端 rtuaddr:%d 数据超时,数据长度：%d\n",
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
			"%d号终端 rtuaddr:%d 收到数据,数据长度：%d,   count:%d    ",
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
			"%d号终端缓冲区越限,rcv_NUM=%d,buf_len=2048\n", rtuno,buflen);
	#endif
		pRxBuf->Move(buflen);
		pLink->SetCommStatus(CMST_NORMAL);
		return ;
	}

	
	//m_lastReqTime = nowtime;
	ProcessBuf(buf, rtuno,  buflen,  datalen);
	m_lastcbdataTime = nowtime;
	if(gdtuonline == 0){
		gdtuonline = 1;
		int yxnum = pRtu->GetYxNum(rtuno);
		PRawCtrl->PutAYx(rtuno, yxnum-3, gdtuonline);
	}
}

void CC5Modbus::ProcessBuf(uint8* buf, sint32 rtuno, int buflen, int datalen){
	switch(m_askFrameType)
	{
	case YC_FRAME:
	case YC_CYCLIC:
		ProcessYc(buf, datalen);
		gcuryc++;
		//if(gcuryc >= m_ycsectionNum){
		//	gcuryc = 0;
		//}
		break;
	case YX_FRAME:
	case YX_CYCLIC:
		ProcessYx(buf, datalen);
		gcuryx++;
		//if(gcuryx >= m_yxsectionNum){
		//	gcuryx = 0;
		//}
		break;
	case DD_FRAME:
	case DD_CYCLIC:
		ProcessDD(buf, datalen);
		gcurdd++;
		//if(gcurdd >= m_ddsectionNum){
		//	gcurdd = 0;
		//}
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
						"CRC校验失败：%d",ycsec.addr);
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
						"yc点号没有填写：%s, %d, %d ",ycstr.desc, gcuryc, i);
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
						"YX数据数校验失败：%d ,%d ,%d ,%d ,%d",yxsec.addr,yxsec.yxdatalen,
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
						"CRC校验失败：%d",yxsec.addr);
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
						"遥信报文解析出错：%s,  ",e.what());
			yxstr = yxsec.yxs[0];
		}
		
		PRawCtrl->GetAYx(rtuno, i, &oldyxval);
		if(oldyxval != newyxval){
			S_RAWSOE rawsoe;
			CJskTime	JTime;
			SJSK_CLOCK NowTime;
			JTime.GetNow(&NowTime);//获得当前时间
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
						"点号没有填写：%s, %d, %d ",yxstr.desc, gcuryx, i);
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
						"CRC校验失败：%d",ddsec.addr);
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
						"点号没有填写：%s , %d, %d ",ddstr.desc, gcurdd, i);
			}
			putvalindex++;
		}
	}
	pLink->RegisterFrmCode(RAW_CODEDIR_UP, (char*)buf, datalen);
	pLink->RegisterFrm(FRAME_RX_SUC);
}

//以下为配套函数，包含查询sqlite数据库

void CC5Modbus::setCommonConfig(){
	//gcbtimeout =  4;
	gisregiste = 1;
	gisheartbeat = 1;//是否有心跳报文
	//grutInterval = 3;//RTU时间间隔
	
	
	gheartbeatInterval = 12;//心跳报文间隔
	//gcbdatalenbit = 2;//返回报文数据数量所占位数
	
	strcpy(ghearthead,"eeeeeeee");//心跳报文内容
	strcpy(gregistehead,"eb90eb90");//注册报文头
	strcpy(gregistetail,"ea80ea80");//注册报文尾
	strcpy(gcbheartsucceed,"eeee01ee");//心跳注册成功返回报文
	strcpy(gcbheartfail,"eeee00ee");//心跳注册失败返回报文


	gregisterhorl = 'h';//寄存器地址高低位规则
	//gcbdatalenhorl = 'l';//返回数据长度高低位规则
}

std::string CC5Modbus::bigmakeSqlStr(int rtuno){
	std::string sqlStr =  "select id,desc,sort,addrhorl,gcheckcrc,version from big where id = ";
	char s[12];
    itoa(rtuno,s,10);
	std::string strTmp = s;
	sqlStr += strTmp;
	return sqlStr;
}

bool CC5Modbus::bigquery(const std::string& sqlStr)
{
  char* cErrMsg;
  int nrownum = 0, ncolnum = 0;  
  char **azResult;
 
  int nRes = sqlite3_get_table(pDB, sqlStr.c_str(), &azResult, &nrownum, &ncolnum, &cErrMsg);
  if (nRes != SQLITE_OK)
  {
	kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"查询数据库big表失败，%s",cErrMsg);
    return false;
  }else if(nrownum < 1){
	kprintf(LOG_COMM,
		DCV_LOG_MODBUS,
		LOG_VIOLATION,
		"查询数据库big表没有数据");
	return false;
  }else{
	bigResultFormat(nrownum, ncolnum, azResult);
  }
  return true;
}
void CC5Modbus::bigResultFormat(int nrownum, int ncolnum, char **argv){
	
	for(int iRow = 1; iRow <= nrownum; iRow++){
		int offset = ncolnum * iRow;
		
		nowVersion = argv[offset + 5] ? atoi(argv[offset + 5]) : 0;
		
		gregisterhorl = *argv[offset + 3];
		strcpy(gbigid, argv[offset + 0]);
		gcheckcrc = argv[offset + 4] ? atoi(argv[offset + 4]) : 0;
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
std::string CC5Modbus::ycmakeSqlStr(char* bigid){
	std::string sqlStr =  "select bigid,id,desc,addr,coe,relation,length,horl,method,used,sort,startindex,cbdatalenbit,cbdatahorl,hasnan,nankey,nanvalue from yc where bigid = ";
	std::string strTmp = bigid;
	sqlStr += strTmp;
	return sqlStr;
}

bool CC5Modbus::ycquery(const std::string& sqlStr)
{
  char* cErrMsg = 0;

  int nrownum = 0, ncolnum = 0;  
  char **azResult;
  int nRes = sqlite3_get_table(pDB, sqlStr.c_str(), &azResult, &nrownum, &ncolnum, &cErrMsg);
  if (nRes != SQLITE_OK)
  {
	 kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"查询数据库yc表失败，%s",cErrMsg);
    return false;
  }else{
	  ycResultFormat(nrownum, ncolnum, azResult);
  }
  return true;
}
void CC5Modbus::ycResultFormat(int nrownum, int ncolnum, char **argv){
	if(nrownum < 1){
		m_ycsectionNum = 0;
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"查询数据库yc表没有数据");
		return;
	}
	for(int iRow = 1; iRow <= nrownum; iRow++){
		int offset = ncolnum * iRow;
		if(argv[offset + 3] && *argv[offset + 3]){
			ycssindex++;
			ycssindex = ycssindex;
			m_ycss[ycssindex].addr = atoi(argv[offset + 3]);
			ycsindex = 0;
			
			m_ycss[ycssindex].startindex = argv[offset + 11] ? atoi(argv[offset + 11]) : 0;
			m_ycss[ycssindex].method = atoi(argv[offset + 8]);
			m_ycss[ycssindex].cbdatalenbit = argv[offset + 12] ? atoi(argv[offset + 12]) : 1;



			m_ycss[ycssindex].cbdatahorl = argv[offset + 13] ? *argv[offset + 13] : 'h'; 

			m_ycss[ycssindex].hasnan = argv[offset + 14] ? atoi(argv[offset + 14]) : 0;
			m_ycss[ycssindex].nankey = argv[offset + 15] ? atoi(argv[offset + 15]) : 0;
			m_ycss[ycssindex].nanvalue = argv[offset + 16] ? atoi(argv[offset + 16]) : 0;
		}
			
		strcpy(m_ycss[ycssindex].ycs[ycsindex].id, argv[offset + 1]);
		strcpy(m_ycss[ycssindex].ycs[ycsindex].desc, argv[offset + 2]);
		m_ycss[ycssindex].ycs[ycsindex].coe = argv[offset + 4] ? atof(argv[offset + 4]) : 1.0;
		m_ycss[ycssindex].ycs[ycsindex].rel = argv[offset + 5] ? atoi(argv[offset + 5]) : -1;
		m_ycss[ycssindex].ycs[ycsindex].length = argv[offset + 6] ? atoi(argv[offset + 6]) : 2;
		m_ycss[ycssindex].ycs[ycsindex].horl = *argv[offset + 7];
		m_ycss[ycssindex].ycs[ycsindex].method = atoi(argv[offset + 8]);
		m_ycss[ycssindex].ycs[ycsindex].used = atoi(argv[offset + 9]);
		m_ycss[ycssindex].ycs[ycsindex].sort = atoi(argv[offset + 10]);
		m_ycss[ycssindex].ycs[ycsindex].dit = argv[offset + 11] ? atoi(argv[offset + 11]) : -1;
		ycsindex++;
		m_ycss[ycssindex].ycslen = ycsindex;

		m_ycsectionNum = ycssindex + 1;
	}
}

std::string CC5Modbus::yxmakeSqlStr(char* bigid){
	std::string sqlStr =  "select bigid,id,desc,addr,horl,method,used,sort,startindex,offset,yxnum,cbdatalenbit,cbdatahorl from yx where bigid = ";
	std::string strTmp = bigid;
	sqlStr += strTmp;
	return sqlStr;
}

bool CC5Modbus::yxquery(const std::string& sqlStr)
{
  char* cErrMsg = 0;

  int nrownum = 0, ncolnum = 0;  
  char **azResult;
 
  int nRes = sqlite3_get_table(pDB, sqlStr.c_str(), &azResult, &nrownum, &ncolnum, &cErrMsg);
  if (nRes != SQLITE_OK)
  {
	kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"查询数据库yx表失败，%s",cErrMsg);
    return false;
  }else{
	yxResultFormat(nrownum, ncolnum, azResult);
  }
  return true;
}
void CC5Modbus::yxResultFormat(int nrownum, int ncolnum, char **argv){
	if(nrownum < 1){
		m_yxsectionNum = 0;
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"查询数据库yx表没有数据");
		return;
	}
	for(int iRow = 1; iRow <= nrownum; iRow++){
		int offset = ncolnum * iRow;
		if(argv[offset + 3] && *argv[offset + 3]){
			yxssindex++;
			yxssindex = yxssindex;
			m_yxss[yxssindex].addr = atoi(argv[offset + 3]);
			yxsindex = 0;
			m_yxss[yxssindex].startindex = argv[offset + 8] ? atoi(argv[offset + 8]) : 0;
			m_yxss[yxssindex].yxnum = argv[offset + 10] ? atoi(argv[offset + 10]) : 0;
			m_yxss[yxssindex].offset = argv[offset + 9] ? atoi(argv[offset + 9]) : 0;
			m_yxss[yxssindex].method = atoi(argv[offset + 5]);
			m_yxss[yxssindex].cbdatalenbit = argv[offset + 11] ? atoi(argv[offset + 11]) : 1;

			m_yxss[yxssindex].cbdatahorl = argv[offset + 12] ? *argv[offset + 12] : 'h';
		}
			
		strcpy(m_yxss[yxssindex].yxs[yxsindex].id, argv[offset + 1]);
		strcpy(m_yxss[yxssindex].yxs[yxsindex].desc, argv[offset + 2]);
		m_yxss[yxssindex].yxs[yxsindex].horl = *argv[offset + 4];
		m_yxss[yxssindex].yxs[yxsindex].used = argv[offset + 6] ? atoi(argv[offset + 6]) : 0;
		m_yxss[yxssindex].yxs[yxsindex].dit = argv[offset + 8] ? atoi(argv[offset + 8]) : -1;
		yxsindex++;
		m_yxss[yxssindex].yxslen = yxsindex;

		m_yxsectionNum = yxssindex + 1;
	}
}

std::string CC5Modbus::ddmakeSqlStr(char* bigid){
	std::string sqlStr =  "select bigid,id,desc,addr,coe,relation,length,horl,method,used,sort,startindex,reqnum,cbdatalenbit,cbdatahorl,hasnan,nankey,nanvalue from dd where bigid = ";
	std::string strTmp = bigid;
	sqlStr += strTmp;
	return sqlStr;
}

bool CC5Modbus::ddquery(const std::string& sqlStr)
{
  char* cErrMsg;

  int nrownum = 0, ncolnum = 0;  
  char **azResult;
 
  int nRes = sqlite3_get_table(pDB, sqlStr.c_str(), &azResult, &nrownum, &ncolnum, &cErrMsg);
  if (nRes != SQLITE_OK)
  {
	kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"查询数据库dd表失败，%s",cErrMsg);
    return false;
  }else{
	ddResultFormat(nrownum, ncolnum, azResult);
  }
  return true;
}
void CC5Modbus::ddResultFormat(int nrownum, int ncolnum, char **argv){
	if(nrownum < 1){
		m_ddsectionNum = 0;
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"查询数据库dd表没有数据");
		return;
	}
	for(int iRow = 1; iRow <= nrownum; iRow++){
		int offset = ncolnum * iRow;
		if(argv[offset + 3] && *argv[offset + 3]){
			ddssindex++;
			ddssindex = ddssindex;
			m_ddss[ddssindex].addr = atoi(argv[offset + 3]);
			ddsindex = 0;
			m_ddss[ddssindex].startindex = argv[offset + 11] ? atoi(argv[offset + 11]) : 0;
			m_ddss[ddssindex].reqnum = argv[offset + 12] ? atoi(argv[offset + 12]) : 0;
			m_ddss[ddssindex].method = atoi(argv[offset + 8]);
			m_ddss[ddssindex].cbdatalenbit = argv[offset + 13] ? atoi(argv[offset + 13]) : 1;



			m_ddss[ddssindex].cbdatahorl = argv[offset + 14] ? *argv[offset + 14] : 'h';

			m_ddss[ddssindex].hasnan = argv[offset + 15] ? atoi(argv[offset + 15]) : 0;
			m_ddss[ddssindex].nankey = argv[offset + 16] ? atoi(argv[offset + 16]) : 0;
			m_ddss[ddssindex].nanvalue = argv[offset + 17] ? atoi(argv[offset + 17]) : 0;
		}
			
		strcpy(m_ddss[ddssindex].dds[ddsindex].id, argv[offset + 1]);
		strcpy(m_ddss[ddssindex].dds[ddsindex].desc, argv[offset + 2]);
		m_ddss[ddssindex].dds[ddsindex].coe = argv[offset + 4] ? atof(argv[offset + 4]) : 1.0;
		m_ddss[ddssindex].dds[ddsindex].rel = argv[offset + 5] ? atoi(argv[offset + 5]) : -1;
		m_ddss[ddssindex].dds[ddsindex].length = argv[offset + 6] ? atoi(argv[offset + 6]) : 2;
		m_ddss[ddssindex].dds[ddsindex].horl = *argv[offset + 7];
		m_ddss[ddssindex].dds[ddsindex].method = atoi(argv[offset + 8]);
		m_ddss[ddssindex].dds[ddsindex].used = atoi(argv[offset + 9]);
		m_ddss[ddssindex].dds[ddsindex].sort = atoi(argv[offset + 10]);
		m_ddss[ddssindex].dds[ddsindex].dit = argv[offset + 11] ? atoi(argv[offset + 11]) : -1;
		ddsindex++;
		m_ddss[ddssindex].ddslen = ddsindex;

		m_ddsectionNum = ddssindex + 1;
	}
}





bool CC5Modbus::C5DBycGroupQuery(char* bigid, C5DB c5db){
	CString id = bigid;
	//gYcType.ycseclen = 0;
	//CString sqlStr = "select * from ycgroup where bigid = '" + id +"'";

	char* ycGroupHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2002_CODE) as F2002_CODE,RTRIM(F2002_DESC) as F2002_DESC,F2002_ADDR,F2002_RXDATALEN,F2002_DATALENHL,F2002_METHOD,F2002_RXDATALENBIT,F2002_RXDATALENBITHL,F2002_HASINVALIDVAL,F2002_INVALIDIFVAL,F2002_INVALIDREVAL";
	CString sqlStr;
	sqlStr.Format("select %s from TB2002_YCGROUP where F2001_CODE = '%s' order by CONVERT(int, F2002_CODE)", ycGroupHeadName, id);

	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			//gYcType.ycseclen++;
			//int ycssindex = gYcType.ycseclen-1;
			ycssindex++;
			m_ycss[ycssindex].ycslen = 0;
			varName = m_pRecordset->GetCollect ("F2002_ADDR");
			m_ycss[ycssindex].addr = _ttoi((char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("addr");
			m_ycss[ycssindex].startindex =  0;
			varName = m_pRecordset->GetCollect ("F2002_RXDATALEN");
			m_ycss[ycssindex].ycdatalen = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_RXDATALENBIT");
			m_ycss[ycssindex].cbdatalenbit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 1;
			varName = m_pRecordset->GetCollect ("F2002_METHOD");
			m_ycss[ycssindex].method = _ttoi((char *)_bstr_t(varName));

			varName = m_pRecordset->GetCollect ("F2002_DATALENHL");
			m_ycss[ycssindex].horl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2002_RXDATALENBITHL");
			m_ycss[ycssindex].cbdatahorl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2002_HASINVALIDVAL");
			m_ycss[ycssindex].hasnan = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_INVALIDIFVAL");
			m_ycss[ycssindex].nankey = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_INVALIDREVAL");
			m_ycss[ycssindex].nanvalue = _ttoi((char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("F2002_DESC");
			//strcpy(m_ycss[ycssindex].desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2002_CODE");
			strcpy(m_ycss[ycssindex].id, (char *)_bstr_t(varName));

			/*bigstruct bigstr;
			_variant_t varName;
			varName = m_pRecordset->GetCollect ("id");
			strcpy(bigstr.id, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("describe");
			strcpy(bigstr.desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("addrhorl");
			bigstr.addrhorl = *(char *)_bstr_t(varName);
			varName = m_pRecordset->GetCollect ("gcheckcrc");
			bigstr.checkcrc = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("version");
			bigstr.version = _ttoi((char *)_bstr_t(varName));
			vbig.push_back(bigstr);
			*/

			//cout<<strName.GetBuffer(0)<<endl;
			//OutputDebugString(strName.GetBuffer(0));
			//OutputDebugString("\n");

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		m_ycsectionNum = ycssindex + 1;
		return true;
	}else{
		m_ycsectionNum = 0;
	}
	return false;
}
bool CC5Modbus::C5DByxGroupQuery(char* bigid, C5DB c5db){
	CString id = bigid;
	//gYxType.yxseclen = 0;
	//CString sqlStr = "select * from yxgroup where bigid = '" + id +"'";

	char * yxgroupHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2004_CODE) as F2004_CODE,RTRIM(F2004_DESC) as F2004_DESC,F2004_ADDR,F2004_DATALENHL,F2004_METHOD,F2004_YXNUM,F2004_RXDATALENBIT,F2004_RXDATALENBITHL";
	CString sqlStr;
	sqlStr.Format("select %s from TB2004_YXGROUP where F2001_CODE = '%s' order by CONVERT(int, F2004_CODE)", yxgroupHeadName, id);

	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			//gYxType.yxseclen++;
			//int yxssindex = gYxType.yxseclen-1;
			yxssindex++;
			m_yxss[yxssindex].yxslen = 0;

			varName = m_pRecordset->GetCollect ("F2004_ADDR");
			m_yxss[yxssindex].addr = _ttoi((char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("addr");
			m_yxss[yxssindex].startindex =  0;
			
			varName = m_pRecordset->GetCollect ("F2004_RXDATALENBIT");
			m_yxss[yxssindex].cbdatalenbit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 1;
			varName = m_pRecordset->GetCollect ("F2004_METHOD");
			m_yxss[yxssindex].method = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2004_YXNUM");
			m_yxss[yxssindex].yxnum = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 0;

			varName = m_pRecordset->GetCollect ("F2004_DATALENHL");
			m_yxss[yxssindex].horl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2004_RXDATALENBITHL");
			m_yxss[yxssindex].cbdatahorl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			
			//varName = m_pRecordset->GetCollect ("F2004_DESC");
			//strcpy(m_yxss[yxssindex].desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2004_CODE");
			strcpy(m_yxss[yxssindex].id, (char *)_bstr_t(varName));

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		m_yxsectionNum = yxssindex + 1;
		return true;
	}else{
		m_yxsectionNum = 0;
	}
	return false;
}
bool CC5Modbus::C5DBddGroupQuery(char* bigid, C5DB c5db){
	CString id = bigid;
	//gDdType.ddseclen = 0;
	//CString sqlStr = "select * from ddgroup where bigid = '" + id +"'";

	char * ddGroupHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2006_CODE) as F2006_CODE,RTRIM(F2006_DESC) as F2006_DESC,F2006_ADDR,F2006_RXDATALEN,F2006_DATALENHL,F2006_METHOD,F2006_RXDATALENBIT,F2006_RXDATALENBITHL,F2006_HASINVALIDVAL,F2006_INVALIDIFVAL,F2006_INVALIDREVAL";
	CString sqlStr;
	sqlStr.Format("select %s from TB2006_DDGROUP where F2001_CODE = '%s' order by CONVERT(int, F2006_CODE)",ddGroupHeadName, id);

	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			//gDdType.ddseclen++;
			//int ddssindex = gDdType.ddseclen-1;
			ddssindex++;
			m_ddss[ddssindex].ddslen = 0;
			varName = m_pRecordset->GetCollect ("F2006_ADDR");
			m_ddss[ddssindex].addr = _ttoi((char *)_bstr_t(varName));
			m_ddss[ddssindex].startindex =  0;
			varName = m_pRecordset->GetCollect ("F2006_RXDATALEN");
			m_ddss[ddssindex].dddatalen = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_RXDATALENBIT");
			m_ddss[ddssindex].cbdatalenbit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 1;
			varName = m_pRecordset->GetCollect ("F2006_METHOD");
			m_ddss[ddssindex].method = _ttoi((char *)_bstr_t(varName));

			varName = m_pRecordset->GetCollect ("F2006_DATALENHL");
			m_ddss[ddssindex].horl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2006_RXDATALENBITHL");
			m_ddss[ddssindex].cbdatahorl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2006_HASINVALIDVAL");
			m_ddss[ddssindex].hasnan = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_INVALIDIFVAL");
			m_ddss[ddssindex].nankey = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_INVALIDREVAL");
			m_ddss[ddssindex].nanvalue = _ttoi((char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("F2006_DESC");
			//strcpy(m_ddss[ddssindex].desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2006_CODE");
			strcpy(m_ddss[ddssindex].id, (char *)_bstr_t(varName));

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		m_ddsectionNum = ddssindex + 1;
		return true;
	}else{
		m_ddsectionNum = 0;
	}
	return false;
}
bool CC5Modbus::C5DBycQuery(char* bigid, char* groupid, int ycssindex, C5DB c5db){
	CString id1 = bigid;
	CString id2= groupid;
	//gYcType.ycseclen = 0;
	//CString sqlStr = "select * from yc where bigid = '" + id1 +"'" + " and groupid = '" + id2 + "'";

	char* ycHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2002_CODE) as F2002_CODE,RTRIM(F2003_CODE) as F2003_CODE,RTRIM(F2003_DESC) as F2003_DESC,F2003_COE,F2003_USED,F2003_POINTNO";
	CString sqlStr;
	sqlStr.Format("select %s from TB2003_YCPOINT where F2001_CODE = '%s' and F2002_CODE = '%s' order by CONVERT(int, F2003_CODE)", ycHeadName, id1, id2);

	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			//gYcType.ycseclen++;
			//ycssindex = gYcType.ycseclen-1;
			m_ycss[ycssindex].ycslen++;
			int ycsindex = m_ycss[ycssindex].ycslen-1;

			varName = m_pRecordset->GetCollect ("F2003_CODE");
			strcpy(m_ycss[ycssindex].ycs[ycsindex].id, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2003_DESC");
			strcpy(m_ycss[ycssindex].ycs[ycsindex].desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2003_COE");
			m_ycss[ycssindex].ycs[ycsindex].coe = (char *)_bstr_t(varName) ? atof((char *)_bstr_t(varName)) : 1.0;
			varName = m_pRecordset->GetCollect ("F2003_USED");
			m_ycss[ycssindex].ycs[ycsindex].used = _ttoi((char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("F2003_SORT");
			//m_ycss[ycssindex].ycs[ycsindex].sort = _ttoi((char *)_bstr_t(varName));
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
	//gYcType.ycseclen = 0;
	//CString sqlStr = "select * from yx where bigid = '" + id1 +"'" + " and groupid = '" + id2 + "'";
	char * yxHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2004_CODE) as F2004_CODE,RTRIM(F2005_CODE) as F2005_CODE,RTRIM(F2005_DESC) as F2005_DESC,F2005_USED,F2005_POINTNO";
	CString sqlStr;
	sqlStr.Format("select %s from TB2005_YXPOINT where F2001_CODE = '%s' and F2004_CODE = '%s' order by CONVERT(int, F2005_CODE)", yxHeadName, id1, id2);
	
	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			//gYxType.yxseclen++;
			m_yxss[yxssindex].yxslen++;
			int yxsindex = m_yxss[yxssindex].yxslen-1;

			varName = m_pRecordset->GetCollect ("F2005_CODE");
			strcpy(m_yxss[yxssindex].yxs[yxsindex].id, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2005_DESC");
			strcpy(m_yxss[yxssindex].yxs[yxsindex].desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2005_USED");
			m_yxss[yxssindex].yxs[yxsindex].used = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2005_POINTNO");
			m_yxss[yxssindex].yxs[yxsindex].dit = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : -1;

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
	//gYcType.ycseclen = 0;
	//CString sqlStr = "select * from dd where bigid = '" + id1 +"'" + " and groupid = '" + id2 + "'";
	char * ddHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2006_CODE) as F2006_CODE,RTRIM(F2007_CODE) as F2007_CODE,RTRIM(F2007_DESC) as F2007_DESC,F2007_COE,F2007_USED,F2007_POINTNO";

	CString sqlStr;
	sqlStr.Format("select %s from TB2007_DDPOINT where F2001_CODE = '%s' and F2006_CODE = '%s' order by CONVERT(int, F2007_CODE)", ddHeadName, id1, id2);
	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			_variant_t varName;
			//gDdType.ddseclen++;
			m_ddss[ddssindex].ddslen++;
			int ddsindex = m_ddss[ddssindex].ddslen-1;

			varName = m_pRecordset->GetCollect ("F2007_CODE");
			strcpy(m_ddss[ddssindex].dds[ddsindex].id, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2007_DESC");
			strcpy(m_ddss[ddssindex].dds[ddsindex].desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2007_COE");
			m_ddss[ddssindex].dds[ddsindex].coe = (char *)_bstr_t(varName) ? atof((char *)_bstr_t(varName)) : 1.0;
			varName = m_pRecordset->GetCollect ("F2007_USED");
			m_ddss[ddssindex].dds[ddsindex].used = _ttoi((char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("sort");
			//m_ddss[ddssindex].dds[ddsindex].sort = _ttoi((char *)_bstr_t(varName));
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
	//CString sqlStr = "select * from big order by id asc";
	char s[12];
    itoa(bigid,s,10);
	CString ss = s;
	//CString sqlStr = "select id,describe,sort,addrhorl,gcheckcrc,version from big where id = '" + ss + "'";

	char* bigHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2001_DESC) as F2001_DESC,F2001_ADDRHL,F2001_CRC,F2001_VERSION,F2001_USED,RTRIM(F2001_REMARK) as F2001_REMARK,RTRIM(F1102_CODE) as F1102_CODE";
	CString sqlStr;
	sqlStr.Format("select %s from TB2001_PROTOCOL where F2001_CODE = %s", bigHeadName, ss);

	//return 0;

	_RecordsetPtr m_pRecordset;
	if(c5db.querySQL(sqlStr, m_pRecordset)){
		while(!m_pRecordset->GetadoEOF()){
			//bigstruct bigstr;
			
			_variant_t varName;
			varName = m_pRecordset->GetCollect ("F2001_CODE");
			strcpy(gbigid, (char *)_bstr_t(varName));
			//varName = m_pRecordset->GetCollect ("describe");
			//strcpy(bigstr.desc, (char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2001_ADDRHL");
			gregisterhorl = (char *)_bstr_t(varName) ? *(char *)_bstr_t(varName) : 'h';
			varName = m_pRecordset->GetCollect ("F2001_CRC");
			gcheckcrc = _ttoi((char *)_bstr_t(varName));
			varName = m_pRecordset->GetCollect ("F2001_VERSION");
			nowVersion = _ttoi((char *)_bstr_t(varName));
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
			
			//vbig.push_back(bigstr);

			//cout<<strName.GetBuffer(0)<<endl;
			//OutputDebugString(strName.GetBuffer(0));
			//OutputDebugString("\n");

			m_pRecordset->MoveNext();
		}
		m_pRecordset->Close();
		m_pRecordset.Release();
		return true;
	}
	return false;
}


void CC5Modbus::checkChange(){
	
	lastCheckChange = GetNowSecond();
	int version;
/*
	char dbBasePath[128];
	char dbPath[128];
	sint32 rtuno = pLink->GetRtuNo();
	sprintf(dbBasePath, "./%s.db", pPara);
	sprintf(dbPath, "./%s_%d.db", pPara, rtuno);
	CopyFile(dbBasePath, dbPath, false);


	//打开路径采用utf-8编码
	//如果路径中包含中文，需要进行编码转换
	int nRes = sqlite3_open(dbPath, &pDB);
	if (nRes != SQLITE_OK)
	{
	  kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"打开数据库%s失败,",dbPath);
	  return;
	}
	std::string bigsqlStr = bigmakeSqlStr(nowBigid);
	char* cErrMsg;
	int nrownum = 0, ncolnum = 0;  
	char **azResult;

	nRes = sqlite3_get_table(pDB, bigsqlStr.c_str(), &azResult, &nrownum, &ncolnum, &cErrMsg);
	if (nRes != SQLITE_OK)
	{
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"查询数据库big表失败，%s",cErrMsg);
	}else if(nrownum < 1){
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"查询数据库big表没有数据");
	}else{
		//bigResultFormat(nrownum, ncolnum, azResult);
		for(int iRow = 1; iRow <= nrownum; iRow++){
			int offset = ncolnum * iRow;
			version = azResult[offset + 5] ? atoi(azResult[offset + 5]) : 0;
		}
	}
	sqlite3_close(pDB);
	remove(dbPath);
*/

	if(m_c5db.openDB()){
		
		char s[12];
		itoa(nowBigid,s,10);
		CString ss = s;
		//CString sqlStr = "select id,describe,sort,addrhorl,gcheckcrc,version from big where id = '" + ss + "'";
		char* bigHeadName = "RTRIM(F2001_CODE) as F2001_CODE,RTRIM(F2001_DESC) as F2001_DESC,F2001_ADDRHL,F2001_CRC,F2001_VERSION,F2001_USED,RTRIM(F2001_REMARK) as F2001_REMARK,RTRIM(F1102_CODE) as F1102_CODE";
		CString sqlStr;
		sqlStr.Format("select %s from TB2001_PROTOCOL where F2001_CODE = %s ", bigHeadName, ss);

		_RecordsetPtr m_pRecordset;
		if(m_c5db.querySQL(sqlStr, m_pRecordset)){
			while(!m_pRecordset->GetadoEOF()){
				//bigstruct bigstr;
				
				_variant_t varName;
				
				varName = m_pRecordset->GetCollect ("F2001_VERSION");
				version = (char *)_bstr_t(varName) ? _ttoi((char *)_bstr_t(varName)) : 0;

				m_pRecordset->MoveNext();
			}
			m_pRecordset->Close();
			m_pRecordset.Release();
		}else{
			kprintf(LOG_COMM,
				DCV_LOG_MODBUS,
				LOG_ERROR,
				"查找是否有更改时，规约配置读取失败 %s", sqlStr);
		}
		m_c5db.closeDB();
	}else{
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"查找是否有更改时，规约配置打开数据库失败");
	}

	if(nowVersion != version){
		isSendBuf = 0;
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_VIOLATION,
			"C2配置改变：版本号 %d =》 %d  ", nowVersion,version);
	}
	nowVersion = version;
	getC2RtuInfo();
}

//获取RTU信息，并检查 bigid、bigname 是否有改变
void CC5Modbus::getC2RtuInfo(){
	int bigid,name;
	char dbPath[128];
	char* envvar = getenv("C2PLAT");
	if( envvar == NULL ){
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"获取环境变量C2PLAT失败！   ");
		return;
	}
	sprintf(dbPath, "%s/db/jskpara.db", envvar);

	int nRes = sqlite3_open(dbPath, &pDB);

	char* cErrMsg;
	int nrownum = 0, ncolnum = 0;  
	char **azResult;
	sint32 rtuno = pLink->GetRtuNo();
	char str[10];
	sprintf(str, "%d", rtuno); //将100转为16进制表示的字符串。
	std::string sqlStr = "select 装置序号,召全数据间隔,召电度间隔 from T004_装置表 where 装置序号=";
	sqlStr += str;
	nRes = sqlite3_get_table(pDB , sqlStr.c_str() , &azResult, &nrownum, &ncolnum, &cErrMsg);
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
			name = azResult[offset + 2] ? atoi(azResult[offset + 2]) : 0;
			
			sprintf(gbigname,"%05d",name);
			kprintf(2,8,LOG_INFORMATION,"******ID:%s", gbigname);
			if(nowBigid != bigid || nowBigname != name){
				isSendBuf = 0;
				kprintf(LOG_COMM,
					DCV_LOG_MODBUS,
					LOG_VIOLATION,
					"C2配置改变：规约号 %d =》 %d, 设备名 %d =》 %d  ", nowBigid,bigid,nowBigname,name);
			}
			nowBigname = name;
			nowBigid = bigid;
		}
	}
	sqlite3_close(pDB);
	pDB=NULL;
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
		break;
	case 17:
		break;
	case 18:
		break;
	default:
		kprintf(LOG_COMM,
			DCV_LOG_MODBUS,
			LOG_ERROR,
			"传入的解析方法错误");
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
	iResult = pData[2*index+1] + pData[2*index]*256;
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
	float val=(t1+t2*10000+t3*100000000+(float)t4*1000000000000.0)/1000;//单位变成MW
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
		val=(t4+t3*10000+t2*100000000+(float)t1*1000000000000.0)/1000;//单位变成MW
	}else{
		val=(t1+t2*10000+t3*100000000+(float)t4*1000000000000.0)/1000;//单位变成MW
	}
	fResult = val;
	return 0;
}
//每一位代表一个遥信
int CC5Modbus::yx4bit(unsigned char*buf, int index, char cbdatahorl){
	//int i = index / 8;
	sint16 value=buf[1]+buf[0]*0x100;//这里注意高低字节的顺序
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
//每个字节代表一个遥信
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

int CC5Modbus::IpsYc(unsigned char* pData, int index, char cbdatahorl){
	sint16 value=0;
	int len_num=0;

	switch(gcuryc)
	{
	case 0://电流
		len_num=4;
		break;
	case 1://电压
		len_num=7;
		break;
	case 2://功率
		len_num=12;
		break;
	case 3://功率因数
		len_num=5;
		break;
	case 4://频率
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

	sint16 value=pData[1]+pData[0]*0x100;//这里注意高低字节的顺序
	
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
    fResult = t1/1000+t2*10+t3*100000+t4*1000000000;//单位kwh

	return 0;
}

int CC5Modbus::NewIpsYc(unsigned char* pData, int index, char cbdatahorl){
	int t1;
	pData += 2 * index;
	t1=pData[0]*0x100+pData[1];
	
    iResult = t1;//单位kwh

	return 0;
}
