#pragma once
#define MAX_SIZE 200

typedef struct queue {
	char value[MAX_SIZE];
	struct queue *next;
}queue;

void InitQueue(queue *head);
void PushInQueue(queue** head, char *val);
int PopFromQueue(queue** head, char *message);
int PopFromQueue2(List** head, char *message, int id);
void Delete_queue(queue* head);

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
	memset(message, 0, MAX_SIZE);
	memcpy(message, (*head)->value, strlen((*head)->value));
	free(*head);
	*head = next_node;
	//sizeOfMessage = strlen(message);
	return strlen(message);
}

int PopFromQueue2(List** head, char *message, int id) {

	if (*head != NULL) {
		List *temp = *head;

		while (temp->id != id) {
			temp = temp->next;
		}

		if (temp->clientMessages != NULL) {
			queue* q = NULL;
			q = temp->clientMessages;
			temp->clientMessages = temp->clientMessages->next;
			memset(message, 0, MAX_SIZE);
			memcpy(message, q->value, strlen(q->value));
			free(q);
			return strlen(message);
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

