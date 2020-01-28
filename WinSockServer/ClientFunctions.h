#pragma once
int PublisherFunction(int id, SOCKET acceptedSocket, HANDLE semaphore);
int SubscriberFunction(int id, SOCKET acceptedSocket, HANDLE semaphore);


