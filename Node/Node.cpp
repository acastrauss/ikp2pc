// Node.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <ws2tcpip.h>

#include <iostream>
#include <Models/Config.h>
#include <Models/StudentDB.h>
#include <Serializator/Serialization.h>
#include <string>
#include <Models/NodeLock.h>
#include <Models/CommitCondition.h>
#include <deque>

#pragma comment(lib, "ws2_32.lib")

bool InitializeWindowsSockets();

short GetPort(
    SOCKET* listenSocket,
    addrinfo** serverAddress,
    addrinfo* hints,
    unsigned short firstPort,
    std::vector<unsigned short>* ports = NULL
);

std::deque<unsigned short> GetOtherPorts(unsigned short myPort, unsigned short firstPort);

void SyncWithNodesFirstTime(std::deque<unsigned short> ports, StudentDB* localDb);
bool TwoPhaseCommit(StudentDB* localDb, unsigned short myInternalPort);
/// <summary>
/// List for client connections and process them
/// </summary>
/// <param name="lpParam"></param>
/// <returns></returns>
DWORD WINAPI ClientProcessing(LPVOID lpParam);
DWORD WINAPI WaitForSync(LPVOID lpParam);

/// <summary>
/// List for nodes connections and process them
/// </summary>
/// <param name="lpParam"></param>
/// <returns></returns>
DWORD WINAPI NodeProcessing(LPVOID lpParam);


struct NodeThreadParam {
    StudentDB* m_LocalDb;
    NodeLock* m_NodeLock;
    unsigned short* m_ExternalPort;
    unsigned short* m_InternalPort;

    NodeThreadParam() = delete;
    explicit NodeThreadParam(
        StudentDB* studentDb, NodeLock* nodeLock,
        unsigned short* externalPort, unsigned short* interrnalPort 
    ):
        m_LocalDb(studentDb), m_NodeLock(nodeLock),
        m_ExternalPort(externalPort), m_InternalPort(interrnalPort)
    {}
    
    NodeThreadParam(const NodeThreadParam& ref):
        m_LocalDb(ref.m_LocalDb), m_NodeLock(ref.m_NodeLock),
        m_ExternalPort(ref.m_ExternalPort), m_InternalPort(ref.m_InternalPort)
    {}

    NodeThreadParam& operator=(const NodeThreadParam& ref){
        m_LocalDb = ref.m_LocalDb;
        m_NodeLock = ref.m_NodeLock;
        m_ExternalPort = ref.m_ExternalPort;
        m_InternalPort = ref.m_InternalPort;
        return *this;
    }
};


StudentDB students;

int main()
{
    unsigned short externalPort = 0;
    unsigned short internalPort = 0;

    std::map<std::string, USHORT> classesPassed = {};
    classesPassed.insert({
        std::string("ewqewqeewq"), 7
        });

    students.AddStudentPrepare(
        Student(
            std::string("asasds"),
            std::string("asasds"),
            std::string("asasds"),
            classesPassed
        )
    );

    students.SavePermanentChanges();

    InitializeWindowsSockets();

    NodeLock nodeLock(false);

    NodeThreadParam ntp(
        &students, &nodeLock, &externalPort, &internalPort
    );

    HANDLE hClientThread = CreateThread(
        NULL, 0, ClientProcessing, &ntp, 0, 0
    );

    HANDLE hNodesThread = CreateThread(
        NULL, 0, NodeProcessing, &ntp, 0, 0
    );

    HANDLE hSync = CreateThread(
        NULL, 0, WaitForSync, NULL, 0, 0
    );
    
    if (hClientThread != NULL) {
        WaitForSingleObject(hClientThread, INFINITE);
        if (hClientThread) {
            CloseHandle(hClientThread);
        }
    }

    if (hNodesThread != NULL) {
        WaitForSingleObject(hNodesThread, INFINITE);
        if (hNodesThread) {
            CloseHandle(hNodesThread);
        }
    }
    
    if (hSync != NULL) {
        WaitForSingleObject(hSync, INFINITE);
        if (hSync) {
            CloseHandle(hSync);
        }
    }

	// check if there are nodes up (sync if yes)
	// wait for client connection
	// receive client message
	// send commit message
	// if all accept send current db

    WSACleanup();

	return 0;
}

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

void SyncWithNodesFirstTime(std::deque<unsigned short> ports, StudentDB* localDb)
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


