// DHClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#pragma comment(lib, "ws2_32.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD processId = GetCurrentProcessId();

	WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }

	struct addrinfo *result = NULL,
                *ptr = NULL,
                hints;

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	int iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		DebugBreak();
		return 1;
	}

	ptr=result;
	printf("family:%d type:%d proto:%d\n",ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	SOCKET ConnectSocket = INVALID_SOCKET;

	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		DebugBreak();
		return 1;
	}

	printf("ready, press any key to connect...\n");
	getc(stdin);

	iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("Unable to connect to server! %ld\n",  WSAGetLastError());
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		WSACleanup();
		DebugBreak();
		return 1;
	}

	int sendLen = send(ConnectSocket, (char*) &processId, sizeof(processId), 0);
	if (sendLen != sizeof(processId)) {
		printf("Error at send(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ConnectSocket);
		WSACleanup();
		DebugBreak();
		return 1;
	}

	HANDLE hFile = NULL;

	int recvLen = recv(ConnectSocket, (char*) &hFile, sizeof(hFile), MSG_WAITALL);
	if (recvLen != sizeof(hFile)) {
		printf("Error at recv(): %ld\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		DebugBreak();
		return 1;
	}

	closesocket(ConnectSocket);
	WSACleanup();

	printf("sent:%ld recv:%ld\n", processId, hFile);

	CHAR buffer[16384];
	DWORD readLen = 0;

	if (!ReadFile(hFile, buffer, sizeof(buffer), &readLen, NULL)) {
		printf("error at ReadFile: %ld\n", GetLastError());
		DebugBreak();
		return 1;
	}

	printf("read:%ld", readLen);

	fwrite(buffer, 1, readLen, stdout);

	CloseHandle(hFile);

	printf("\ndone, press any key...\n");
	getc(stdin);


	return 0;
}

