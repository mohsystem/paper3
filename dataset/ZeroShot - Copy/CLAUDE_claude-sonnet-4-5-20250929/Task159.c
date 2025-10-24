
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define BUFFER_SIZE 100

typedef struct {
    int* buffer;
    int size;
} Task159;

Task159* create_task159() {
    Task159* task = (Task159*)malloc(sizeof(Task159));
    if (task == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    task->size = BUFFER_SIZE;
    task->buffer = (int*)malloc(BUFFER_SIZE * sizeof(int));
    
    if (task->buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        free(task);
        return NULL;
    }
    
    // Initialize buffer with sample data
    for (int i = 0; i < BUFFER_SIZE; i++) {
        task->buffer[i] = i * 10;
    }
    
    return task;
}

bool read_from_buffer(Task159* task, int index, int* value) {
    if (task == NULL || task->buffer == NULL) {
        fprintf(stderr, "Error: Invalid task object\\n");
        return false;
    }
    
    // Secure bounds checking
    if (index < 0 || index >= task->size) {
        printf("Error: Index out of bounds. Valid range: 0-%d\\n", task->size - 1);
        return false;
    }
    
    *value = task->buffer[index];
    return true;
}

int get_buffer_size(Task159* task) {
    if (task == NULL) {
        return 0;
    }
    return task->size;
}

void destroy_task159(Task159* task) {
    if (task != NULL) {
        if (task->buffer != NULL) {
            free(task->buffer);
        }
        free(task);
    }
}

int main() {
    Task159* task = create_task159();
    if (task == NULL) {
        return 1;
    }
    
    int value;
    
    printf("=== Test Cases ===\\n\\n");
    
    // Test case 1: Valid index at start
    printf("Test 1 - Valid index (0):\\n");
    if (read_from_buffer(task, 0, &value)) {
        printf("Value at index 0: %d\\n", value);
    }
    printf("\\n");
    
    // Test case 2: Valid index in middle
    printf("Test 2 - Valid index (50):\\n");
    if (read_from_buffer(task, 50, &value)) {
        printf("Value at index 50: %d\\n", value);
    }
    printf("\\n");
    
    // Test case 3: Valid index at end
    printf("Test 3 - Valid index (99):\\n");
    if (read_from_buffer(task, 99, &value)) {
        printf("Value at index 99: %d\\n", value);
    }
    printf("\\n");
    
    // Test case 4: Invalid negative index
    printf("Test 4 - Invalid negative index (-5):\\n");
    read_from_buffer(task, -5, &value);
    printf("\\n");
    
    // Test case 5: Invalid index beyond bounds
    printf("Test 5 - Invalid index beyond bounds (150):\\n");
    read_from_buffer(task, 150, &value);
    printf("\\n");
    
    destroy_task159(task);
    return 0;
}
