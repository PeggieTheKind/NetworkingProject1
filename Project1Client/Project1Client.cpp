//Client Side Code: 
//Created by Alex V and Ryan M 
//For: Lukas Gustafson : INFO 6016
//
//Goals:
// Creating a Client for a Server:Client pair

// WinSock2 Windows Sockets
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "hBufferC.h" // Ensure this matches the server end

struct PacketHeader
{
    uint32_t packetSize;
    uint32_t messageType;
};

struct ChatMessage
{
    PacketHeader header;
    uint32_t messageLength;
    std::string message;
};

// Need to link Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

// First, make it work (messy), then organize
#define DEFAULT_PORT "8412"

int main(int arg, char** argv)
{
    // Initialize WinSock
    WSADATA wsaData;
    int result;

    // Set version 2.2 with MAKEWORD(2,2)
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed with error %d\n", result);
        return 1;
    }
    printf("WSAStartup successfully!\n");

    struct addrinfo* info = nullptr;
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));    // ensure we don't have garbage data 
    hints.ai_family = AF_INET;            // IPv4
    hints.ai_socktype = SOCK_STREAM;    // Stream
    hints.ai_protocol = IPPROTO_TCP;    // TCP
    hints.ai_flags = AI_PASSIVE;

    result = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &info);
    if (result != 0) {
        printf("getaddrinfo failed with error %d\n", result);
        WSACleanup();
        return 1;
    }
    printf("getaddrinfo successfully!\n");

    // Socket
    SOCKET serverSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (serverSocket == INVALID_SOCKET) {
        printf("socket failed with error %d\n", WSAGetLastError());
        freeaddrinfo(info);
        WSACleanup();
        return 1;
    }
    printf("socket created successfully!\n");

    // Connect
    result = connect(serverSocket, info->ai_addr, (int)info->ai_addrlen);
    if (serverSocket == INVALID_SOCKET) {
        printf("connect failed with error %d\n", WSAGetLastError());
        closesocket(serverSocket);
        freeaddrinfo(info);
        WSACleanup();
        return 1;
    }
    printf("Connect to the server successfully!\n");

   
    // Socket
    // https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-socket
    SOCKET listenSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        printf("socket failed with error %d\n", WSAGetLastError());
        freeaddrinfo(info);
        WSACleanup();
        return 1;
    }
    printf("socket created successfully!\n");

    // Bind
    // https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-bind
    result = bind(listenSocket, info->ai_addr, (int)info->ai_addrlen);
    if (result == SOCKET_ERROR) {
        printf("bind failed with error %d\n", WSAGetLastError());
        closesocket(listenSocket);
        freeaddrinfo(info);
        WSACleanup();
        return 1;
    }
    printf("bind was successful!\n");

    // Listen
    // https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-listen
    result = listen(listenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        printf("listen failed with error %d\n", WSAGetLastError());
        closesocket(listenSocket);
        freeaddrinfo(info);
        WSACleanup();
        return 1;
    }
    printf("listen successful\n");

    std::string userName;
    std::string currentChatRoom;
    std::string userInput;
    std::string roomName;


    std::cout << "Enter user name: ";
    getline(std::cin, userName);

    //gathers msgs
    std::cout << "Enter a message to send to the server (or type 'exit' to quit): ";
    std::getline(std::cin, userInput);

    std::cout << "Enter room wanted (or type 'exit' to quit): ";
    std::getline(std::cin, roomName);

    while (true)
    {
        
        
            //RYANS crafting 
            // Create a ChatMessage
            ChatMessage message;
            // message.message = "[" + username + "] " + userInput;
            message.message = userName + " " + roomName + " " + userInput;
            message.messageLength = static_cast<uint32_t>(message.message.length());
            message.header.messageType = 1; // Assuming 1 represents a regular chat message

            // Calculate the packet size
            message.header.packetSize =
                message.message.length() +
                sizeof(message.messageLength) +
                sizeof(message.header.messageType) +
                sizeof(message.header.packetSize);

            // Write the message to the buffer
            const int bufSize = 512;
            Buffer buffer(bufSize);
            buffer.WriteUInt32LE(message.header.packetSize);
            buffer.WriteUInt32LE(message.header.messageType);
            buffer.WriteUInt32LE(message.messageLength);
            buffer.WriteString(message.message);

            result = send(serverSocket, (char*)(&buffer.m_BufferData[0]), 512, 0);
            if (result == SOCKET_ERROR) {
                printf("send failed with error %d\n", WSAGetLastError());
                closesocket(serverSocket);
                freeaddrinfo(info);
                WSACleanup();
                break;
            }

            int resultTemp = recv(serverSocket, (char*)(&buffer.m_BufferData[0]), bufSize, 0);
            if (resultTemp == SOCKET_ERROR) {
                printf("recv failed with error %d\n", WSAGetLastError());
                closesocket(serverSocket);
                freeaddrinfo(info);
                WSACleanup();
                break;

            }
            if (resultTemp != 0)
                printf("Received %d bytes from the server!\n", result);

            //default path 
            uint32_t messageLength = buffer.ReadUInt32LE();
            std::string msg = buffer.ReadString(messageLength);

            printf("\nMessageLength:%d\nMessage:%s\n",
                	 messageLength, msg.c_str());

            system("Pause"); // Force the user to press enter to continue;
            std::cout << "To Send another message hit enter and type (or type 'exit' to quit): ";

            //gathers msgs
            std::getline(std::cin, userInput);
        if (userInput == "exit") //we should send a disconnect msg
        {
            break; // Exit the loop and close the program
        }
    }

    // Close
    freeaddrinfo(info);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
