#include "queue.h"
#include <stdlib.h>

int is_queue_empty(struct queue* q)
{
	if (q == NULL || q->size == 0)
		return 1;
	return 0;
}

int pop_queue(struct queue* q)
{
	if (q == NULL || q->size == 0)
		return -1;
	struct node* to_return = q->head;
	int to_return_value = to_return->value;
	q->size = q->size - 1;
	if (q->size == 0)
	{
		q->head = NULL;
		q->tail = NULL;
	}
	else
	{
		q->head = q->head->next;
		q->head->prev = NULL;
	}
	free(to_return);
	return to_return_value;
}

void push_queue(struct queue* q, int value)
{
	if (q == NULL)
		return;
	struct node* new_node = (struct node*)malloc(sizeof(struct node));
	new_node->value = value;
	if (is_queue_empty(q))
	{
		q->head = new_node;
		q->head->next = NULL;
		q->head->prev = NULL;
		q->tail = new_node;
		q->tail->next = NULL;
		q->tail->prev = NULL;
	}
	else if (q->head->next == NULL)
	{
		// queue size is 1
		q->tail = new_node;
		q->tail->prev = q->head;
		q->tail->next = NULL;
		q->head->next = new_node;
	}
	else
	{
		// queue size is > 1
		q->tail->next = new_node;
		new_node->prev = q->tail;
		new_node->next = NULL;
		q->tail = new_node;
	}
	q->size = q->size + 1;
}

struct queue* create_empty_queue()
{
	struct queue* q = (struct queue*)malloc(sizeof(struct queue));
	q->size = 0;
	q->head = NULL;
	q->tail = NULL;
	return q;
}
