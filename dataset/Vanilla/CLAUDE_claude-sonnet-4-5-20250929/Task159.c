
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* buffer;
    int bufferSize;
} Task159;

void initializeBuffer(Task159* task, int size, int* initialData, int dataSize) {
    if (task->buffer != NULL) {
        free(task->buffer);
    }
    
    task->bufferSize = size;
    task->buffer = (int*)malloc(size * sizeof(int));
    
    for (int i = 0; i < size; i++) {
        task->buffer[i] = 0;
    }
    
    if (initialData != NULL) {
        for (int i = 0; i < size && i < dataSize; i++) {
            task->buffer[i] = initialData[i];
        }
    }
}

int readFromBuffer(Task159* task, int index, int* result) {
    if (task->buffer == NULL) {
        printf("Error: Buffer not initialized\\n");
        return 0;
    }
    if (index < 0 || index >= task->bufferSize) {
        printf("Error: Index out of bounds\\n");
        return 0;
    }
    *result = task->buffer[index];
    return 1;
}

void deallocateBuffer(Task159* task) {
    if (task->buffer != NULL) {
        free(task->buffer);
        task->buffer = NULL;
    }
    task->bufferSize = 0;
}

int main() {
    // Test case 1: Basic read operation
    printf("Test Case 1: Basic read operation\\n");
    Task159 task1 = {NULL, 0};
    int data1[] = {10, 20, 30, 40, 50};
    initializeBuffer(&task1, 5, data1, 5);
    int result;
    if (readFromBuffer(&task1, 2, &result)) {
        printf("Read from index 2: %d\\n", result);
    }
    deallocateBuffer(&task1);
    printf("\\n");
    
    // Test case 2: Read from first index
    printf("Test Case 2: Read from first index\\n");
    Task159 task2 = {NULL, 0};
    int data2[] = {100, 200, 300};
    initializeBuffer(&task2, 3, data2, 3);
    if (readFromBuffer(&task2, 0, &result)) {
        printf("Read from index 0: %d\\n", result);
    }
    deallocateBuffer(&task2);
    printf("\\n");
    
    // Test case 3: Read from last index
    printf("Test Case 3: Read from last index\\n");
    Task159 task3 = {NULL, 0};
    int data3[] = {5, 15, 25, 35, 45, 55};
    initializeBuffer(&task3, 6, data3, 6);
    if (readFromBuffer(&task3, 5, &result)) {
        printf("Read from index 5: %d\\n", result);
    }
    deallocateBuffer(&task3);
    printf("\\n");
    
    // Test case 4: Invalid index (negative)
    printf("Test Case 4: Invalid index (negative)\\n");
    Task159 task4 = {NULL, 0};
    int data4[] = {1, 2, 3, 4};
    initializeBuffer(&task4, 4, data4, 4);
    if (!readFromBuffer(&task4, -1, &result)) {
        printf("Result: Failed to read\\n");
    }
    deallocateBuffer(&task4);
    printf("\\n");
    
    // Test case 5: Invalid index (out of bounds)
    printf("Test Case 5: Invalid index (out of bounds)\\n");
    Task159 task5 = {NULL, 0};
    int data5[] = {7, 14, 21, 28};
    initializeBuffer(&task5, 4, data5, 4);
    if (!readFromBuffer(&task5, 10, &result)) {
        printf("Result: Failed to read\\n");
    }
    deallocateBuffer(&task5);
    printf("\\n");
    
    return 0;
}
