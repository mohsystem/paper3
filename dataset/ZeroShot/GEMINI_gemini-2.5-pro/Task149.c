#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // For INT_MIN

// Node structure
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Queue structure
typedef struct Queue {
    Node *front;
    Node *rear;
} Queue;

// Function to create an empty queue
Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        perror("Failed to allocate memory for queue");
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    return q;
}

// Function to check if the queue is empty
int isEmpty(Queue* q) {
    return q->front == NULL;
}

// Function to add an element to the queue
void enqueue(Queue* q, int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        perror("Failed to allocate memory for node");
        return; // Or exit, depending on desired error handling
    }
    newNode->data = data;
    newNode->next = NULL;

    if (isEmpty(q)) {
        q->front = q->rear = newNode;
        return;
    }
    q->rear->next = newNode;
    q->rear = newNode;
}

// Function to remove an element from the queue
int dequeue(Queue* q) {
    if (isEmpty(q)) {
        fprintf(stderr, "Error: Queue is empty. Cannot dequeue.\n");
        return INT_MIN; // Error value
    }
    Node* temp = q->front;
    int data = temp->data;
    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    return data;
}

// Function to get the front element of the queue
int peek(Queue* q) {
    if (isEmpty(q)) {
        fprintf(stderr, "Error: Queue is empty. Cannot peek.\n");
        return INT_MIN; // Error value
    }
    return q->front->data;
}

// Function to free the entire queue
void destroyQueue(Queue* q) {
    if (!q) return;
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
    Queue* queue = createQueue();

    // Test Case 1: Enqueue elements
    printf("Test Case 1: Enqueue 10, 20, 30\n");
    enqueue(queue, 10);
    enqueue(queue, 20);
    enqueue(queue, 30);
    printf("Enqueue successful.\n");
    printf("--------------------\n");

    // Test Case 2: Peek at the front element
    printf("Test Case 2: Peek front element\n");
    int front_val = peek(queue);
    if (front_val != INT_MIN) {
        printf("Front element is: %d\n", front_val); // Expected: 10
    }
    printf("--------------------\n");

    // Test Case 3: Dequeue an element
    printf("Test Case 3: Dequeue an element\n");
    int dequeued_val = dequeue(queue);
    if (dequeued_val != INT_MIN) {
        printf("Dequeued element: %d\n", dequeued_val); // Expected: 10
    }
    front_val = peek(queue);
    if (front_val != INT_MIN) {
        printf("Front element after dequeue is: %d\n", front_val); // Expected: 20
    }
    printf("--------------------\n");

    // Test Case 4: Dequeue all elements
    printf("Test Case 4: Dequeue all elements\n");
    dequeued_val = dequeue(queue);
    if (dequeued_val != INT_MIN) printf("Dequeued: %d\n", dequeued_val); // Expected: 20
    dequeued_val = dequeue(queue);
    if (dequeued_val != INT_MIN) printf("Dequeued: %d\n", dequeued_val); // Expected: 30
    printf("Is queue empty? %s\n", isEmpty(queue) ? "true" : "false"); // Expected: true
    printf("--------------------\n");

    // Test Case 5: Operations on an empty queue
    printf("Test Case 5: Operations on empty queue\n");
    printf("Attempting to peek...\n");
    peek(queue); // Will print error to stderr
    printf("Attempting to dequeue...\n");
    dequeue(queue); // Will print error to stderr
    printf("--------------------\n");

    // Clean up memory
    destroyQueue(queue);

    return 0;
}