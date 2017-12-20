#if defined(WIN32)
#include    <io.h>
#endif

#include    "agc.h"
#include "plat_log.h"

#include<string>

using namespace std;


void CZfAgc::Init( S_PROTOCOLCFG * pcfg )
{
	PRawCtrl	=	pcfg->PRawCtrl;
	pRxBuf		=	pcfg->pRxBuf;
	pTxBuf		=	pcfg->pTxBuf;
	pCmdMem		=	pcfg->pCmdMem;
	pRtu		=	pcfg->pRtu;
	pLink		=	pcfg->pLink;
	pTable		=	pcfg->pTable;
	pZfDataCtrl	=	pcfg->pZfDataCtrl;

	int buflen=pRxBuf->GetReadableSize();
	pRxBuf->Move(buflen);
	buflen=pTxBuf->GetReadableSize();
	pTxBuf->Move(buflen);

	readIni();

	m_lastCheckTime = GetNowSecond()-10;
	m_limitNum = 0;
	//InitStation(); 
	//resttime=0;
	//SendFrameNo=0;
}
/********************************************************************************
*
*	描述: 接收处理函数.
*	参数: 无.
*	返回: 无
********************************************************************************/

int CZfAgc::GetNowSecond()
{
	CJskTime     JSK_T;
	TCriterionTime tmptime;
	JSK_T.GetNow(&tmptime);
	return (int)tmptime;
}
sint32	CZfAgc::RxProc()
{

	return 1;

}

sint32 CZfAgc::TxProc()
{
	//const int TP = 400;
	int now = GetNowSecond();
	if(now - m_lastCheckTime < 10){
		return 1;
	}

	m_lastCheckTime = GetNowSecond();
	CEasyList *yclist=pZfDataCtrl->GetYcStructList();
	S_RDB_ZFYc *ycinfo;
	int l = yclist->GetListLen();
	if(m_curPowerYcPoint < l){
		ycinfo=(S_RDB_ZFYc *)((*yclist)[m_curPowerYcPoint]);
		m_curP = ycinfo->Val * m_curPowerCoef;
	}else{
		kprintf(LOG_DCS,DCV_LOG_DEFAULT,LOG_WARNING,"当前功率YC点 配置错误");
	}
	if(m_yTPowerYcPoint < l){
		ycinfo=(S_RDB_ZFYc *)((*yclist)[m_yTPowerYcPoint]);
		m_targetP = ycinfo->Val * m_yTPowerCoef;
	}else{
		kprintf(LOG_DCS,DCV_LOG_DEFAULT,LOG_WARNING,"调度功率YC点 配置错误");
	}

	if(m_curP - m_targetP > m_ctrlUpVal){
		closeDevs(m_curP - m_targetP);
	}else if(m_targetP - m_curP > m_ctrlUpVal){
		openDevs(m_targetP - m_curP);
	}


/*
	int totalPower = 0;
	for(i=0;i<l;i++)
	{
		ycinfo=(S_RDB_ZFYc *)((*yclist)[i]);
		totalPower += ycinfo->Val/100;
	}
	if(totalPower <= TP){
		return 1;
	}

	i = 0;
	int pOver = 0;
	while(i < l && pOver < totalPower - TP){
		ycinfo=(S_RDB_ZFYc *)((*yclist)[i]);
		pOver += (ycinfo->Val/100 - m_ctrlDownVal);
		i++;
	}
	m_limitNum = i;
	//int dValue = m_ctrlUpVal - m_ctrlDownVal;
	//if(totalPower > TP){
	//	m_limitNum += ((totalPower - TP)/dValue + 1);
	//	if(m_limitNum > l){
	//		m_limitNum = l;
	//	}
	//}

	//m_limitNum = 3;


	for(i = 0; i < m_limitNum; i++){
		ycinfo=(S_RDB_ZFYc *)((*yclist)[i]);

		S_CmdInfo cmdinfo;
		S_RAWCMD  rawcmd;
		sint32 rtuno=pLink->GetRtuNo();
		//rawcmd.rtuno=ycinfo->sr_rtuno;
		rawcmd.rtuno=rtuno;
		cmdinfo.ctrlpt=ycinfo->YcNo;
		//cmdinfo.ctrlpt=ycinfo->sr_ycno;
		//rawcmd.type=DC_K_LETTER_YKOPER;
		rawcmd.type=DC_K_LETTER_YKOPER;
		rawcmd.len=sizeof(cmdinfo);
		//cmdinfo.func1=DC_K_LETTER_YKCMD;
		cmdinfo.func1=DC_K_LETTER_YTEXE;
		cmdinfo.func2=0;
		if(i < m_limitNum){
			cmdinfo.deepctrlpt = m_ctrlDownVal;
		}else{
			cmdinfo.deepctrlpt = m_ctrlUpVal;
		}
		//cmdinfo.ret = 1;
		
		memcpy(rawcmd.Data,&cmdinfo,sizeof(cmdinfo));
		//rawcmd.cmdsrc_rtu = ycinfo->zfrtuno; 
		rawcmd.cmdsrc_rtu = ycinfo->sr_rtuno; 
		//rawcmd.cmdsrc_rtu = rtuno;
		rawcmd.cmdsrc_sort = E_PA_GZF;
		pCmdMem->ZfACmd(ycinfo->zfrtuno,rawcmd);
	}
*/

	return 1;
}

