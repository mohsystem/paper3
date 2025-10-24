
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct Queue {
    Node* front;
    Node* rear;
    int size;
} Queue;

Queue* createQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if (queue == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        exit(1);
    }
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    return queue;
}

bool isEmpty(Queue* queue) {
    if (queue == NULL) {
        return true;
    }
    return queue->front == NULL;
}

void enqueue(Queue* queue, int item) {
    if (queue == NULL) {
        fprintf(stderr, "Queue is NULL\\n");
        return;
    }
    
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return;
    }
    
    newNode->data = item;
    newNode->next = NULL;
    
    if (isEmpty(queue)) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
    queue->size++;
}

int dequeue(Queue* queue) {
    if (queue == NULL || isEmpty(queue)) {
        fprintf(stderr, "Queue is empty or NULL\\n");
        return -1;
    }
    
    Node* temp = queue->front;
    int data = temp->data;
    queue->front = queue->front->next;
    
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    
    free(temp);
    queue->size--;
    return data;
}

int peek(Queue* queue) {
    if (queue == NULL || isEmpty(queue)) {
        fprintf(stderr, "Queue is empty or NULL\\n");
        return -1;
    }
    return queue->front->data;
}

int getSize(Queue* queue) {
    if (queue == NULL) {
        return 0;
    }
    return queue->size;
}

void destroyQueue(Queue* queue) {
    if (queue == NULL) {
        return;
    }
    
    while (!isEmpty(queue)) {
        dequeue(queue);
    }
    free(queue);
}

int main() {
    // Test Case 1: Basic enqueue and dequeue operations
    printf("Test Case 1: Basic operations\\n");
    Queue* queue1 = createQueue();
    enqueue(queue1, 10);
    enqueue(queue1, 20);
    enqueue(queue1, 30);
    printf("Peek: %d\\n", peek(queue1));
    printf("Dequeue: %d\\n", dequeue(queue1));
    printf("Peek after dequeue: %d\\n", peek(queue1));
    printf("\\n");
    destroyQueue(queue1);
    
    // Test Case 2: Multiple enqueue operations
    printf("Test Case 2: Multiple operations\\n");
    Queue* queue2 = createQueue();
    enqueue(queue2, 100);
    enqueue(queue2, 200);
    enqueue(queue2, 300);
    printf("Size: %d\\n", getSize(queue2));
    printf("Dequeue: %d\\n", dequeue(queue2));
    printf("Dequeue: %d\\n", dequeue(queue2));
    printf("\\n");
    destroyQueue(queue2);
    
    // Test Case 3: Empty queue check
    printf("Test Case 3: Empty queue operations\\n");
    Queue* queue3 = createQueue();
    printf("Is empty: %s\\n", isEmpty(queue3) ? "true" : "false");
    enqueue(queue3, 500);
    printf("Is empty after enqueue: %s\\n", isEmpty(queue3) ? "true" : "false");
    dequeue(queue3);
    printf("Is empty after dequeue: %s\\n", isEmpty(queue3) ? "true" : "false");
    printf("\\n");
    destroyQueue(queue3);
    
    // Test Case 4: Multiple operations
    printf("Test Case 4: Multiple operations\\n");
    Queue* queue4 = createQueue();
    for (int i = 1; i <= 5; i++) {
        enqueue(queue4, i * 10);
    }
    printf("Size: %d\\n", getSize(queue4));
    while (!isEmpty(queue4)) {
        printf("Dequeue: %d\\n", dequeue(queue4));
    }
    printf("\\n");
    destroyQueue(queue4);
    
    // Test Case 5: Error handling
    printf("Test Case 5: Error handling\\n");
    Queue* queue5 = createQueue();
    printf("Attempting to dequeue from empty queue:\\n");
    dequeue(queue5);
    destroyQueue(queue5);
    
    return 0;
}
