#pragma once
#define SAFE_DELETE_HANDLE(a) if(a){CloseHandle(a);} 
typedef struct list
{
	int num;
	SOCKET s;
	DWORD threadID;
	HANDLE clienth;
	HANDLE semaphore;
	struct list *next;
	struct queue *clientMessages;
}List;

void ListAdd(int number, SOCKET s, DWORD id, HANDLE h, HANDLE sem, List **head);
int ListCount(List* head);
void ListInsert(int index, int number, SOCKET s, DWORD id, HANDLE h, List **head);
SOCKET ListElementAt(int index, List *head);
HANDLE ListHandleAt(int index, List *head);
HANDLE ListSemaphoreAt(int index, List *head);

void ModifyListAt(int index, DWORD id, HANDLE handle, list *head);



void ListAdd(int number, SOCKET s, DWORD id, HANDLE h, HANDLE sem, List **head)
{
	List* el;
	el = (List*)malloc(sizeof(List));
	el->num = number;
	el->s = s;
	el->threadID = id;
	el->semaphore = sem;
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
	if (head != NULL) {
		List *temp = head;

		while (temp->num != index) {
			temp = temp->next;
		}
		return temp->s;
	}
	return NULL;
}

HANDLE ListHandleAt(int index, List *head) {
	if (head != NULL) {
		List *temp = head;

		while (temp->num != index) {
			temp = temp->next;
		}
		return temp->clienth;
	}
	return NULL;
}

HANDLE ListSemaphoreAt(int index, List *head) {
	if (head != NULL) {
		List *temp = head;

		while (temp->num != index) {
			temp = temp->next;
		}
		return temp->semaphore;
	}
	return NULL;
}

void ModifyListAt(int index, DWORD id, HANDLE handle, list *head) {
	if (head != NULL) {
		List *temp = head;

		while (temp->num != index) {
			temp = temp->next;
		}
		temp->threadID = id;
		temp->clienth = handle;
	}
}



