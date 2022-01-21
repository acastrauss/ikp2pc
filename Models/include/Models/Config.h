#pragma once

constexpr unsigned int defaultBufferLength = 500;

constexpr unsigned short nodesNumber = 3;

// used for internal node communication
constexpr unsigned short nodesFirstInternalPort = 15678;

// used for communication with client
constexpr unsigned short nodesFirstExternalPort = 17891;

constexpr unsigned short nodesFirstSyncPort = 20000;

constexpr const char listAllCommand[] = "ListAll";