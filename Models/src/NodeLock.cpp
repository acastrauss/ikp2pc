#include "pch.h"
#include "Models/NodeLock.h"

NodeLock::NodeLock():m_Busy(false)
{
	InitializeCriticalSection(&cs);
}

NodeLock::NodeLock(bool busy)
{
	m_Busy = busy;
	InitializeCriticalSection(&cs);
}

NodeLock::~NodeLock()
{
	DeleteCriticalSection(&cs);
}

void NodeLock::SetBusy(bool busy)
{
	EnterCriticalSection(&cs);
	m_Busy = busy;
	LeaveCriticalSection(&cs);
}

bool NodeLock::GetBusy()
{
	EnterCriticalSection(&cs);
	bool retVal = m_Busy;
	LeaveCriticalSection(&cs);
	return retVal;
}