#pragma once
int Select(SOCKET socket, bool read, HANDLE semaphore);
int  Select2(SOCKET socket, bool read);
void SendFinishSignal(List* head);

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
		ReleaseSemaphore(ListSemaphoreAt(temp->num, head), 2, NULL);
		temp = temp->next;
	}
}