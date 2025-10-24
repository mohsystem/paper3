#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Node structure for the linked list
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Queue structure
typedef struct Queue {
    Node *front, *rear;
} Queue;

// Function to create a new node
Node* newNode(int data) {
    Node* temp = (Node*)malloc(sizeof(Node));
    if (!temp) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    temp->data = data;
    temp->next = NULL;
    return temp;
}

// Function to create an empty queue
Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    return q;
}

// Function to add an item to the queue.
void enqueue(Queue* q, int item) {
    Node* temp = newNode(item);
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}

// Function to remove an item from the queue.
int dequeue(Queue* q) {
    if (q->front == NULL) {
        fprintf(stderr, "Queue is empty. Cannot dequeue.\n");
        return INT_MIN; // Return sentinel value for error
    }
    Node* temp = q->front;
    int item = temp->data;
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    return item;
}

// Function to get the front of the queue.
int peek(Queue* q) {
    if (q->front == NULL) {
        fprintf(stderr, "Queue is empty. Cannot peek.\n");
        return INT_MIN; // Return sentinel value for error
    }
    return q->front->data;
}

// Function to check if the queue is empty.
int isEmpty(Queue* q) {
    return (q->front == NULL);
}

// Function to free the entire queue
void freeQueue(Queue* q) {
    if (q == NULL) return;
    Node* current = q->front;
    Node* next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    free(q);
}

int main() {
    printf("C Queue Implementation Test\n");
    Queue* queue = createQueue();

    // Test Case 1: Enqueue elements
    printf("Test Case 1: Enqueue 10, 20, 30\n");
    enqueue(queue, 10);
    enqueue(queue, 20);
    enqueue(queue, 30);
    printf("Front element is: %d\n", peek(queue));

    // Test Case 2: Dequeue an element
    printf("\nTest Case 2: Dequeue\n");
    printf("Dequeued element: %d\n", dequeue(queue));
    printf("Front element is now: %d\n", peek(queue));

    // Test Case 3: Enqueue another element
    printf("\nTest Case 3: Enqueue 40\n");
    enqueue(queue, 40);
    printf("Front element is: %d\n", peek(queue));

    // Test Case 4: Dequeue all elements
    printf("\nTest Case 4: Dequeue all elements\n");
    printf("Dequeued element: %d\n", dequeue(queue)); // 20
    printf("Dequeued element: %d\n", dequeue(queue)); // 30
    printf("Dequeued element: %d\n", dequeue(queue)); // 40
    printf("Is queue empty? %s\n", isEmpty(queue) ? "true" : "false");

    // Test Case 5: Underflow condition
    printf("\nTest Case 5: Underflow condition\n");
    int peek_val = peek(queue);
    if (peek_val == INT_MIN) {
        printf("Peek on empty queue handled correctly.\n");
    }
    int dequeue_val = dequeue(queue);
    if (dequeue_val == INT_MIN) {
        printf("Dequeue on empty queue handled correctly.\n");
    }
    
    // Clean up allocated memory
    freeQueue(queue);
    
    return 0;
}