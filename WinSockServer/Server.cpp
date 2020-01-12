#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

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
}List;

int sizeOfMessage = 0;


void InitQueue(queue *head);
void PushInQueue(queue* head, char *val);
char* PopFromQueue(queue** head);
void Delete_queue(queue* head);

void Add(int number, SOCKET s, DWORD id, HANDLE h, List **head);
int Count(List* head);
void Insert(int index, int number, SOCKET s, DWORD id, HANDLE h, List **head);
SOCKET ElementAt(int index, List *head);
HANDLE HandleAt(int index, List *head);
void ModifyListAt(int index, DWORD id, HANDLE handle, list *head);
void RemoveAt(int index, List **head);
void Clear(List **head);


bool InitializeWindowsSockets();
void Select(SOCKET socket, bool read);

List* listHead;

DWORD WINAPI ClientThread(LPVOID lpParam) {
	int *idP = (int*)lpParam;  // pokazivac na int
	int id = *idP;  // sacuvamo id
	free(idP);   // oslobodimo pokazivac

	char buffer[DEFAULT_BUFLEN];
	int iResult = 0;

	SOCKET acceptedSocket = ElementAt(id, listHead);

	while (true) {
		memset(buffer, 0, strlen(buffer));
		// Receive data until the client shuts down the connection
		Select(acceptedSocket, true);
		iResult = recv(acceptedSocket, buffer, DEFAULT_BUFLEN, 0);
		if (iResult > 0)
		{
			printf("Message received from client: %s.\n", buffer);

		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with client closed.\n");
			closesocket(acceptedSocket);
			HANDLE h = HandleAt(id, listHead);
			if (h != NULL)
				CloseHandle(h);
			RemoveAt(id, &listHead);
			break;
		}
		else
		{
			// there was an error during recv
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(acceptedSocket);
			HANDLE h = HandleAt(id, listHead);
			if (h != NULL)
				CloseHandle(h);
			RemoveAt(id, &listHead);
			break;
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

	//inicijalizacija head cvora u redu
	queue* head_node = (queue*)malloc(sizeof(queue));


    
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

	InitQueue(head_node);
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

		Add(id, acceptedSocket, 0, NULL, &listHead);
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

void PushInQueue(queue* head, char *val) {
	queue* current_node = head;
	while (current_node->next != NULL) {
		current_node = current_node->next;
	}

	queue* new_node = (queue*)malloc(sizeof(queue));
	new_node->next = NULL;
	memset(current_node->value, 0, MAX_SIZE);
	memcpy(current_node->value, val, strlen(val));
	current_node->next = new_node;
}

char* PopFromQueue(queue** head) {
	if (*head == NULL) {
		return NULL;
	}
	
	queue* next_node = NULL;
	next_node = (*head)->next;
	char retVal[MAX_SIZE];
	memset(retVal, 0, MAX_SIZE);
	memcpy(retVal, (*head)->value, strlen((*head)->value));
	free(*head);
	*head = next_node;
	sizeOfMessage = strlen(retVal);
	return retVal;
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

void Add(int number, SOCKET s, DWORD id, HANDLE h, List **head)
{
	List* el;
	el = (List*)malloc(sizeof(List));
	el->num = number;
	el->s = s;
	el->threadID = id;
	el->clienth = h;
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

int Count(List* head)
{
	List *temp = head;
	int ret = 0;
	while (temp) {
		ret++;
		temp = temp->next;
	}
	return ret;
}

void Insert(int index, int number, SOCKET s, DWORD id, HANDLE h, List **head)
{
	List* novi = (List*)malloc(sizeof(List));
	novi->num = number;
	novi->s = s;
	novi->threadID = id;
	novi->clienth = h;
	if (index == 0) {
		novi->next = *head;
		*head = novi;
	}
	else if (Count(*head) > index) {
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

SOCKET ElementAt(int index, List *head)
{
	if (Count(head) > 0) {
		List *temp = head;

		while (temp->num != index) {
			temp = temp->next;
		}
		return temp->s;
	}
	return NULL;
}

HANDLE HandleAt(int index, List *head) {
	if (Count(head) > 0) {
		List *temp = head;

		while (temp->num != index) {
			temp = temp->next;
		}
		return temp->clienth;
	}
	return NULL;
}

void RemoveAt(int index, List **head)
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
		free(del);
	}
}

void ModifyListAt(int index, DWORD id, HANDLE handle, list *head) {
	if (Count(head) > 0) {
		List *temp = head;

		while (temp->num != index) {
			temp = temp->next;
		}
		temp->threadID = id;
		temp->clienth = handle;
	}
}

void Clear(List **head)
{
	List *current = *head;
	List *next;

	while (current) {
		closesocket(current->s);
		CloseHandle(current->clienth);
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
