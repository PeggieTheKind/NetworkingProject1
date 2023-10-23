//Server Side Code: 
//Created by Alex V and Ryan M 
//For: Lukas Gustafson : INFO 6016
//
//Goals:
// Creating a Server for a Server:Client pair

#pragma once
// WinSock2 Windows Sockets
#define WIN32_LEAN_AND_MEAN
#define  _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include "hBufferS.h"
#include <map>//for Room list
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")// Need to link Ws2_32.lib
#define DEFAULT_PORT "8412"// First, make it work (messy), then organize

//struct lib
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

//List of clients
std::vector<SOCKET> gClientList;

//main stage
int main(int arg, char** argv)
{
	// Initialize WinSock
	WSADATA wsaData;
	int result;


	// Set version 2.2 with MAKEWORD(2,2) https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsastartup
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		printf("WSAStartup failed with error %d\n", result);
		return 1;
	}
	printf("WSAStartup successfully!\n");

	//addressing setup
	struct addrinfo* info = nullptr;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));	// ensure we don't have garbage data 
	hints.ai_family = AF_INET;			// IPv4
	hints.ai_socktype = SOCK_STREAM;	// Stream
	hints.ai_protocol = IPPROTO_TCP;	// TCP
	hints.ai_flags = AI_PASSIVE;

	// https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfo
	result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &info);
	if (result != 0) {
		printf("getaddrinfo failed with error %d\n", result);
		WSACleanup();
		return 1;
	}
	printf("getaddrinfo successfully!\n");

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

	//vector of current connections
	std::vector<SOCKET> activeConnections;
	FD_SET activeSockets;				// List of all of the clients ready to read.
	FD_SET socketsReadyForReading;		// List of all of the connections
	FD_ZERO(&activeSockets);			// Initialize the sets
	FD_ZERO(&socketsReadyForReading);

	// Use a timeval to prevent select from waiting forever.
	timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	//selecting loop
	while (true)
	{
		// Reset the socketsReadyForReading
		FD_ZERO(&socketsReadyForReading);

		// Add our listenSocket to our set to check for new connections
		// This will remain set if there is a "connect" call from a 
		// client to our server.
		FD_SET(listenSocket, &socketsReadyForReading);

		// Add all of our active connections to our socketsReadyForReading
		// set.
		for (int i = 0; i < activeConnections.size(); i++)
		{
			FD_SET(activeConnections[i], &socketsReadyForReading);
		}

		// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-select
		int count = select(0, &socketsReadyForReading, NULL, NULL, &tv);
		if (count == 0)
		{
			// Timevalue expired
			continue;
		}
		if (count == SOCKET_ERROR)
		{
			// Handle an error
			printf("select had an error %d\n", WSAGetLastError());
			continue;
		}

		// Loop through socketsReadyForReading
		//   recv
		for (int i = 0; i < activeConnections.size(); i++)
		{
			SOCKET socket = activeConnections[i];
			if (FD_ISSET(socket, &socketsReadyForReading))
			{
				// Handle receiving data with a recv call
				//char buffer[bufSize];
				const int bufSize = 512;
				Buffer buffer(bufSize);

				// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-recv
				// result 
				//		-1 : SOCKET_ERROR (More info received from WSAGetLastError() after)
				//		0 : client disconnected
				//		>0: The number of bytes received.
				int result = recv(socket, (char*)(&buffer.m_BufferData[0]), bufSize, 0);
				if (result == SOCKET_ERROR) {
					printf("recv failed with error %d\n", WSAGetLastError());
					closesocket(listenSocket);
					freeaddrinfo(info);
					WSACleanup();
					break;
					
				}
				if (result != 0)
					printf("Received %d bytes from the client!\n", result);
				//else
					//printf("Client #%s disconnected\n", activeConnections[i]);

				// We must receive 4 bytes before we know how long the packet actually is
				// We must receive the entire packet before we can handle the message.
				// Our protocol says we have a HEADER[pktsize, messagetype];
				uint32_t packetSize = buffer.ReadUInt32LE();
				uint32_t messageType = buffer.ReadUInt32LE();
				
				if (buffer.m_BufferData.size() >= packetSize)
				{
					
				}

				//default path 
				uint32_t messageLength = buffer.ReadUInt32LE();
				std::string msg = buffer.ReadString(messageLength);

				// We can finally handle our message
				//find params 
				const char* temp = msg.c_str();
				int counterSetter = 0;
				std::string userName = "";
				std::string roomPlan = "";
				std::string overallMsg = "";

				char* p = strtok((char*)temp, " ");
				while (p != NULL) 
				{
					if (counterSetter == 0)
						userName = p;

					if (counterSetter == 1)
						roomPlan = p;

					if (counterSetter > 1)
					{
						if(counterSetter == 2)
							overallMsg += p;
						else
							overallMsg += ' ' + p;
					}

					counterSetter++;
					std::cout << p << std::endl;
					p = strtok(NULL, " ");
				}

				for (int i = 0; i < activeConnections.size(); i++)
				{
					SOCKET socket = activeConnections[i];

					//pack it up

					//send too all in room
					ChatMessage message;
					//message.message = overallMsg;
					message.message = "helloworld from server";
					message.messageLength = message.message.length();
					message.header.messageType = 1;// Can use an enum to determine this
					message.header.packetSize =
						message.message.length()				// 5 'hello' has 5 bytes in it
						+ sizeof(message.messageLength)			// 4, uint32_t is 4 bytes
						+ sizeof(message.header.messageType)	// 4, uint32_t is 4 bytes
						+ sizeof(message.header.packetSize);	// 4, uint32_t is 4 bytes

					//protocol
					buffer.WriteUInt32LE(message.header.packetSize);	// LPMF
					buffer.WriteUInt32LE(message.header.messageType);	// 1
					buffer.WriteUInt32LE(message.messageLength);		// 5
					buffer.WriteString(message.message);				// Big edian Message should be the biggest value ASSCII

					//overallMsg
					result = send(socket, (char*)(&buffer.m_BufferData[0]), 512, 0);
							if (result == SOCKET_ERROR) {
								printf("send failed with error %d\n", WSAGetLastError());
								closesocket(listenSocket);
								freeaddrinfo(info);
								WSACleanup();
								break;
								}

				}

				FD_CLR(socket, &socketsReadyForReading);
				count--;
			}
		}

		// Handle any new connections, if count is not 0 then we have 
		// a socketReadyForReading that is not an active connection,
		// which means we have a 'connect' request from a client.
		//   accept
		if (count > 0)
		{
			if (FD_ISSET(listenSocket, &socketsReadyForReading))
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-accept
				SOCKET newConnection = accept(listenSocket, NULL, NULL);
				if (newConnection == INVALID_SOCKET)
				{
					// Handle errors
					printf("accept failed with error: %d\n", WSAGetLastError());
				}
				else
				{
					//// io = input, output, ctl = control
					//// input output control socket
					//// https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-ioctlsocket
					////-------------------------
					//// Set the socket I/O mode: In this case FIONBIO
					//// enables or disables the blocking mode for the 
					//// socket based on the numerical value of iMode.
					//// If iMode = 0, blocking is enabled; 
					//// If iMode != 0, non-blocking mode is enabled.
					// unsigned long NonBlock = 1;
					// ioctlsocket(newConnection, FIONBIO, &NonBlock);

					// Handle successful connection
					activeConnections.push_back(newConnection);
					FD_SET(newConnection, &activeSockets);
					FD_CLR(listenSocket, &socketsReadyForReading);

					printf("Client connected with Socket: %d\n", (int)newConnection);
				}
			}
		}
	}

	// Cleanup
	// https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-freeaddrinfo
	freeaddrinfo(info);

	// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-closesocket
	closesocket(listenSocket);

	// TODO Close connection for each client socket
	// closesocket(clientSocket);
	// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsacleanup
	WSACleanup();

	return 0;
}