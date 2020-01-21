#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27016"
#define MAX_SIZE 100


typedef struct queue {
	char value[MAX_SIZE];
	struct queue *next;
}queue;

typedef struct list
{
	int num;
	SOCKET s;
	DWORD threadID;
	HANDLE clienth;
	struct list *next;
	struct queue *clientMessages;
}List;

typedef struct listID {
	int id;
	struct listID* next;
}ListID;

typedef struct dict{
	char* topic;
	struct listID* clients;
	struct dict* next;
}Dictionary;

int sizeOfMessage = 0;


void InitQueue(queue *head);
void PushInQueue(queue** head, char *val);
int PopFromQueue(queue** head, char *message);
int PopFromQueue2(List** head, char *message, int id);
void Delete_queue(queue* head);

void ListAdd(int number, SOCKET s, DWORD id, HANDLE h, List **head);
int ListCount(List* head);
void ListInsert(int index, int number, SOCKET s, DWORD id, HANDLE h, List **head);
SOCKET ListElementAt(int index, List *head);
HANDLE ListHandleAt(int index, List *head);
queue* ListQueueAt(int index, List *head);
void ModifyListAt(int index, DWORD id, HANDLE handle, list *head);
void ListRemoveAt(int index, List **head);
void ListAddMessageToQueue(int id, char* message, list *head);
void ListClear(List **head);

void DictionaryAddClient(char *topic, int id, Dictionary** head);
ListID* DictionaryGetClients(char *topic, Dictionary* head);

void ListIDAdd(int id, ListID** head);
void ListIDClear(ListID **head);


bool InitializeWindowsSockets();
void Select(SOCKET socket, bool read);
void InitializeDictionary(Dictionary** head);
void DictionaryClear(Dictionary** head);

CRITICAL_SECTION cs;

List* listHead;
Dictionary* dictionary;

