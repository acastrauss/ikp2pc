#pragma once

#include <Windows.h>

/// <summary>
/// Used to indicate if node is busy processing client
/// </summary>
class NodeLock {

private:
	bool m_Busy;
	CRITICAL_SECTION cs;

public:
	NodeLock();
	NodeLock(bool busy);
	NodeLock(const NodeLock& ref) = delete;
	~NodeLock();

	NodeLock& operator=(const NodeLock& ref) = delete;

	void SetBusy(bool busy);
	bool GetBusy();
};