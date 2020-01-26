#pragma once
queue* ListQueueAt(int index, List *head);
void ListRemoveAt(int index, List **head);
void ListAddMessageToQueue(int id, char* message, list *head);
void ListClear(List **head);

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

void ListClear(List **head)
{
	List *current = *head;
	List *next;

	while (current) {
		int iResult = shutdown(current->s, SD_SEND);
		if (iResult == SOCKET_ERROR)
		{
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(current->s);
		}
		else {
			closesocket(current->s);
		}
		SAFE_DELETE_HANDLE(current->clienth);
		SAFE_DELETE_HANDLE(current->semaphore);
		Delete_queue(current->clientMessages);
		next = current->next;
		free(current);
		current = next;
	}
	*head = NULL;
}



void ListRemoveAt(int index, List **head)
{
	List *temp = *head;
	List *pom = NULL;
	while (temp->num != index) {
		pom = temp;
		temp = temp->next;
	}
	if (pom == NULL) {
		List *del = temp;
		*head = temp->next;
		Delete_queue(del->clientMessages);

		SAFE_DELETE_HANDLE(del->clienth);
		SAFE_DELETE_HANDLE(del->semaphore);
		free(del);
	}
	else {
		List *del = pom->next;
		pom->next = del->next;
		Delete_queue(del->clientMessages);

		SAFE_DELETE_HANDLE(del->clienth);
		SAFE_DELETE_HANDLE(del->semaphore);

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