short GetPort(
    SOCKET* listenSocket,
    addrinfo** serverAddress,
    addrinfo* hints,
    unsigned short firstPort,
    std::vector<unsigned short>* ports
)
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
            
            if (++portToUse >= 
                    firstPort + nodesNumber
                )
            {
                std::cout << "No ports available for node." << std::endl;
                closesocket(*listenSocket);
                WSACleanup();
                return -1;
            }
            else {
                if (firstPort != nodesFirstExternalPort && ports != NULL) {
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
            printf("Unable to connect to server.\n");
            std::cout << WSAGetLastError() << std::endl;
            closesocket(connectSocket);
            continue;
        }

        char firstMsg[] = "Try Commit";

        iResult = send(
            connectSocket,
            firstMsg,
            strlen(firstMsg),
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
                localDb->GetBufferSize(),
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

DWORD __stdcall NodeProcessing(LPVOID lpParam)
{
    NodeThreadParam* ntp = ((NodeThreadParam*)lpParam);

    StudentDB* localDb = ntp->m_LocalDb;
    NodeLock* nodeLock = ntp->m_NodeLock;

    std::vector<unsigned short> usedPorts = {};

    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // Buffer used for storing incoming data
    char recvbuf[defaultBufferLength];

    // Prepare address information structures
    addrinfo* resultingAddress = NULL;
    addrinfo hints;

    short myPort = GetPort(&listenSocket, &resultingAddress, &hints, nodesFirstInternalPort, &usedPorts);
    
    if (myPort == -1) {
        std::cout << "Failed acquiring port. Quiting." << std::endl;
        return 1;
    }
    
    freeaddrinfo(resultingAddress);

    *(ntp->m_InternalPort) = myPort;

    std::cout << "Using internal port:" << myPort << std::endl;

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        return 1;
    }

    while (true)
    {
        std::cout << "Waiting for 2pc" << std::endl;
        // wait for other nodes to connect   
        SOCKET acceptedSocket = accept(listenSocket, NULL, NULL);

        iResult = recv(
            acceptedSocket,
            recvbuf,
            defaultBufferLength,
            0
        );

        if (iResult == SOCKET_ERROR || iResult == 0)
        {
            continue;
        }

        int resp = !nodeLock->GetBusy();

        iResult = send(
            acceptedSocket,
            std::to_string(resp).c_str(),
            sizeof(resp),
            0
        );

        if (iResult == SOCKET_ERROR)
        {
            continue;
        }

        std::cout << "Send response:" << resp << std::endl;

        if (resp) {
            iResult = recv(
                acceptedSocket,
                recvbuf,
                defaultBufferLength,
                0
            );

            StudentDB newDb = DeserializeStudentDB(recvbuf);

            std::cout << "New db:" << newDb << std::endl;
            std::cout << "Using internal port:" << myPort << std::endl;

            localDb->SetCurrentDb(newDb.GetStudents());

            if (iResult == SOCKET_ERROR || iResult == 0)
            {
                continue;
            }
        }
        
        Sleep(500);
    }
}

DWORD __stdcall ClientProcessing(LPVOID lpParam)
{
    NodeThreadParam* ntp = ((NodeThreadParam*)lpParam);

    NodeLock* nodeLock = ntp->m_NodeLock;

    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // Buffer used for storing incoming data
    char recvbuf[defaultBufferLength];

    // Prepare address information structures
    addrinfo* resultingAddress = NULL;
    addrinfo hints;

    unsigned short portToUse = GetPort(&listenSocket, &resultingAddress, &hints, nodesFirstExternalPort);
    
    if (portToUse != -1) {
        std::cout << "Using external port:" << portToUse << std::endl;
        *(ntp->m_ExternalPort) = portToUse;

        // Set listenSocket in listening mode
        iResult = listen(listenSocket, SOMAXCONN);
        if (iResult == SOCKET_ERROR)
        {
            printf("listen failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        do
        {
            SOCKET acceptedSocket = accept(listenSocket, NULL, NULL);

            iResult = recv(
                acceptedSocket,
                recvbuf,
                defaultBufferLength,
                0
            );

            if (iResult == SOCKET_ERROR || iResult == 0)
            {
                continue;
            }

            int userChoice = std::atoi(recvbuf);

            switch (userChoice)
            {
            case 1: {
                // list all students
                char* message = SerializeStudentDB(students);

                iResult = send(
                    acceptedSocket,
                    message,
                    (int)students.GetBufferSize(),
                    0
                );

                if (iResult == SOCKET_ERROR)
                {
                    continue;
                }

                delete[] message;
                break;
            }

            case 2: {
                // add student
            
                iResult = recv(
                    acceptedSocket,
                    recvbuf,
                    defaultBufferLength,
                    0
                );

                if (iResult == SOCKET_ERROR || iResult == 0)
                {
                    std::cout << "Nothing received" << std::endl;
                    continue;
                }

                Student s = DeserializeStudent(recvbuf);


                nodeLock->SetBusy(true);

                students.AddStudentPrepare(s);
                students.SavePermanentChanges();

                nodeLock->SetBusy(false);

                if (!TwoPhaseCommit(ntp->m_LocalDb, *(ntp->m_InternalPort))) {
                    students.RemoveStudent(s.Index);
                }

                break;
            }

            case 3: {
                // add passed subject
                iResult = recv(
                    acceptedSocket,
                    recvbuf,
                    defaultBufferLength,
                    0
                );

                if (iResult == SOCKET_ERROR || iResult == 0)
                {
                    continue;
                }

                PassedClassEntry pce = DeserializePassedClass(recvbuf);
                
                nodeLock->SetBusy(true);

                students.AddPassedSubjectToStudentPrepare(
                    pce.StudentIndex,
                    {
                        pce.ClassName, pce.Grade
                    }
                );
                students.SavePermanentChanges();


                nodeLock->SetBusy(false);

                if (!TwoPhaseCommit(ntp->m_LocalDb, *(ntp->m_InternalPort))) {
                    students.RemoveStudent(pce.StudentIndex);
                }

                break;
            }
            
            default:
                continue;
            }

        } while (true);
    }

    return 0;
}

DWORD __stdcall WaitForSync(LPVOID lpParam) {

    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // Buffer used for storing incoming data
    char recvbuf[defaultBufferLength];

    // Prepare address information structures
    addrinfo* resultingAddress = NULL;
    addrinfo hints;

    unsigned short syncPort = GetPort(&listenSocket, &resultingAddress, &hints, nodesFirstSyncPort);

    auto otherPorts = GetOtherPorts(syncPort, nodesFirstSyncPort);

    SyncWithNodesFirstTime(otherPorts, &students);

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    while (true)
    {
        SOCKET acceptedSocket = accept(listenSocket, NULL, NULL);

        char* message = SerializeStudentDB(students);

        iResult = send(
            acceptedSocket,
            message,
            (int)students.GetBufferSize(),
            0
        );

        if (iResult == SOCKET_ERROR)
        {
            continue;
        }

        delete[] message;
    }

}