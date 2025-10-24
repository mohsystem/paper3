
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Maximum capacity to prevent excessive memory usage
#define MAX_CAPACITY 1000000

// Queue structure definition
typedef struct {
    int* data;           // Dynamic array to store queue elements
    size_t capacity;     // Maximum capacity of the queue
    size_t frontIndex;   // Index of the front element
    size_t rearIndex;    // Index where next element will be inserted
    size_t count;        // Current number of elements
} Queue;

// Initialize queue with given capacity
// Security: Validate capacity, check malloc return, initialize memory
bool queue_init(Queue* queue, size_t initialCapacity) {
    // Validate queue pointer
    if (queue == NULL) {
        fprintf(stderr, "Invalid queue pointer\\n");
        return false;
    }
    
    // Security: Initialize all fields to safe values
    queue->data = NULL;
    queue->capacity = 0;
    queue->frontIndex = 0;
    queue->rearIndex = 0;
    queue->count = 0;
    
    // Validate capacity is within safe bounds
    if (initialCapacity == 0 || initialCapacity > MAX_CAPACITY) {
        fprintf(stderr, "Invalid capacity. Using default (10).\\n");
        initialCapacity = 10;
    }
    
    queue->capacity = initialCapacity;
    
    // Security: Check for integer overflow in allocation size
    if (queue->capacity > SIZE_MAX / sizeof(int)) {
        fprintf(stderr, "Capacity too large, would cause overflow\\n");
        queue->capacity = 0;
        return false;
    }
    
    // Security: Use calloc to allocate and initialize memory to zero
    queue->data = (int*)calloc(queue->capacity, sizeof(int));
    if (queue->data == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        queue->capacity = 0;
        return false;
    }
    
    return true;
}

// Free queue memory
// Security: Clear sensitive data, check pointer validity, prevent double-free
void queue_destroy(Queue* queue) {
    if (queue == NULL) {
        return;
    }
    
    if (queue->data != NULL) {
        // Security: Clear sensitive data before freeing
        memset(queue->data, 0, queue->capacity * sizeof(int));
        free(queue->data);
        queue->data = NULL; // Prevent double-free
    }
    
    // Reset all fields
    queue->capacity = 0;
    queue->frontIndex = 0;
    queue->rearIndex = 0;
    queue->count = 0;
}

// Enqueue: Add element to rear of queue
// Security: Validate all parameters, check bounds, prevent overflow
bool queue_enqueue(Queue* queue, int value) {
    // Validate queue pointer
    if (queue == NULL) {
        fprintf(stderr, "Invalid queue pointer\\n");
        return false;
    }
    
    // Check if queue was properly initialized
    if (queue->data == NULL) {
        fprintf(stderr, "Queue not initialized\\n");
        return false;
    }
    
    // Check if queue is full
    if (queue->count >= queue->capacity) {
        fprintf(stderr, "Queue is full\\n");
        return false;
    }
    
    // Security: Bounds check - ensure rearIndex is within valid range
    if (queue->rearIndex >= queue->capacity) {
        fprintf(stderr, "Invalid rear index\\n");
        return false;
    }
    
    // Insert element at rear
    queue->data[queue->rearIndex] = value;
    queue->rearIndex = (queue->rearIndex + 1) % queue->capacity; // Circular increment
    queue->count++;
    
    return true;
}

// Dequeue: Remove and return element from front of queue
// Security: Validate all parameters, check bounds, prevent underflow
bool queue_dequeue(Queue* queue, int* result) {
    // Validate parameters
    if (queue == NULL || result == NULL) {
        fprintf(stderr, "Invalid parameters\\n");
        return false;
    }
    
    // Check if queue was properly initialized
    if (queue->data == NULL) {
        fprintf(stderr, "Queue not initialized\\n");
        return false;
    }
    
    // Check if queue is empty
    if (queue->count == 0) {
        fprintf(stderr, "Queue is empty\\n");
        return false;
    }
    
    // Security: Bounds check - ensure frontIndex is within valid range
    if (queue->frontIndex >= queue->capacity) {
        fprintf(stderr, "Invalid front index\\n");
        return false;
    }
    
    // Remove element from front
    *result = queue->data[queue->frontIndex];
    queue->data[queue->frontIndex] = 0; // Clear the slot
    queue->frontIndex = (queue->frontIndex + 1) % queue->capacity; // Circular increment
    queue->count--;
    
    return true;
}

