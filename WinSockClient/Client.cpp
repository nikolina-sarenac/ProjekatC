#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

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
	int choose = 0;
	char message[MAX_SIZE];
	memset(message, 0, MAX_SIZE);
    // Validate the parameters

    if(InitializeWindowsSockets() == false)
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

	while (true) {
		memset(messageToSend, 0, MAX_SIZE);
		memcpy(messageToSend, "Publisher", strlen("Publisher"));
		printf("Choose the topic:\n");
		printf("1. Music\n");
		printf("2. Movie\n");
		printf("3. Books\n");
		scanf("%d", &choose);


		switch (choose)
		{
		case 1:
			memset(messageToSend + strlen("Publisher"), choose, sizeof(char));
		case 2:
			memset(messageToSend + strlen("Publisher"), choose, sizeof(char));
		case 3:
			memset(messageToSend + strlen("Publisher"), choose, sizeof(char));

		default:
			break;
		}

		if (choose > 0 && choose < 4) {
			printf("Write the message: ");
			while (message[0] == 0 || message[0] == 10) {
				fgets(message, MAX_SIZE, stdin);
			}
		}
		else {
			printf("Incorrect option");
		}
		
		//scanf("%s", message);
		memcpy(messageToSend + strlen("Publisher") + 1, message, strlen(message));

		Select(connectSocket, false);
		// Send an prepared message with null terminator included
		iResult = send( connectSocket, messageToSend, (int)strlen(messageToSend) + 1, 0 );

		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}

		printf("Bytes Sent: %ld\n", iResult);
		message[0] = 0;

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
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
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
