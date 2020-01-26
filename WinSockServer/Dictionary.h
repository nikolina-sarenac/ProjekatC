#pragma once
#include <ListSubscribed.h>
typedef struct dict {
	char* topic;
	struct listID* clients;
	struct dict* next;
}Dictionary;

void DictionaryAddClient(char *topic, int id, Dictionary** head);
void DictionaryRemoveClient(int id, Dictionary** head);
ListID* DictionaryGetClients(char *topic, Dictionary* head);
void InitializeDictionary(Dictionary** head);
void DictionaryClear(Dictionary** head);

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

void DictionaryRemoveClient(int id, Dictionary ** head)
{
	if (head != NULL) {
		Dictionary *temp = *head;

		while (temp != NULL) {
			if (temp->clients != NULL) {
				ListIDRemoveAt(id, &temp->clients);
			}
			temp = temp->next;
		}
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

	Dictionary *element3 = (Dictionary*)malloc(sizeof(Dictionary));
	element3->topic = "History";
	element3->clients = NULL;
	element3->next = NULL;
	element2->next = element3;

	Dictionary *element4 = (Dictionary*)malloc(sizeof(Dictionary));
	element4->topic = "Weather";
	element4->clients = NULL;
	element4->next = NULL;
	element3->next = element4;
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