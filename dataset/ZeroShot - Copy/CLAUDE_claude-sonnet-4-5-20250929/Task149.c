
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct Task149 {
    Node* front;
    Node* rear;
    int size;
} Task149;

Task149* createQueue() {
    Task149* queue = (Task149*)malloc(sizeof(Task149));
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    return queue;
}

void enqueue(Task149* queue, int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    
    if (queue->rear == NULL) {
        queue->front = queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
    queue->size++;
}

int dequeue(Task149* queue) {
    if (queue->front == NULL) {
        printf("Queue is empty\\n");
        exit(1);
    }
    int data = queue->front->data;
    Node* temp = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(temp);
    queue->size--;
    return data;
}

int peek(Task149* queue) {
    if (queue->front == NULL) {
        printf("Queue is empty\\n");
        exit(1);
    }
    return queue->front->data;
}

bool isEmpty(Task149* queue) {
    return queue->front == NULL;
}

int getSize(Task149* queue) {
    return queue->size;
}

void freeQueue(Task149* queue) {
    while (!isEmpty(queue)) {
        dequeue(queue);
    }
    free(queue);
}

int main() {
    // Test case 1: Basic enqueue and dequeue
    printf("Test Case 1: Basic enqueue and dequeue\\n");
    Task149* queue1 = createQueue();
    enqueue(queue1, 10);
    enqueue(queue1, 20);
    enqueue(queue1, 30);
    printf("Dequeue: %d\\n", dequeue(queue1));
    printf("Peek: %d\\n", peek(queue1));
    printf("Size: %d\\n", getSize(queue1));
    printf("\\n");
    freeQueue(queue1);
    
    // Test case 2: Enqueue and peek without dequeue
    printf("Test Case 2: Enqueue and peek\\n");
    Task149* queue2 = createQueue();
    enqueue(queue2, 5);
    enqueue(queue2, 15);
    printf("Peek: %d\\n", peek(queue2));
    printf("Peek again: %d\\n", peek(queue2));
    printf("Size: %d\\n", getSize(queue2));
    printf("\\n");
    freeQueue(queue2);
    
    // Test case 3: Multiple operations
    printf("Test Case 3: Multiple operations\\n");
    Task149* queue3 = createQueue();
    enqueue(queue3, 1);
    enqueue(queue3, 2);
    printf("Dequeue: %d\\n", dequeue(queue3));
    enqueue(queue3, 3);
    enqueue(queue3, 4);
    printf("Dequeue: %d\\n", dequeue(queue3));
    printf("Peek: %d\\n", peek(queue3));
    printf("Size: %d\\n", getSize(queue3));
    printf("\\n");
    freeQueue(queue3);
    
    // Test case 4: Empty queue
    printf("Test Case 4: Empty queue check\\n");
    Task149* queue4 = createQueue();
    printf("Is empty: %s\\n", isEmpty(queue4) ? "true" : "false");
    enqueue(queue4, 100);
    printf("Is empty: %s\\n", isEmpty(queue4) ? "true" : "false");
    dequeue(queue4);
    printf("Is empty after dequeue: %s\\n", isEmpty(queue4) ? "true" : "false");
    printf("\\n");
    freeQueue(queue4);
    
    // Test case 5: Large number of operations
    printf("Test Case 5: Large number of operations\\n");
    Task149* queue5 = createQueue();
    for (int i = 1; i <= 5; i++) {
        enqueue(queue5, i * 10);
    }
    printf("Size: %d\\n", getSize(queue5));
    printf("Dequeue: %d\\n", dequeue(queue5));
    printf("Dequeue: %d\\n", dequeue(queue5));
    printf("Peek: %d\\n", peek(queue5));
    printf("Size: %d\\n", getSize(queue5));
    freeQueue(queue5);
    
    return 0;
}
