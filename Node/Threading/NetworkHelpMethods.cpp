#include "NetworkHelpMethods.h"
#include <Models/Config.h>
#include <stdio.h>
#include <iostream>
#include <Serializator/Serialization.h>

bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}

std::deque<unsigned short> GetOtherPorts(unsigned short myPort, unsigned short firstPort)
{
    unsigned short firstPortToUse = firstPort;

    std::deque<unsigned short> ports = {};

    for (unsigned short i = firstPortToUse; i < firstPortToUse + nodesNumber; i++)
    {
        if (i != myPort) {
            ports.push_back(i);
        }
    }

    return ports;
}

void SyncWithNodesFirstTime(std::vector<unsigned short> ports, StudentDB* localDb)
{
    if (ports.empty()) return;

    unsigned short port = ports[
        rand() % ports.size()
    ];

    SOCKET connectSocket = INVALID_SOCKET;

    // create and initialize address structure
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(port);

    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        return;
    }

    // connect to server specified in serverAddress and socket connectSocket
    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cout << "Can't connect to node." << std::endl;
        return;
    }

    int iResult;
    // Buffer used for storing incoming data
    char recvbuf[defaultBufferLength];

    iResult = recv(
        connectSocket,
        recvbuf,
        defaultBufferLength,
        0
    );

    if (iResult == SOCKET_ERROR || iResult == 0)
    {
        std::cout << "Failed recv from node." << std::endl;
        return;
    }

    StudentDB nodeDb = DeserializeStudentDB(recvbuf);

    localDb->SetCurrentDb(nodeDb.GetStudents());

    std::cout << *localDb << std::endl;

    closesocket(connectSocket);
}


short GetPort(SOCKET* listenSocket,
    addrinfo** serverAddress,
    addrinfo* hints,
    unsigned short firstPort,
    std::vector<unsigned short>* ports)
{
    int iResult = 0;
    unsigned short portToUse = firstPort;

    while (true)
    {
        *listenSocket = INVALID_SOCKET;
        *serverAddress = NULL;

        memset(hints, 0, sizeof(*hints));
        hints->ai_family = AF_INET;       // IPv4 address
        hints->ai_socktype = SOCK_STREAM; // Provide reliable data streaming
        hints->ai_protocol = IPPROTO_TCP; // Use TCP protocol
        hints->ai_flags = AI_PASSIVE;     // 

        iResult = getaddrinfo(NULL, std::to_string(portToUse).c_str(), hints, serverAddress);
        if (iResult != 0)
        {
            printf("getaddrinfo failed with error: %d\n", iResult);
            //WSACleanup();
            return -1;
        }

        // Create a SOCKET for connecting to server
        *listenSocket = socket(AF_INET,      // IPv4 address famly
            SOCK_STREAM,  // stream socket
            IPPROTO_TCP); // TCP

        if (*listenSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            freeaddrinfo(*serverAddress);
            //WSACleanup();
            return -1;
        }

        // Setup the TCP listening socket - bind port number and local address 
        // to socket
        iResult = bind(*listenSocket, (*serverAddress)->ai_addr, (int)(*serverAddress)->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            printf("bind failed with error: %d\n", WSAGetLastError());
            freeaddrinfo(*serverAddress);

            if (++portToUse >= firstPort + nodesNumber)
            {
                std::cout << "No ports available for node." << std::endl;
                closesocket(*listenSocket);
                WSACleanup();
                return -1;
            }
            else {
                if (ports != NULL) {
                    ports->push_back(portToUse - 1);
                }

                continue;
            }
        }
        else {
            break;
        }
    }

    return portToUse;
}

bool TwoPhaseCommit(StudentDB* localDb, unsigned short myInternalPort)
{
    std::deque<unsigned short> nodesPorts = GetOtherPorts(myInternalPort, nodesFirstInternalPort);
    std::deque<unsigned short> portsCopy = nodesPorts;

    int iResult;

    std::vector<SOCKET> usedSockets = {};

    bool allReady = true;

    char recvbuf[defaultBufferLength];

    // check if ready for commit 
    while (!nodesPorts.empty()) {

        SOCKET connectSocket = socket(AF_INET,
            SOCK_STREAM,
            IPPROTO_TCP);

        unsigned short port = nodesPorts.front();
        nodesPorts.pop_front();

        // create and initialize address structure
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddress.sin_port = htons(port);

        if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
        {
            printf("Node is not on.\n");
            std::cout << WSAGetLastError() << std::endl;
            closesocket(connectSocket);
            continue;
        }

        char firstMsg[] = "Try Commit";

        iResult = send(
            connectSocket,
            firstMsg,
            (int)strlen(firstMsg),
            0
        );

        if (iResult == SOCKET_ERROR)
        {
            continue;
        }

        iResult = recv(
            connectSocket,
            recvbuf,
            defaultBufferLength,
            0
        );

        if (iResult == SOCKET_ERROR || iResult == 0)
        {
            continue;
        }

        int resp = std::stoi(recvbuf);

        if (resp == 0) {
            allReady = false;
            break;
        }

        usedSockets.push_back(connectSocket);
    }

    if (allReady) {
        for (int i = 0; i < usedSockets.size(); i++)
        {
            char* datamsg = SerializeStudentDB(*localDb);

            iResult = send(
                usedSockets[i],
                datamsg,
                (int)localDb->GetBufferSize(),
                0
            );

            std::cout << *localDb << std::endl;

            delete[] datamsg;

            if (iResult == SOCKET_ERROR)
            {
                continue;
            }
        }
    }

    return allReady;
}
