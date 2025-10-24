
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_SIZE 1000

typedef struct {
    int data[MAX_SIZE];
    int front;
    int rear;
    int count;
} Task149;

Task149* createQueue() {
    Task149* queue = (Task149*)malloc(sizeof(Task149));
    queue->front = 0;
    queue->rear = -1;
    queue->count = 0;
    return queue;
}

void enqueue(Task149* queue, int value) {
    if (queue->count >= MAX_SIZE) {
        printf("Queue is full\\n");
        return;
    }
    queue->rear = (queue->rear + 1) % MAX_SIZE;
    queue->data[queue->rear] = value;
    queue->count++;
}

int dequeue(Task149* queue, bool* success) {
    if (queue->count == 0) {
        *success = false;
        return -1;
    }
    int value = queue->data[queue->front];
    queue->front = (queue->front + 1) % MAX_SIZE;
    queue->count--;
    *success = true;
    return value;
}

int peek(Task149* queue, bool* success) {
    if (queue->count == 0) {
        *success = false;
        return -1;
    }
    *success = true;
    return queue->data[queue->front];
}

bool isEmpty(Task149* queue) {
    return queue->count == 0;
}

int size(Task149* queue) {
    return queue->count;
}

void destroyQueue(Task149* queue) {
    free(queue);
}

int main() {
    bool success;
    
    // Test Case 1: Basic enqueue and dequeue
    printf("Test Case 1: Basic enqueue and dequeue\\n");
    Task149* q1 = createQueue();
    enqueue(q1, 10);
    enqueue(q1, 20);
    enqueue(q1, 30);
    int val = dequeue(q1, &success);
    if (success) printf("Dequeue: %d\\n", val); // 10
    val = dequeue(q1, &success);
    if (success) printf("Dequeue: %d\\n", val); // 20
    printf("\\n");
    destroyQueue(q1);
    
    // Test Case 2: Peek operation
    printf("Test Case 2: Peek operation\\n");
    Task149* q2 = createQueue();
    enqueue(q2, 5);
    enqueue(q2, 15);
    val = peek(q2, &success);
    if (success) printf("Peek: %d\\n", val); // 5
    val = peek(q2, &success);
    if (success) printf("Peek: %d\\n", val); // 5
    val = dequeue(q2, &success);
    if (success) printf("Dequeue: %d\\n", val); // 5
    val = peek(q2, &success);
    if (success) printf("Peek: %d\\n", val); // 15
    printf("\\n");
    destroyQueue(q2);
    
    // Test Case 3: Empty queue operations
    printf("Test Case 3: Empty queue operations\\n");
    Task149* q3 = createQueue();
    val = dequeue(q3, &success);
    if (!success) printf("Dequeue from empty: Queue is empty\\n");
    val = peek(q3, &success);
    if (!success) printf("Peek from empty: Queue is empty\\n");
    enqueue(q3, 100);
    val = dequeue(q3, &success);
    if (success) printf("Dequeue: %d\\n", val); // 100
    val = dequeue(q3, &success);
    if (!success) printf("Dequeue from empty: Queue is empty\\n");
    printf("\\n");
    destroyQueue(q3);
    
    // Test Case 4: Multiple operations
    printf("Test Case 4: Multiple operations\\n");
    Task149* q4 = createQueue();
    enqueue(q4, 1);
    enqueue(q4, 2);
    val = dequeue(q4, &success);
    if (success) printf("Dequeue: %d\\n", val); // 1
    enqueue(q4, 3);
    enqueue(q4, 4);
    val = peek(q4, &success);
    if (success) printf("Peek: %d\\n", val); // 2
    val = dequeue(q4, &success);
    if (success) printf("Dequeue: %d\\n", val); // 2
    val = dequeue(q4, &success);
    if (success) printf("Dequeue: %d\\n", val); // 3
    printf("\\n");
    destroyQueue(q4);
    
    // Test Case 5: Size check
    printf("Test Case 5: Size check\\n");
    Task149* q5 = createQueue();
    printf("Size: %d\\n", size(q5)); // 0
    enqueue(q5, 50);
    enqueue(q5, 60);
    enqueue(q5, 70);
    printf("Size: %d\\n", size(q5)); // 3
    dequeue(q5, &success);
    printf("Size after dequeue: %d\\n", size(q5)); // 2
    printf("Is empty: %s\\n", isEmpty(q5) ? "true" : "false"); // false
    destroyQueue(q5);
    
    return 0;
}
