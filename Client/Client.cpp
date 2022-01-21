// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>
#include <stdlib.h>
#include <time.h>  

#include "Models/Config.h"
#include "Serializator/Serialization.h"

#pragma comment(lib, "ws2_32.lib")

struct BufferInput {
    char* Buffer;
    size_t BufferSize;
};

bool InitializeWindowsSockets();
unsigned short GetRandomNodePort();
unsigned int GetUserInput();
BufferInput GetBufferForInput(unsigned int input);


int main()
{
    // socket used to communicate with server
    SOCKET connectSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;

    if (InitializeWindowsSockets() == false)
    {
        // we won't log anything since it will be logged
        // by InitializeWindowsSockets() function
        return 1;
    }

    // create a socket
    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // create and initialize address structure
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(GetRandomNodePort());

    unsigned int userInput = GetUserInput();
    std::vector<unsigned int> possibleInputs = { 1, 2, 3 };
    
    auto it = std::find(
        possibleInputs.begin(),
        possibleInputs.end(),
        userInput
    );

    if (it != possibleInputs.end()) {

        std::cout << "Found input" << std::endl;

        // connect to server specified in serverAddress and socket connectSocket
        if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
        {
            printf("Unable to connect to server.\n");
            closesocket(connectSocket);
            WSACleanup();
        }

        std::string userInputStr = std::to_string(userInput);

        iResult = send(connectSocket, userInputStr.c_str(), (int)userInputStr.size() + 1, 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            return 1;
        }

        if (userInput != 1) {
            BufferInput messageToSend = GetBufferForInput(userInput);

            iResult = send(connectSocket, messageToSend.Buffer, (int)messageToSend.BufferSize, 0);

            if (iResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(connectSocket);
                WSACleanup();
                return 1;
            }

            delete[] messageToSend.Buffer;

            printf("Bytes Sent: %ld\n", iResult);
        }
        else {
            char recvBuffer[defaultBufferLength];

            iResult = recv(connectSocket, recvBuffer, defaultBufferLength, 0);
            if (iResult == SOCKET_ERROR)
            {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(connectSocket);
                WSACleanup();
                return 1;
            }

            StudentDB currentDb = DeserializeStudentDB(recvBuffer);

            std::cout << currentDb << std::endl;
        }
    }
    
    // cleanup
    closesocket(connectSocket);
    WSACleanup();

    std::getchar();

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

bool firstCall = false;

unsigned short GetRandomNodePort()
{
    srand(time(NULL));
    auto rrr = (unsigned short)(
        nodesFirstExternalPort + rand() % nodesNumber);

    std::cout << "Connecting to port:" << rrr << std::endl;

    return rrr;
}

unsigned int GetUserInput()
{
    std::cout << "Please select an option:" << std::endl;
    std::cout << "1. List all students." << std::endl;
    std::cout << "2. Add a student." << std::endl;
    std::cout << "3. Add passed class to student." << std::endl;
    
    unsigned int opt = 0;
    std::cin >> opt;

    return opt;
}

BufferInput GetBufferForInput(unsigned int input)
{
    switch (input)
    {
    case 1:
        return {
            (char*)listAllCommand,
            strlen(listAllCommand)
        };
    case 2: {
        Student student;
        std::cout << "Enter student index number:" << std::endl;
        std::cin >> student.Index;
        std::cout << "Enter student first name:" << std::endl;
        std::cin >> student.FirstName;
        std::cout << "Enter student last name:" << std::endl;
        std::cin >> student.LastName;
        return {
            SerializeStudent(student),
            student.GetBufferSize()
        };
    }
    case 3: {
        PassedClassEntry pce;
        std::cout << "Enter student index number:" << std::endl;
        std::cin >> pce.StudentIndex;

        std::cout << "Enter class name:" << std::endl;
        std::cin >> pce.ClassName;

        std::cout << "Enter grade:" << std::endl;
        std::cin >> pce.Grade;
        return {
            SerializePassedClass(pce),
            pce.GetBufferSize()
        };
    }
    default: {
        std::cout << "Incorrect input." << std::endl;
        return {
            nullptr, 0
        };
    }
    }
}
