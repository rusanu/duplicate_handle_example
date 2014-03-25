// DHServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#pragma comment(lib, "ws2_32.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hFile = CreateFile(L"c:\\temp\\yarn-1865.1.patch", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		printf( "CreateFile failed with error: %ld\n", GetLastError() );
		DebugBreak();
		return 1;
	}

	HANDLE hSelf = OpenProcess(PROCESS_DUP_HANDLE, FALSE, GetCurrentProcessId());
	if (hSelf == INVALID_HANDLE_VALUE) {
		printf( "OpenProcess failed with error: %ld\n", GetLastError() );
		DebugBreak();
		return 1;
	}

	WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("WSAStartup failed with error: %d\n", err);
		DebugBreak();
        return 1;
    }

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof (hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		DebugBreak();
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		DebugBreak();
		return 1;
	}

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
		DebugBreak();
        return 1;
    }

	freeaddrinfo(result);

	if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
		printf( "Listen failed with error: %ld\n", WSAGetLastError() );
		closesocket(ListenSocket);
		WSACleanup();
		DebugBreak();
		return 1;
	}

	SOCKET ClientSocket = INVALID_SOCKET;

	struct sockaddr  faraddr;
	int addr_len = sizeof(faraddr);

	printf("Listenning...\n");

	while (1) {

		// Accept a client socket
		ClientSocket = accept(ListenSocket, &faraddr, &addr_len);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			DebugBreak();
			return 1;
		}

		DWORD processId = 0;
		int recvLen = recv(ClientSocket, (char*) &processId, sizeof(processId), MSG_WAITALL);
		if (recvLen != sizeof(processId)) {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			DebugBreak();
			return 1;
		}

		HANDLE targetProcHandle = OpenProcess(PROCESS_DUP_HANDLE, FALSE, processId);
		if (targetProcHandle == INVALID_HANDLE_VALUE) {
			printf("OpenProcess failed: %d\n", GetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			DebugBreak();
			return 1;
		}

		HANDLE hDup = NULL;

		if (!DuplicateHandle(hSelf, hFile, targetProcHandle, &hDup, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
			printf("DuplicateHandle failed: %d\n", GetLastError());
			closesocket(ListenSocket);
			closesocket(ClientSocket);
			WSACleanup();
			DebugBreak();
			return 1;
		}

		int sendLen = send(ClientSocket, (char*) &hDup, sizeof(hDup), 0);
		if (sendLen != sizeof(hDup)) {
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			closesocket(ClientSocket);
			WSACleanup();
			DebugBreak();
			return 1;
		}

		closesocket(ClientSocket);
		printf("recv: %ld sent: %ld\n", targetProcHandle, hDup);
	}

	return 0;
}

