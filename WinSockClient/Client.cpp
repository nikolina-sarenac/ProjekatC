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
#define MAX_SIZE 200

// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();
void Select(SOCKET socket, bool read);
int Send(SOCKET connectSocket, char* messageToSend, int len);

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

	Select(connectSocket, false);
	// Send an prepared message with null terminator included
	iResult = send(connectSocket, "Publisher*X", (int)strlen("Publisher*X"), 0);

	while (true) {
		printf("Select action:\n");
		printf("1. Test\n");
		printf("2. Publish messages\n");
		printf("3. Exit\n");
		scanf("%d", &choose);
		bool stop = false;

		switch (choose)
		{
		case 1:
			printf("Sending messages to random topics. Press 'S' to stop.\n");
			
			while (!stop) {
				if (_kbhit()) {
					char c = getch();
					if (c == 's' || c == 'S') {
						stop = true;
						break;
					}
				}
				char * message = "Random message for topic";

				for (int i = 0; i < 500; i++) {
					if (_kbhit()) {
						char c = getch();
						if (c == 's' || c == 'S') {
							stop = true;
							break;
						}
					}
					int topic = rand() % 5; // random brojevi 0-4

					memset(messageToSend, 0, MAX_SIZE);

					memcpy(messageToSend, "Publisher*", strlen("Publisher*"));
					int pos = strlen("Publisher*");

					switch (topic) {
					case 0:
						memcpy(messageToSend + pos, "Music:", strlen("Music:"));
						pos = pos + strlen("Music:");
						break;
					case 1:
						memcpy(messageToSend + pos, "Movies:", strlen("Movies:"));
						pos = pos + strlen("Movies:");
						break;
					case 2:
						memcpy(messageToSend + pos, "Books:", strlen("Books:"));
						pos = pos + strlen("Books:");
						break;
					case 3:
						memcpy(messageToSend + pos, "History:", strlen("History:"));
						pos = pos + strlen("History:");
						break;
					case 4:
						memcpy(messageToSend + pos, "Weather:", strlen("Weather:"));
						pos = pos + strlen("Weather:");
						break;
					default:
						break;
					}

					memcpy(messageToSend + pos, message, strlen(message));

					Select(connectSocket, false);
					// Send an prepared message with null terminator included
					printf("%s\n", messageToSend);
					//iResult = send(connectSocket, messageToSend, (int)strlen(messageToSend) + 1, 0);
					iResult = Send(connectSocket, messageToSend, MAX_SIZE);
					if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(connectSocket);
						WSACleanup();
						return 1;
					}
					//Sleep(1000);
				}
				

				// slati poruku na random temu
				printf("%s\n", message);
				Sleep(200);
			}
			break;
		case 2:
			while (true) {
				bool ret = false;

				memset(messageToSend, 0, MAX_SIZE);
				memcpy(messageToSend, "Publisher*", strlen("Publisher*"));
				printf("Choose the topic:\n");
				printf("1. Music\n");
				printf("2. Movies\n");
				printf("3. Books\n");
				printf("4. History\n");
				printf("5. Weather\n");
				printf("6. Return\n");
				scanf("%d", &choose);

				int pos = strlen("Publisher*");

				switch (choose)
				{
				case 1:
					memcpy(messageToSend + pos, "Music:", strlen("Music:"));
					pos = pos + strlen("Music:");
					break;
				case 2:
					memcpy(messageToSend + pos, "Movies:", strlen("Movies:"));
					pos = pos + strlen("Movies:");
					break;
				case 3:
					memcpy(messageToSend + pos, "Books:", strlen("Books:"));
					pos = pos + strlen("Books:");
					break;
				case 4:
					memcpy(messageToSend + pos, "History:", strlen("History:"));
					pos = pos + strlen("History:");
					break;
				case 5:
					memcpy(messageToSend + pos, "Weather:", strlen("Weather:"));
					pos = pos + strlen("Weather:");
					break;
				case 6:
					break;

				default:
					break;
				}

				if (choose > 0 && choose < 6) {
					printf("Write the message: ");
					while (message[0] == 0 || message[0] == 10) {
						fgets(message, MAX_SIZE, stdin);
					}
				}
				else if (choose == 6) {
					break;
				}
				else {
					printf("Incorrect option");
					break;
				}

				memcpy(messageToSend + pos, message, strlen(message));
				memcpy(messageToSend + pos + strlen(message), " ", 1);

				Select(connectSocket, false);
				// Send an prepared message with null terminator included
				iResult = Send(connectSocket, messageToSend, MAX_SIZE);

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
			break;
		case 3:

			break;

		default:
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

int Send(SOCKET connectSocket, char* messageToSend, int len)
{
	int brojac = 0;

	while (brojac < len) {
		Select(connectSocket, false);
		int res = send(connectSocket, messageToSend + brojac, len - brojac, 0);
		if (res > 0) {
			brojac += res;
		}
	}
	return brojac;
}
