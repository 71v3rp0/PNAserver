#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <iomanip>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Gumming.h"

#pragma comment(lib, "Ws2_32.lib")

int main()
{
	using namespace std;

	// Initializing Winsock

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cerr << "WSAStartup failed";
		return 1;
	}

	struct addrinfo* result = NULL;
	struct addrinfo hints =
	{
		AI_PASSIVE,		// flags
			AF_INET,		// family
			SOCK_STREAM,	// socktype
			IPPROTO_TCP		// protocol
	};

	if (getaddrinfo(NULL, "7000", &hints, &result))
	{
		cerr << "getaddrinfo failed";
		WSACleanup();
		return 1;
	}

	// Creating a Socket for the Server

	SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (listenSocket == INVALID_SOCKET)
	{
		cerr << "socket failed";
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Binding a Socket

	if (bind(listenSocket, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR)
	{
		cerr << "bind failed";
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	// Listening on a Socket

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		cerr << "listen failed";
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	//Accepting a Connection

	cout << "Waiting for connection...";

	sockaddr_in remoteHostInfo;
	socklen_t remoteHostInfoLen = sizeof(remoteHostInfo);

	SOCKET clientSocket = accept(listenSocket, (struct sockaddr*)&remoteHostInfo, &remoteHostInfoLen);

	if (clientSocket == INVALID_SOCKET)
	{
		cerr << "accept failed";
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		system("cls");

		cout << "\nServer: connected to [" << setfill(' ') << setw(15) << inet_ntoa(remoteHostInfo.sin_addr)
			<< "] successful...\n\n";
	}

	closesocket(listenSocket);

	//Receiving and Sending Data on the Server

	const unsigned bufferSize = 513;
	int gammaBuffer[bufferSize - 1];
	char dataBuffer[bufferSize];
	int res;

	do
	{
		// Receiving a message
		res = recv(clientSocket, dataBuffer, bufferSize, 0);

		if (dataBuffer[0] == '-' && dataBuffer[1] == 'g' && dataBuffer[2] == ' ')
		{
			// Get gamma
			recv(clientSocket, (char*)gammaBuffer, (bufferSize - 1) * sizeof(int), 0);

			// Decrypt message
			decryption(dataBuffer, gammaBuffer, res);
		}

		if (res > 0)
		{
			// Sending a message

			cout << "Client: " << dataBuffer << endl;

			const char* md = "Message delivered";

			if (send(clientSocket, md, (int)strlen(md) + 1, 0) == SOCKET_ERROR)
			{
				cerr << "send failed";
				closesocket(clientSocket);
				WSACleanup();
				return 1;
			}
		}
		else if (res == 0)
		{
			cout << "\nServer: connection closed...\n";
		}
		else
		{
			cerr << "recv failed";
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

	} while (res > 0);

	// Disconnecting the Server

	if (shutdown(clientSocket, SD_SEND))
	{
		cerr << "shutdown failed";
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	closesocket(clientSocket);
	WSACleanup();
	system("pause");

	return 0;
}
