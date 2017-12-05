#pragma once

class CMyLock
{	
public:
	CMyLock();
	~CMyLock();
public:
	void init();
	void end();
	void lock();
	void unlock();
private:
	void* m_pData;	
};