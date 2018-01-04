#ifndef QUEUE_H
#define QUEUE_H

struct node
{
	int value;
	struct node* next;
	struct node* prev;
};

struct queue
{
	int size;
	struct node* head;
	struct node* tail;
};

int is_queue_empty(struct queue* q);
int pop_queue(struct queue* q);
void push_queue(struct queue* q, int value);
struct queue* create_empty_queue();

#endif  // QUEUE_H
