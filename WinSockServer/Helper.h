#pragma once
int Select(SOCKET socket, bool read, HANDLE semaphore);
int  Select2(SOCKET socket, bool read);
void SendFinishSignal(List* head);
int Receive(SOCKET acceptedSocket, char* recvbuf, int size, HANDLE sem);
int Send(SOCKET acceptedSocket, char* messageToSend, int len, HANDLE sem);

int Select2(SOCKET socket, bool read) {
	while (!_kbhit()) {
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
			//printf("Select failed with error: %s", WSAGetLastError());
		}
		else if (iResult == 0) {
			Sleep(100);
			continue;
		}

		return 0;
	}
	return 1;
}

int Select(SOCKET socket, bool read, HANDLE semaphore) {
	while (WaitForSingleObject(semaphore, 0L) != WAIT_OBJECT_0) {
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
			//printf("Select failed with error: %s", WSAGetLastError());
		}
		else if (iResult == 0) {
			Sleep(100);
			continue;
		}

		return 0;
	}
	return 1;
}


void SendFinishSignal(List * head)
{
	List* temp = head;
	while (temp != NULL) {
		ReleaseSemaphore(ListSemaphoreAt(temp->id, head), 2, NULL);
		temp = temp->next;
	}
}

int Receive(SOCKET acceptedSocket, char* recvbuf, int size, HANDLE sem)
{
	int brojac = 0;

	while (brojac < size) {
		if (Select(acceptedSocket, true, sem) == 1)
			return -1;
		int res = recv(acceptedSocket, recvbuf + brojac, size - brojac, 0);
		if (res > 0)
			brojac += res;
		else {
			break;
		}
	}
	return brojac;
}

int Send(SOCKET acceptedSocket, char* messageToSend, int len, HANDLE sem)
{
	int brojac = 0;

	while (brojac < len) {
		if (Select(acceptedSocket, false, sem) == 1)
			return -2;
		int res = send(acceptedSocket, messageToSend + brojac, len - brojac, 0);
		if (res > 0) {
			brojac += res;
		}
		else if (res < 0) {
			return res;
		}
	}
	return brojac;
}