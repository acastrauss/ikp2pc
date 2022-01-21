#pragma once

#include <Windows.h>
#include "Models/Config.h"

class CommitCondition {
private:
	
	unsigned short m_TotalNodes;
	unsigned short m_ReadyNodes;
	
	CRITICAL_SECTION cs;
	CONDITION_VARIABLE cv;
public:

	CommitCondition() = delete;
	CommitCondition(unsigned short totalNodes = nodesNumber);
	CommitCondition(
		const CommitCondition& ref
	) = delete;

	~CommitCondition();

	CommitCondition& operator=(
		const CommitCondition& ref
		) = delete;

	void IncreaseReadyNodes();
	unsigned short GetTotalNodes();
	unsigned short GetReadyNodes();
	bool GetReady();
};