void CZfAgc::setYt(int YcNo, int sr_rtuno, int val){

	S_CmdInfo cmdinfo;
	S_RAWCMD  rawcmd;
	sint32 rtuno=pLink->GetRtuNo();
	rawcmd.rtuno=rtuno;
	cmdinfo.ctrlpt=YcNo;
	rawcmd.type=DC_K_LETTER_YKOPER;
	rawcmd.len=sizeof(cmdinfo);
	cmdinfo.func1=DC_K_LETTER_YTEXE;
	cmdinfo.func2=0;
	cmdinfo.deepctrlpt = val;
	
	memcpy(rawcmd.Data,&cmdinfo,sizeof(cmdinfo));
	rawcmd.cmdsrc_rtu = sr_rtuno; 
	rawcmd.cmdsrc_sort = E_PA_GZF;
	pCmdMem->ZfACmd(rtuno,rawcmd);
}

void CZfAgc::setYk(int YcNo, int sr_rtuno, int val){
	S_CmdInfo cmdinfo;
	S_RAWCMD  rawcmd;
	sint32 rtuno=pLink->GetRtuNo();
	rawcmd.rtuno=rtuno;
	cmdinfo.ctrlpt=YcNo;
	rawcmd.type=DC_K_LETTER_YKOPER;
	rawcmd.len=sizeof(cmdinfo);
	cmdinfo.func1=DC_K_LETTER_YKEXE;
	cmdinfo.func2=val;
	//cmdinfo.deepctrlpt = val;
	
	memcpy(rawcmd.Data,&cmdinfo,sizeof(cmdinfo));
	rawcmd.cmdsrc_rtu = sr_rtuno; 
	rawcmd.cmdsrc_sort = E_PA_GZF;
	pCmdMem->ZfACmd(rtuno,rawcmd);
}

void CZfAgc::closeDevs(int differ){
	S_RDB_ZFYc *ycinfo;
	S_RDB_ZFYx *yxinfo;
	int pOver = 0;
	CEasyList *yclist=pZfDataCtrl->GetYcStructList();
	CEasyList *yxlist=pZfDataCtrl->GetYxStructList();
	int l = m_pointVector.size();
	int i = 0;
	try{
	while(i < l && differ > 0){
		pointStruct pt = m_pointVector.at(i);
		
		if(m_ctrlType == 1){
			int index = pt.ycSetVal;
			ycinfo=(S_RDB_ZFYc *)((*yclist)[index]);
			if(ycinfo->Val * pt.ycCole != pt.ycCtrlUpVal){
				setYt(ycinfo->YcNo, ycinfo->sr_rtuno, pt.ycCtrlDownVal);
				differ -= (ycinfo->Val * pt.ycCole - pt.ycCtrlDownVal);
			}
		}else{
			int index = pt.st;
			yxinfo=(S_RDB_ZFYx *)((*yxlist)[index]);
			if(yxinfo->Val != 0){
				setYk(pt.st, yxinfo->sr_rtuno, 0);
				differ -= ycinfo->Val * pt.ycCole;
			}
		}
		i++;
	}
	
	}catch(...){
		kprintf(LOG_DCS,DCV_LOG_DEFAULT,LOG_WARNING,"关闭设备：第%d个点配置错误", i);
	}
}

