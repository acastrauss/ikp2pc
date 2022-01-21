#pragma once

#include <Models/StudentDB.h>
#include <Models/NodeLock.h>

struct NodeThreadParam {
    StudentDB* m_LocalDb;
    NodeLock* m_NodeLock;
    unsigned short* m_ExternalPort;
    unsigned short* m_InternalPort;

    NodeThreadParam() = delete;

    explicit NodeThreadParam(
        StudentDB* studentDb, NodeLock* nodeLock,
        unsigned short* externalPort, unsigned short* interrnalPort
    );

    NodeThreadParam(const NodeThreadParam& ref);

    NodeThreadParam& operator=(const NodeThreadParam& ref);
};
