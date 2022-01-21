
#include "NetworkHelpMethods.h"
#include "ThreadingMethods.h"
#include "NodeThreadParam.h"
#include <Models/Config.h>
#include <iostream>
#include <Serializator/Serialization.h>

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
            std::cout << "---------------------------------------" << std::endl;
            std::cout << "New db:" << newDb << std::endl;
            std::cout << "---------------------------------------" << std::endl;

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
                char* message = SerializeStudentDB(*(ntp->m_LocalDb));

                iResult = send(
                    acceptedSocket,
                    message,
                    (int)(ntp->m_LocalDb->GetBufferSize()),
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

                std::cout << "Client trying to add:" << s << std::endl;

                nodeLock->SetBusy(true);

                ntp->m_LocalDb->AddStudentPrepare(s);
                ntp->m_LocalDb->SavePermanentChanges();

                nodeLock->SetBusy(false);

                if (!TwoPhaseCommit(ntp->m_LocalDb, *(ntp->m_InternalPort))) {
                    ntp->m_LocalDb->RemoveStudent(s.Index);
                    std::cout << "2PC failed, removing student from temporary DB" << std::endl;
                }
                else {
                    std::cout << "2PC success, adding student to permament DB" << std::endl;
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

                std::cout << "Client trying to add:" << pce.ClassName  << " to " << pce.StudentIndex << " with " << pce.Grade << std::endl;

                nodeLock->SetBusy(true);

                ntp->m_LocalDb->AddPassedSubjectToStudentPrepare(
                    pce.StudentIndex,
                    {
                        pce.ClassName, pce.Grade
                    }
                );

                ntp->m_LocalDb->SavePermanentChanges();


                nodeLock->SetBusy(false);

                if (!TwoPhaseCommit(ntp->m_LocalDb, *(ntp->m_InternalPort))) {
                    ntp->m_LocalDb->RemoveStudent(pce.StudentIndex);
                    std::cout << "2PC failed, removing class from temporary DB" << std::endl;
                }
                else {
                    std::cout << "2PC success, adding class to permament DB" << std::endl;
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

    StudentDB* localDbP = ((StudentDB*)lpParam);

    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;

    // Prepare address information structures
    addrinfo* resultingAddress = NULL;
    addrinfo hints;

    std::vector<unsigned short> otherPorts = {};

    unsigned short syncPort = GetPort(&listenSocket, &resultingAddress, &hints, nodesFirstSyncPort, &otherPorts);

    if (otherPorts.size() > 0) {
        SyncWithNodesFirstTime(otherPorts, localDbP);
    }
    else {
        std::cout << "No other nodes on." << std::endl;
    }

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

        char* message = SerializeStudentDB(*localDbP);

        iResult = send(
            acceptedSocket,
            message,
            (int)localDbP->GetBufferSize(),
            0
        );

        if (iResult == SOCKET_ERROR)
        {
            continue;
        }

        delete[] message;
    }
}
