#include "pch.h"
#include "Models/CommitCondition.h"
#include <iostream>

CommitCondition::CommitCondition(unsigned short totalNodes)
	: m_TotalNodes(totalNodes), m_ReadyNodes(0)
{
	InitializeCriticalSection(&cs);
	InitializeConditionVariable(&cv);
}

CommitCondition::~CommitCondition()
{
	DeleteCriticalSection(&cs);
}

void CommitCondition::IncreaseReadyNodes()
{
	EnterCriticalSection(&cs);

	if (m_ReadyNodes < m_TotalNodes) {
		m_ReadyNodes++;
	}
	else {
		std::cout << "More ready than total nodes." << std::endl;
	}

	LeaveCriticalSection(&cs);
}

unsigned short CommitCondition::GetTotalNodes()
{
	EnterCriticalSection(&cs);

	unsigned short retVal = m_TotalNodes;

	LeaveCriticalSection(&cs);

	return retVal;
}

unsigned short CommitCondition::GetReadyNodes()
{
	EnterCriticalSection(&cs);

	unsigned short retVal = m_ReadyNodes;

	LeaveCriticalSection(&cs);

	return retVal;
}

bool CommitCondition::GetReady()
{
	EnterCriticalSection(&cs);

	bool retVal = m_ReadyNodes == m_TotalNodes;

	LeaveCriticalSection(&cs);

	return retVal;
}