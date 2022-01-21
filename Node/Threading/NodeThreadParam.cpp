#include "NodeThreadParam.h"


NodeThreadParam::NodeThreadParam(
	StudentDB* studentDb, NodeLock* nodeLock,
	unsigned short* externalPort, unsigned short* interrnalPort)
:
	m_LocalDb(studentDb), m_NodeLock(nodeLock),
	m_ExternalPort(externalPort), m_InternalPort(interrnalPort)
{}


NodeThreadParam::NodeThreadParam(const NodeThreadParam& ref)
:
	m_LocalDb(ref.m_LocalDb), m_NodeLock(ref.m_NodeLock),
	m_ExternalPort(ref.m_ExternalPort), m_InternalPort(ref.m_InternalPort)
{}


NodeThreadParam& NodeThreadParam::operator=(const NodeThreadParam& ref)
{
	m_LocalDb = ref.m_LocalDb;
	m_NodeLock = ref.m_NodeLock;
	m_ExternalPort = ref.m_ExternalPort;
	m_InternalPort = ref.m_InternalPort;
	return *this;
}