DWORD WINAPI ClientThread(LPVOID lpParam) {
	int id = *((int*)lpParam);
	free(lpParam);   // oslobodimo pokazivac

	char buffer[DEFAULT_BUFLEN];
	int iResult = 0;

	EnterCriticalSection(&cs);
	SOCKET acceptedSocket = ListElementAt(id, listHead);
	LeaveCriticalSection(&cs);

	//primi prvu poruku i vidi koji je tip klijenta
	memset(buffer, 0, DEFAULT_BUFLEN);
	char *split;
	// Receive data 
	Select(acceptedSocket, true);
	iResult = recv(acceptedSocket, buffer, DEFAULT_BUFLEN, 0);
	if (iResult > 0)
	{
		//printf("%s \n", buffer);
		printf("Message received from client %d: %s.\n", id, buffer);
		split = strtok(buffer, "*");
		//printf("1. split: %s\n", split);

	}
	else if (iResult == 0)
	{
		// connection was closed gracefully
		printf("Connection with client closed.\n");
		EnterCriticalSection(&cs);
		closesocket(acceptedSocket);
		HANDLE h = ListHandleAt(id, listHead);
		if (h != NULL)
			CloseHandle(h);
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
		HANDLE h = ListHandleAt(id, listHead);
		if (h != NULL)
			CloseHandle(h);
		ListRemoveAt(id, &listHead);
		LeaveCriticalSection(&cs);
		return 0;
	}

	if (strcmp(split, "Publisher") == 0) {
		while (true) {
			memset(buffer, 0, strlen(buffer));
			// Receive data until the client shuts down the connection
			Select(acceptedSocket, true);
			iResult = recv(acceptedSocket, buffer, DEFAULT_BUFLEN, 0);
			if (iResult > 0)
			{
				printf("Message received from client %d: %s.\n", id, buffer);
				// ovaj dio poruke je Publisher
				char *split = strtok(buffer, "*");
				//printf("Message received from client: ");
				// dobicemo topic
				char* topic = strtok(NULL, ":");
				//printf("Topic: %s\n", topic);
				char* text = strtok(NULL, "|");
				//printf("Message: %s\n", message);

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
				// connection was closed gracefully
				printf("Connection with client closed.\n");
				closesocket(acceptedSocket);
				HANDLE h = ListHandleAt(id, listHead);
				if (h != NULL)
					CloseHandle(h);
				ListRemoveAt(id, &listHead);
				break;
			}
			else
			{
				// there was an error during recv
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(acceptedSocket);
				HANDLE h = ListHandleAt(id, listHead);
				if (h != NULL)
					CloseHandle(h);
				ListRemoveAt(id, &listHead);
				break;
			}
		}
	}
	else {
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

		/*char* topics = strtok(NULL, "*");

		EnterCriticalSection(&cs);
		DictionaryAddClient(topics, id, &dictionary);
		LeaveCriticalSection(&cs);*/

		while (true) {
			// proverava da li ima poruka u redu za ovog klijenta, ako ima, salje
			char *message = (char*)malloc(520);
			//memset(message, 0, 520);
			EnterCriticalSection(&cs);
			queue *messages = ListQueueAt(id, listHead);
			LeaveCriticalSection(&cs);
			//char* message = 
			EnterCriticalSection(&cs);
			int ret = PopFromQueue2(&listHead, message, id);
			LeaveCriticalSection(&cs);
			if (ret > 0) {
				EnterCriticalSection(&cs);
				SOCKET s = ListElementAt(id, listHead);
				LeaveCriticalSection(&cs);
				Select(s, false);
				iResult = send(s, message, int(strlen(message)), 0);
			}
			free(message);
			Sleep(500);
		}
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

	//inicijalizacija head cvora u redu
	

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
        // Wait for clients and accept client connections.
        // Returning value is acceptedSocket used for further
        // Client<->Server communication. This version of
        // server will handle only one client.
		Select(listenSocket, true);
        acceptedSocket = accept(listenSocket, NULL, NULL);

        if (acceptedSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

		ListAdd(id, acceptedSocket, 0, NULL, &listHead);
		DWORD dw;
		HANDLE hThread;
		int *param = (int*)malloc(sizeof(int));
		*param = id;
		hThread = CreateThread(NULL, 0, &ClientThread, param, 0, &dw);
		ModifyListAt(id, dw, hThread, listHead);
		id++;

    } while (1);

    // shutdown the connection since we're done
    iResult = shutdown(acceptedSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
        return 1;
    }

	DeleteCriticalSection(&cs);
    // cleanup
    closesocket(listenSocket);
    closesocket(acceptedSocket);
    WSACleanup();

    return 0;
}


void InitQueue(queue *head_node)
{
	head_node->next = NULL;
	memset(head_node->value, 0, MAX_SIZE);
}

void PushInQueue(queue **head, char *val) {

	queue* new_node = (queue*)malloc(sizeof(queue));
	new_node->next = NULL;

	if (*head == NULL)
	{
		*head = new_node;
		memset((*head)->value, 0, MAX_SIZE);
		memcpy((*head)->value, val, strlen(val));
		(*head)->next = NULL;
		
	}
	else 
	{
		queue* current_node = *head;
		while (current_node->next != NULL) {
			current_node = current_node->next;
		}

		
		memset(new_node->value, 0, MAX_SIZE);
		memcpy(new_node->value, val, strlen(val));
		current_node->next = new_node;
	}


}

int PopFromQueue(queue** head, char *message) {
	if (*head == NULL) {
		return 0;
	}
	
	queue* next_node = NULL;
	next_node = (*head)->next;
	//char* retVal = (char*)malloc(strlen((*head)->value) + 1);
	memset(message, 0, 520);
	memcpy(message, (*head)->value, strlen((*head)->value));
	free(*head);
	*head = next_node;
	sizeOfMessage = strlen(message);
	return sizeOfMessage;
}

int PopFromQueue2(List** head, char *message, int id) {

	if (*head != NULL) {
		List *temp = *head;

		while (temp->num != id) {
			temp = temp->next;
		}
		
		if (temp->clientMessages != NULL) {
			queue* q = NULL;
			q = temp->clientMessages;
			temp->clientMessages = temp->clientMessages->next;
			memset(message, 0, 520);
			memcpy(message, q->value, strlen(q->value));
			free(q);
			sizeOfMessage = strlen(message);
			return sizeOfMessage;
		}
	}
	return 0;
}

void Delete_queue(queue* head) {
	queue* current = head;
	queue* next;

	while (current != NULL) {
		next = current->next;
		free(current);
		current = next;
	}
}

void ListAdd(int number, SOCKET s, DWORD id, HANDLE h, List **head)
{
	List* el;
	el = (List*)malloc(sizeof(List));
	el->num = number;
	el->s = s;
	el->threadID = id;
	el->clienth = h;
	el->clientMessages = NULL;
	el->next = NULL;
	if (*head == NULL) {
		*head = el;
	}
	else {
		List *temp = *head;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = el;
	}
}

int ListCount(List* head)
{
	List *temp = head;
	int ret = 0;
	while (temp) {
		ret++;
		temp = temp->next;
	}
	return ret;
}

void ListInsert(int index, int number, SOCKET s, DWORD id, HANDLE h, List **head)
{
	List* novi = (List*)malloc(sizeof(List));
	novi->num = number;
	novi->s = s;
	novi->threadID = id;
	novi->clienth = h;
	novi->clientMessages = NULL;
	if (index == 0) {
		novi->next = *head;
		*head = novi;
	}
	else if (ListCount(*head) > index) {
		int cnt = 0;
		List *temp = *head;

		while (cnt < index - 1) {
			temp = temp->next;
			cnt++;
		}

		novi->next = temp->next;
		temp->next = novi;
	}
}

SOCKET ListElementAt(int index, List *head)
{
	if (ListCount(head) > 0) {
		List *temp = head;

		while (temp->num != index) {
			temp = temp->next;
		}
		return temp->s;
	}
	return NULL;
}

HANDLE ListHandleAt(int index, List *head) {
	if (ListCount(head) > 0) {
		List *temp = head;

		while (temp->num != index) {
			temp = temp->next;
		}
		return temp->clienth;
	}
	return NULL;
}

queue* ListQueueAt(int index, List *head) {
	if (head != NULL) {
		List *temp = head;

		while (temp->num != index) {
			temp = temp->next;
		}
		return temp->clientMessages;
	}
	return NULL;
}

void ListRemoveAt(int index, List **head)
{
	if (index == 0) {
		List *del = *head;
		*head = (*head)->next;
		free(del);
	}
	else {
		List *temp = *head;
		List *pom = NULL;
		while (temp->num != index) {
			pom = temp;
			temp = temp->next;
		}
		List *del = pom->next;
		pom->next = del->next;
		Delete_queue(del->clientMessages);
		free(del);
	}
}

void ListAddMessageToQueue(int id, char * message, list * head)
{
	if (head != NULL) {
		List *temp = head;

		while (temp->num != id) {
			temp = temp->next;
		}
		PushInQueue(&(temp->clientMessages), message);
	}
}

void ModifyListAt(int index, DWORD id, HANDLE handle, list *head) {
	if (ListCount(head) > 0) {
		List *temp = head;

		while (temp->num != index) {
			temp = temp->next;
		}
		temp->threadID = id;
		temp->clienth = handle;
	}
}

void ListClear(List **head)
{
	List *current = *head;
	List *next;

	while (current) {
		closesocket(current->s);
		CloseHandle(current->clienth);
		Delete_queue(current->clientMessages);
		next = current->next;
		free(current);
		current = next;
	}

	*head = NULL;
}

void DictionaryAddClient(char * topic, int id, Dictionary **head)
{
	if (head != NULL) {
		Dictionary *temp = *head;

		while (strcmp(topic, temp->topic)) {
			temp = temp->next;
			if (temp == NULL)
				break;
		}
		ListIDAdd(id, &(temp->clients));
	}
}

ListID* DictionaryGetClients(char * topic, Dictionary *head)
{
	if (head != NULL) {
		Dictionary *temp = head;

		while (strcmp(topic, temp->topic)) {
			temp = temp->next;
		}
		return temp->clients;
	}
	return NULL;
}

void ListIDAdd(int id, ListID ** head)
{
	if (*head == NULL) {
		ListID *element = (ListID*)malloc(sizeof(Dictionary));
		element->id = id;
		element->next = NULL;
		*head = element;
	}
	else {
		ListID *element = (ListID*)malloc(sizeof(ListID));
		element->id = id;
		element->next = NULL;

		ListID *temp = *head;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = element;
	}
}

void ListIDClear(ListID ** head)
{
	ListID *current = *head;
	ListID *next;

	while (current) {
		next = current->next;
		free(current);
		current = next;
	}

	*head = NULL;
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

void InitializeDictionary(Dictionary** head) {
	*head = (Dictionary*)malloc(sizeof(Dictionary));

	(*head)->topic = "Music";
	(*head)->clients = NULL;
	(*head)->next = NULL;

	Dictionary *element = (Dictionary*)malloc(sizeof(Dictionary));
	element->topic = "Movies";
	element->clients = NULL;
	element->next = NULL;
	(*head)->next = element;

	Dictionary *element2 = (Dictionary*)malloc(sizeof(Dictionary));
	element2->topic = "Books";
	element2->clients = NULL;
	element2->next = NULL;
	element->next = element2;
}

void DictionaryClear(Dictionary** head) {
	Dictionary *current = *head;
	Dictionary *next;

	while (current) {
		ListIDClear(&(current->clients));
		next = current->next;
		free(current);
		current = next;
	}

	*head = NULL;
}
