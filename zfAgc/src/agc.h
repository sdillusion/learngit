#ifndef	_AGCPROC_
#define	_AGCPROC_

#include "comm/protocolbase.h"
#include <vector>

using namespace std;

#define MOXA_LINUX

#ifdef MOXA_LINUX
#define LITTLEENDIAN 1
#endif //#ifdef MOXA_LINUX

struct pointStruct{
	int ycVal;
	float ycCole;
	int ycSetVal;
	int st;
	int ctrlPoint;
	int ycCtrlUpVal;
	int ycCtrlDownVal;
};

class CZfAgc : public CProtocol
{
	protected:
		int	GetNowSecond();
		
	public:
		virtual sint32 TxProc();	// 一致接口
		virtual sint32 RxProc();
		virtual sint32 GetZfFlag( )
		{
			return 1;
		};
		virtual void Init( S_PROTOCOLCFG * pcfg );
   private:

	   void readIni();
	   void openDevs(int differ);
	   void closeDevs(int differ);
	   void setYt(int YcNo, int sr_rtuno, int val);
	   void setYk(int YcNo, int sr_rtuno, int val);

   private:
		int m_startYxPoint;
		int m_curPowerYcPoint;
		float m_curPowerCoef;
		int m_yTPowerYcPoint;
		float m_yTPowerCoef;
		int m_ctrlUpVal;
		int m_ctrlDownVal;
		int m_intervalTime;
		int m_ctrlType;

		vector<pointStruct> m_pointVector;

		int m_lastCheckTime;//上次检查时间
		int m_limitNum;//限制功率的个数

		int m_curP;//当前总功率
		int m_targetP;//调度总功率
};








#endif //#ifndef	_AGCPROC_