void CZfAgc::openDevs(int differ){
	S_RDB_ZFYc *ycinfo;
	S_RDB_ZFYx *yxinfo;
	int pOver = 0;
	CEasyList *yclist=pZfDataCtrl->GetYcStructList();
	CEasyList *yxlist=pZfDataCtrl->GetYxStructList();
	int l = m_pointVector.size();
	int i = l-1;
	try{
	while(i >= 0 && differ > 0){
		pointStruct pt = m_pointVector.at(i);
		
		if(m_ctrlType == 1){
			int index = pt.ycSetVal;
			ycinfo=(S_RDB_ZFYc *)((*yclist)[index]);
			if(ycinfo->Val * pt.ycCole != pt.ycCtrlUpVal){
				setYt(ycinfo->YcNo, ycinfo->sr_rtuno, pt.ycCtrlUpVal);
				differ -= (pt.ycCtrlUpVal - ycinfo->Val * pt.ycCole);
			}
		}else{
			int index = pt.st;
			yxinfo=(S_RDB_ZFYx *)((*yxlist)[index]);
			if(yxinfo->Val != 1){
				setYk(pt.st, yxinfo->sr_rtuno, 1);
				differ -= pt.ycCtrlUpVal;
			}
		}
		i--;
	}
	}catch(...){
		kprintf(LOG_DCS,DCV_LOG_DEFAULT,LOG_WARNING,"打开设备：第%d个点配置错误", i);
	}
}

void CZfAgc::readIni(){
	char path[32] = "../cfg/agc.ini";
	char globalPara[8] = "global";
	char pointPara[8] = "point";
	CString pointHead;
	int pointNum = ::GetPrivateProfileInt(pointPara,"num",-1,path);
	for(int i = 0; i < pointNum; i++){
		pointHead.Format("p%.2d_", i);
		pointStruct ps;
		ps.ctrlPoint = ::GetPrivateProfileInt(pointPara,(pointHead + CString("ctrlPoint")).GetBuffer(0),-1,path);
		ps.st = ::GetPrivateProfileInt(pointPara,(pointHead + CString("st")).GetBuffer(0),-1,path);
		ps.ycCtrlDownVal = ::GetPrivateProfileInt(pointPara,(pointHead + CString("ycCtrlDownVal")).GetBuffer(0),-1,path);
		ps.ycCtrlUpVal = ::GetPrivateProfileInt(pointPara,(pointHead + CString("ycCtrlUpVal")).GetBuffer(0),-1,path);
		ps.ycSetVal = ::GetPrivateProfileInt(pointPara,(pointHead + CString("ycSetVal")).GetBuffer(0),-1,path);
		ps.ycVal = ::GetPrivateProfileInt(pointPara,(pointHead + CString("ycVal")).GetBuffer(0),-1,path);
		CString ycColeString;
		::GetPrivateProfileString(pointPara,(pointHead + CString("ycCole")).GetBuffer(0),"Error",ycColeString.GetBuffer(20),20,path);
		ps.ycCole = atof(ycColeString.GetBuffer(0));
		m_pointVector.push_back(ps);
	}
	m_startYxPoint = ::GetPrivateProfileInt(globalPara,"startYxPoint",-1,path);
	m_curPowerYcPoint = ::GetPrivateProfileInt(globalPara,"curPowerYcPoint",-1,path);
	CString curPowerCoefString;
	::GetPrivateProfileString(globalPara,"curPowerCoef","Error",curPowerCoefString.GetBuffer(20),20,path);
	m_curPowerCoef = atof(curPowerCoefString.GetBuffer(0));
	m_yTPowerYcPoint = ::GetPrivateProfileInt(globalPara,"yTPowerYcPoint",-1,path);
	CString yTPowerCoefString;
	::GetPrivateProfileString(globalPara,"yTPowerCoef","Error",yTPowerCoefString.GetBuffer(20),20,path);
	m_yTPowerCoef = atof(yTPowerCoefString.GetBuffer(0));
	m_ctrlUpVal = ::GetPrivateProfileInt(globalPara,"ctrlUpVal",-1,path);
	m_ctrlDownVal = ::GetPrivateProfileInt(globalPara,"ctrlDownVal",-1,path);
	m_intervalTime = ::GetPrivateProfileInt(globalPara,"intervalTime",-1,path);
	m_ctrlType = ::GetPrivateProfileInt(globalPara,"ctrlType",-1,path);
	
}


DLLEXPORT CProtocol	*CreateProtocol(char *defpara)
{
	return (CProtocol*)(new CZfAgc());
}

