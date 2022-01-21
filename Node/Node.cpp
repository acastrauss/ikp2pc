// Node.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Threading/NetworkHelpMethods.h"
#include "Threading/ThreadingMethods.h"

#include <iostream>
#include <Models/Config.h>
#include <Models/StudentDB.h>
#include <Serializator/Serialization.h>
#include <deque>
#include <string>
#include <Models/NodeLock.h>
#include <Models/CommitCondition.h>
#include "Threading/NodeThreadParam.h"


#pragma comment(lib, "ws2_32.lib")



int main()
{
    unsigned short externalPort = 0;
    unsigned short internalPort = 0;

    StudentDB students;
    NodeLock nodeLock(false);

    std::map<std::string, USHORT> classesPassed = {};
    classesPassed.insert({
        std::string("IKP"), 10
        });

    students.AddStudentPrepare(
        Student(
            std::string("PR123-2022"),
            std::string("Pera"),
            std::string("Peric"),
            classesPassed
        )
    );

    students.SavePermanentChanges();

    InitializeWindowsSockets();


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
        NULL, 0, WaitForSync, &students, 0, 0
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

