#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016
#define MAX_SIZE 100
// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();
void Select(SOCKET socket, bool read);

int __cdecl main(int argc, char **argv)
{
	// socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;
	// variable used to store function return value
	int iResult;
	// message to send
	char messageToSend[MAX_SIZE];
	char topics[MAX_SIZE];
	char buffer[DEFAULT_BUFLEN];
	memset(buffer, 0, DEFAULT_BUFLEN);
	int choose = 0;
	// Validate the parameters

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
	serverAddress.sin_port = htons(DEFAULT_PORT);
	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}

	unsigned long mode = 1;
	if (ioctlsocket(connectSocket, FIONBIO, &mode) == SOCKET_ERROR) {
		printf("ioctl failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	memset(messageToSend, 0, MAX_SIZE);
	memset(topics, 0, MAX_SIZE);
	memcpy(messageToSend, "Subscriber*", strlen("Subscriber*"));

	int choice = 0;
	int position = 0;
	int topicsChosen[3] = {0, 0, 0};
	int numberOfTopics = 0;
	do {
		bool error = false;

		printf("Subscribe to the topic:\n");
		if (topicsChosen[0] == 0)
			printf("1. Music\n");
		if (topicsChosen[1] == 0)
			printf("2. Movies\n");
		if (topicsChosen[2] == 0)
			printf("3. Books\n");
		printf("4. Done\n");

		scanf("%d", &choose);

		switch (choose)
		{
		case 1:
			if (topicsChosen[0] == 0) {
				memcpy(topics + position, "Music,", 6);
				position += 6;
				numberOfTopics += 1;
				topicsChosen[0] = 1;
				break;
			}
			else {
				printf("Unavailable option\n");
				error = true;
				break;
			}
		case 2:
			if (topicsChosen[1] == 0) {
				memcpy(topics + position, "Movies,", 7);
				position += 7;
				numberOfTopics += 1;
				topicsChosen[1] = 1;
				break;
			}
			else {
				printf("Unavailable option\n");
				error = true;
				break;
			}
		case 3:
			if (topicsChosen[2] == 0) {
				memcpy(topics + position, "Books,", 6);
				position += 6;
				numberOfTopics += 1;
				topicsChosen[2] = 1;
				break;
			}
			else {
				printf("Unavailable option\n");
				error = true;
				break;
			}
		case 4:
			if (numberOfTopics == 0) {
				printf("Please select at least one topic.\n\n");
				error = true;
			}
			break;
		default:
			printf("Incorrect input, please choose topic again.\n\n");
			error = true;
			break;
		}

		if (!error) {
			printf("Topics selected: %s\n\n", topics);
		}
	} while (choose != 4 || numberOfTopics == 0);

	position = strlen("Subscriber*");
	if (numberOfTopics == 1) {
		memcpy(messageToSend + position, "1*", 2);
		position += 2;
	}
	else if (numberOfTopics == 2) {
		memcpy(messageToSend + position, "2*", 2);
		position += 2;
	}
	else {
		memcpy(messageToSend + position, "3*", 2);
		position += 2;
	}

	memcpy(messageToSend + position, topics, strlen(topics));
	printf("Message to send: %s\n\n", messageToSend);

	/*printf("Subscribe to the topic:\n");
	printf("1. Music\n");
	printf("2. Movies\n");
	printf("3. Books\n");
	printf("4. Done\n");
	scanf("%d", &choose);

	switch (choose)
	{
	case 1:
		memcpy(messageToSend + strlen("Subscriber*"), "Music", 5);
		break;
	case 2:
		memcpy(messageToSend + strlen("Subscriber*"), "Movies", 6);
		break;
	case 3:
		memcpy(messageToSend + strlen("Subscriber*"), "Books", 5);
		break;

	default:
		break;
	}*/

	Select(connectSocket, false);
	iResult = send(connectSocket, messageToSend, (int)strlen(messageToSend), 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	//printf("Bytes Sent: %ld\n", iResult);
	printf("You are now being notified whenever a message is published in selected topics.\n\n");

	while (true) {
		Select(connectSocket, true);
		iResult = recv(connectSocket, buffer, DEFAULT_BUFLEN, 0);
		if (iResult > 0)
		{
			printf("Message received from server: %s\n", buffer);

		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with server closed.\n");
			closesocket(connectSocket);
			break;
		}
		else
		{
			// there was an error during recv
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			break;
		}
	}

	getchar();
	// cleanup
	closesocket(connectSocket);
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

void Select(SOCKET socket, bool read) {
	while (true) {
		// Initialize select parameters
		FD_SET set;
		timeval timeVal;
		FD_ZERO(&set);
		// Add socket we will wait to read from
		FD_SET(socket, &set);
		// Set timeouts to zero since we want select to return
		// instantaneously
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;
		int iResult;

		if (read) {
			iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		}
		else {
			iResult = select(0 /* ignored */, NULL, &set, NULL, &timeVal);
		}

		if (iResult < 0) {
			printf("Select failed with error: %s", WSAGetLastError());
		}
		else if (iResult == 0) {
			Sleep(100);
			continue;
		}

		return;
	}

}
