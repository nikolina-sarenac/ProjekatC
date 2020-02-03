#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <iostream>
#include <ListSubscribed.h>
#include <Dictionary.h>
#include <ListUsers.h>
#include <Queue.h>
#include <ListAndQueue.h>
#include <Helper.h>
#include <ClientFunctions.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27016"
#define MAX_SIZE 200

#define SAFE_DELETE_HANDLE(a) if(a){CloseHandle(a);} 

bool InitializeWindowsSockets();

CRITICAL_SECTION cs;
List* listHead;
Dictionary* dictionary;

DWORD WINAPI ClientThread(LPVOID lpParam) {
	int id = *((int*)lpParam);
	free(lpParam);   // oslobodimo pokazivac

	char buffer[DEFAULT_BUFLEN];     // bafer za poruke koje pristizu
	memset(buffer, 0, DEFAULT_BUFLEN);

	EnterCriticalSection(&cs);
	SOCKET acceptedSocket = ListElementAt(id, listHead);
	HANDLE semaphore = ListSemaphoreAt(id, listHead);
	LeaveCriticalSection(&cs);

	char *clientType;
	int iResult = Select(acceptedSocket, true, semaphore);
	if (iResult == 1)
		return 0;
	iResult = recv(acceptedSocket, buffer, DEFAULT_BUFLEN, 0);
	if (iResult > 0)
	{
		//printf("Message received from client %d: %s.\n", id, buffer);
		clientType = strtok(buffer, "*");
	}
	else if (iResult == 0)
	{
		printf("Connection with client %d closed.\n", id);
		closesocket(acceptedSocket);
		EnterCriticalSection(&cs);
		ListRemoveAt(id, &listHead);
		LeaveCriticalSection(&cs);
		return 0;
	}
	else
	{
		// there was an error during recv
		printf("recv failed with error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket);
		EnterCriticalSection(&cs);
		ListRemoveAt(id, &listHead);
		LeaveCriticalSection(&cs);
		return 0;
	}

	if (strcmp(clientType, "Publisher") == 0) {
		PublisherFunction(id, acceptedSocket, semaphore);
	}
	else if (strcmp(clientType, "Subscriber") == 0) {
		SubscriberFunction(id, acceptedSocket, semaphore);
	}

	memset(buffer, 0, strlen(buffer));
	return 0;
}

int  main(void) 
{
    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // Buffer used for storing incoming data
    char recvbuf[DEFAULT_BUFLEN];

	int id = 0;
	dictionary = NULL;
	listHead = NULL;
	InitializeDictionary(&dictionary);

    if(InitializeWindowsSockets() == false)
    {
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
    }
    
    // Prepare address information structures
    addrinfo *resultingAddress = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 address
    hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
    hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
    hints.ai_flags = AI_PASSIVE;     // 

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
    if ( iResult != 0 )
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket(AF_INET,      // IPv4 address famly
                          SOCK_STREAM,  // stream socket
                          IPPROTO_TCP); // TCP

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket - bind port number and local address 
    // to socket
    iResult = bind( listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Since we don't need resultingAddress any more, free it
    freeaddrinfo(resultingAddress);

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

	unsigned long mode = 1;
	if (ioctlsocket(listenSocket, FIONBIO, &mode) == SOCKET_ERROR) {
		printf("ioctl failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server initialized, waiting for clients.\n");
	InitializeCriticalSection(&cs);

    do
    {
		if (_kbhit())
			break;
		else {

			if (Select2(listenSocket, true) == 1)
				break;
			acceptedSocket = accept(listenSocket, NULL, NULL);

			if (acceptedSocket == INVALID_SOCKET)
			{
				printf("accept failed with error: %d\n", WSAGetLastError());
				closesocket(listenSocket);
				WSACleanup();
				return 1;
			}

			ListAdd(id, acceptedSocket, 0, NULL, CreateSemaphore(0, 0, 2, NULL), &listHead);
			DWORD dw;
			HANDLE hThread;
			int *param = (int*)malloc(sizeof(int));
			*param = id;
			hThread = CreateThread(NULL, 0, &ClientThread, param, 0, &dw);
			ModifyListAt(id, dw, hThread, listHead);
			id++;
		}
        

    } while (1);

	//printf("Server is sending finish signals for all threads");
	SendFinishSignal(listHead);

	List* temp = listHead;
	while (temp != NULL) {
		WaitForSingleObject(temp->clienth, INFINITE);
		temp = temp->next;
	}

	DeleteCriticalSection(&cs);
    // cleanup
    closesocket(listenSocket);
    closesocket(acceptedSocket);
	DictionaryClear(&dictionary);
	ListClear(&listHead);
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

int PublisherFunction(int id, SOCKET acceptedSocket, HANDLE semaphore)
{
	while (WaitForSingleObject(semaphore, 0L) != WAIT_OBJECT_0) {
		char recvBuffer[MAX_SIZE];
		memset(recvBuffer, 0, MAX_SIZE);

		int iResult = Receive(acceptedSocket, recvBuffer, MAX_SIZE, semaphore);
		if (iResult > 0)
		{
			//printf("Message received from client %d: %s.\n", id, recvBuffer);
			// ovaj dio poruke je Publisher
			char *split = strtok(recvBuffer, "*");
			// dobicemo topic
			char* topic = strtok(NULL, ":");
			// printf("Topic: %s\n", topic);
			char* text = strtok(NULL, "|");
			// printf("Message: %s\n", message);

			EnterCriticalSection(&cs);
			ListID* clientIDs = DictionaryGetClients(topic, dictionary);
			LeaveCriticalSection(&cs);

			char message[MAX_SIZE];
			memset(message, 0, MAX_SIZE);
			memcpy(message, topic, strlen(topic));
			memcpy(message + strlen(topic), ":", 1);
			memcpy(message + strlen(topic) + 1, text, strlen(text));

			while (clientIDs != NULL) {
				EnterCriticalSection(&cs);
				ListAddMessageToQueue(clientIDs->id, message, listHead);
				LeaveCriticalSection(&cs);

				clientIDs = clientIDs->next;
			}
		}
		else if (iResult == 0)
		{
			closesocket(acceptedSocket);
			ListRemoveAt(id, &listHead);
			return 0;
		}
		else if (iResult == -1) 
		{
			return 0;
		}
		else
		{
			closesocket(acceptedSocket);
			ListRemoveAt(id, &listHead);
			return 0;
		}
	}
}

int SubscriberFunction(int id, SOCKET acceptedSocket, HANDLE semaphore)
{
	char* numberOfTopics = strtok(NULL, "*");
	int number = atoi(numberOfTopics);

	char* topics = strtok(NULL, "*");
	char* topic = strtok(topics, ",");
	while (topic != NULL)
	{
		EnterCriticalSection(&cs);
		DictionaryAddClient(topic, id, &dictionary);
		LeaveCriticalSection(&cs);

		topic = strtok(NULL, ",");
	}


	while (WaitForSingleObject(semaphore, 0L) != WAIT_OBJECT_0) {
		// proverava da li ima poruka u redu za ovog klijenta, ako ima, salje
		char *message = (char*)malloc(MAX_SIZE);

		EnterCriticalSection(&cs);
		int ret = PopFromQueue2(&listHead, message, id);
		LeaveCriticalSection(&cs);

		if (ret > 0) {
			int iResult = Send(acceptedSocket, message, MAX_SIZE, semaphore);

			if (iResult == -2) {   // dobijen signal na semaforu
				free(message);
				return 0;
			}
			else if (iResult == -1)
			{
				free(message);
				printf("Client %d disconnected.\n", id);
				closesocket(acceptedSocket);
				DictionaryRemoveClient(id, &dictionary);
				ListRemoveAt(id, &listHead);
				return 0;
			}
		}
		//Sleep(50);
		free(message);
	}
}







