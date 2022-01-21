#pragma once
#include <ws2tcpip.h>
#include <deque>
#include <vector>
#include <Models/StudentDB.h>

bool InitializeWindowsSockets();

std::deque<unsigned short> GetOtherPorts(unsigned short myPort, unsigned short firstPort);

void SyncWithNodesFirstTime(std::vector<unsigned short> ports, StudentDB* localDb);

short GetPort(
    SOCKET* listenSocket,
    addrinfo** serverAddress,
    addrinfo* hints,
    unsigned short firstPort,
    std::vector<unsigned short>* ports = NULL
);

bool TwoPhaseCommit(StudentDB* localDb, unsigned short myInternalPort);