// Peek: Return element at front without removing it
// Security: Validate all parameters, check bounds, prevent underflow
bool queue_peek(const Queue* queue, int* result) {
    // Validate parameters
    if (queue == NULL || result == NULL) {
        fprintf(stderr, "Invalid parameters\\n");
        return false;
    }
    
    // Check if queue was properly initialized
    if (queue->data == NULL) {
        fprintf(stderr, "Queue not initialized\\n");
        return false;
    }
    
    // Check if queue is empty
    if (queue->count == 0) {
        fprintf(stderr, "Queue is empty\\n");
        return false;
    }
    
    // Security: Bounds check - ensure frontIndex is within valid range
    if (queue->frontIndex >= queue->capacity) {
        fprintf(stderr, "Invalid front index\\n");
        return false;
    }
    
    // Return front element without removing
    *result = queue->data[queue->frontIndex];
    return true;
}

// Check if queue is empty
bool queue_is_empty(const Queue* queue) {
    if (queue == NULL) {
        return true;
    }
    return queue->count == 0;
}

// Get current size of queue
size_t queue_size(const Queue* queue) {
    if (queue == NULL) {
        return 0;
    }
    return queue->count;
}

// Test cases
int main(void) {
    printf("=== Queue Implementation Test Cases ===\\n");
    
    // Test Case 1: Basic enqueue and dequeue operations
    printf("\\nTest Case 1: Basic operations\\n");
    Queue q1;
    if (queue_init(&q1, 5)) {
        queue_enqueue(&q1, 10);
        queue_enqueue(&q1, 20);
        queue_enqueue(&q1, 30);
        int value = 0;
        if (queue_dequeue(&q1, &value)) {
            printf("Dequeued: %d\\n", value); // Expected: 10
        }
        if (queue_peek(&q1, &value)) {
            printf("Peek: %d\\n", value); // Expected: 20
        }
        queue_destroy(&q1);
    }
    
    // Test Case 2: Queue overflow handling
    printf("\\nTest Case 2: Overflow handling\\n");
    Queue q2;
    if (queue_init(&q2, 3)) {
        queue_enqueue(&q2, 1);
        queue_enqueue(&q2, 2);
        queue_enqueue(&q2, 3);
        if (!queue_enqueue(&q2, 4)) { // Should fail - queue is full
            printf("Correctly prevented overflow\\n");
        }
        queue_destroy(&q2);
    }
    
    // Test Case 3: Queue underflow handling
    printf("\\nTest Case 3: Underflow handling\\n");
    Queue q3;
    if (queue_init(&q3, 3)) {
        int value = 0;
        if (!queue_dequeue(&q3, &value)) { // Should fail - queue is empty
            printf("Correctly prevented underflow on dequeue\\n");
        }
        if (!queue_peek(&q3, &value)) { // Should fail - queue is empty
            printf("Correctly prevented underflow on peek\\n");
        }
        queue_destroy(&q3);
    }
    
    // Test Case 4: Circular buffer behavior
    printf("\\nTest Case 4: Circular buffer\\n");
    Queue q4;
    if (queue_init(&q4, 3)) {
        int value = 0;
        queue_enqueue(&q4, 100);
        queue_enqueue(&q4, 200);
        queue_dequeue(&q4, &value);
        queue_enqueue(&q4, 300);
        queue_enqueue(&q4, 400);
        while (!queue_is_empty(&q4)) {
            if (queue_dequeue(&q4, &value)) {
                printf("Dequeued: %d\\n", value); // Expected: 200, 300, 400
            }
        }
        queue_destroy(&q4);
    }
    
    // Test Case 5: Multiple operations sequence
    printf("\\nTest Case 5: Multiple operations\\n");
    Queue q5;
    if (queue_init(&q5, 10)) {
        int value = 0;
        for (int i = 1; i <= 5; i++) {
            queue_enqueue(&q5, i * 10);
        }
        printf("Queue size: %zu\\n", queue_size(&q5)); // Expected: 5
        if (queue_peek(&q5, &value)) {
            printf("Front element: %d\\n", value); // Expected: 10
        }
        queue_dequeue(&q5, &value);
        queue_dequeue(&q5, &value);
        printf("After 2 dequeues, size: %zu\\n", queue_size(&q5)); // Expected: 3
        if (queue_peek(&q5, &value)) {
            printf("New front element: %d\\n", value); // Expected: 30
        }
        queue_destroy(&q5);
    }
    
    return 0;
}
