#pragma once
typedef struct listID {
	int id;
	struct listID* next;
}ListID;

void ListIDAdd(int id, ListID** head);
void ListIDClear(ListID **head);
void ListIDRemoveAt(int index, ListID **head);

void ListIDAdd(int id, ListID ** head)
{
	if (*head == NULL) {
		ListID *element = (ListID*)malloc(sizeof(ListID));
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

void ListIDRemoveAt(int index, ListID ** head)
{
	ListID *temp = *head;
	ListID *pom = NULL;
	while (temp != NULL) {
		if (temp->id == index)
			break;
		pom = temp;
		temp = temp->next;
	}

	if (pom == NULL) {
		ListID *del = temp;
		*head = temp->next;
		free(del);
	}
	else {
		ListID *del = pom->next;
		pom->next = del->next;
		free(del);
	}
}
