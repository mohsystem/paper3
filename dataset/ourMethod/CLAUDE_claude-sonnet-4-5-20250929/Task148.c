
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Maximum stack size to prevent excessive memory usage
#define MAX_STACK_SIZE 1000000
#define INITIAL_CAPACITY 16

// Stack structure with bounds checking
typedef struct {
    int* data;           // Pointer to stack data, initialized to NULL
    size_t size;         // Current number of elements
    size_t capacity;     // Current allocated capacity
} Stack;

// Initialize stack with error checking
bool stack_init(Stack* stack) {
    // Validate input pointer is not NULL
    if (stack == NULL) {
        fprintf(stderr, "Error: NULL stack pointer\\n");
        return false;
    }

    // Initialize all fields
    stack->data = NULL;
    stack->size = 0;
    stack->capacity = 0;

    // Allocate initial memory with overflow check
    if (INITIAL_CAPACITY > SIZE_MAX / sizeof(int)) {
        fprintf(stderr, "Error: Initial capacity too large\\n");
        return false;
    }

    stack->data = (int*)calloc(INITIAL_CAPACITY, sizeof(int));
    if (stack->data == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return false;
    }

    stack->capacity = INITIAL_CAPACITY;
    return true;
}

// Push operation with overflow protection and bounds checking
bool stack_push(Stack* stack, int value) {
    // Validate stack pointer is not NULL
    if (stack == NULL) {
        fprintf(stderr, "Error: NULL stack pointer\\n");
        return false;
    }

    // Check if stack would exceed maximum size
    if (stack->size >= MAX_STACK_SIZE) {
        fprintf(stderr, "Error: Stack overflow - maximum size reached\\n");
        return false;
    }

    // Grow capacity if needed with overflow checks
    if (stack->size >= stack->capacity) {
        // Check for multiplication overflow
        if (stack->capacity > SIZE_MAX / 2 / sizeof(int)) {
            fprintf(stderr, "Error: Capacity overflow\\n");
            return false;
        }

        size_t new_capacity = stack->capacity * 2;
        // Enforce maximum size limit
        if (new_capacity > MAX_STACK_SIZE) {
            new_capacity = MAX_STACK_SIZE;
        }

        // Check for multiplication overflow in allocation
        if (new_capacity > SIZE_MAX / sizeof(int)) {
            fprintf(stderr, "Error: Allocation size overflow\\n");
            return false;
        }

        int* new_data = (int*)realloc(stack->data, new_capacity * sizeof(int));
        if (new_data == NULL) {
            fprintf(stderr, "Error: Memory reallocation failed\\n");
            return false;
        }

        stack->data = new_data;
        stack->capacity = new_capacity;
    }

    // Bounds check before write
    if (stack->size >= stack->capacity) {
        fprintf(stderr, "Error: Array bounds violation\\n");
        return false;
    }

    // Safe to write to buffer
    stack->data[stack->size] = value;
    stack->size++;
    return true;
}

// Pop operation with underflow protection
bool stack_pop(Stack* stack, int* value) {
    // Validate pointers are not NULL
    if (stack == NULL || value == NULL) {
        fprintf(stderr, "Error: NULL pointer\\n");
        return false;
    }

    // Check for underflow
    if (stack->size == 0) {
        fprintf(stderr, "Error: Stack underflow - cannot pop from empty stack\\n");
        return false;
    }

    // Bounds check before read
    if (stack->size > stack->capacity) {
        fprintf(stderr, "Error: Invalid stack state\\n");
        return false;
    }

    stack->size--;
    *value = stack->data[stack->size];
    return true;
}

// Peek operation with bounds checking
bool stack_peek(const Stack* stack, int* value) {
    // Validate pointers are not NULL
    if (stack == NULL || value == NULL) {
        fprintf(stderr, "Error: NULL pointer\\n");
        return false;
    }

    // Check for underflow
    if (stack->size == 0) {
        fprintf(stderr, "Error: Stack underflow - cannot peek empty stack\\n");
        return false;
    }

    // Bounds check before read
    if (stack->size > stack->capacity || stack->size == 0) {
        fprintf(stderr, "Error: Invalid stack state\\n");
        return false;
    }

    *value = stack->data[stack->size - 1];
    return true;
}

// Check if stack is empty
bool stack_is_empty(const Stack* stack) {
    // Validate pointer is not NULL
    if (stack == NULL) {
        return true; // Treat NULL as empty
    }
    return stack->size == 0;
}

// Get stack size
size_t stack_size(const Stack* stack) {
    // Validate pointer is not NULL
    if (stack == NULL) {
        return 0;
    }
    return stack->size;
}

// Free stack memory with secure cleanup
void stack_free(Stack* stack) {
    // Validate pointer is not NULL
    if (stack == NULL) {
        return;
    }

    if (stack->data != NULL) {
        // Clear sensitive data before freeing
        memset(stack->data, 0, stack->capacity * sizeof(int));
        free(stack->data);
        stack->data = NULL; // Prevent use-after-free
    }

    stack->size = 0;
    stack->capacity = 0;
}

int main(void) {
    Stack stack;
    int value = 0;

    // Test case 1: Basic push and pop operations
    printf("Test 1: Basic operations\\n");
    if (!stack_init(&stack)) {
        return 1;
    }
    stack_push(&stack, 10);
    stack_push(&stack, 20);
    stack_push(&stack, 30);
    if (stack_peek(&stack, &value)) {
        printf("Peek: %d\\n", value);
    }
    if (stack_pop(&stack, &value)) {
        printf("Pop: %d\\n", value);
    }
    if (stack_pop(&stack, &value)) {
        printf("Pop: %d\\n", value);
    }
    printf("Size: %zu\\n\\n", stack_size(&stack));
    stack_free(&stack);

    // Test case 2: Empty stack operations
    printf("Test 2: Empty stack check\\n");
    if (!stack_init(&stack)) {
        return 1;
    }
    printf("Is empty: %s\\n", stack_is_empty(&stack) ? "true" : "false");
    stack_push(&stack, 5);
    printf("Is empty after push: %s\\n", stack_is_empty(&stack) ? "true" : "false");
    stack_pop(&stack, &value);
    printf("Is empty after pop: %s\\n\\n", stack_is_empty(&stack) ? "true" : "false");
    stack_free(&stack);

    // Test case 3: Underflow protection
    printf("Test 3: Underflow protection\\n");
    if (!stack_init(&stack)) {
        return 1;
    }
    if (!stack_pop(&stack, &value)) {
        printf("Pop from empty stack correctly prevented\\n\\n");
    }
    stack_free(&stack);

    // Test case 4: Multiple pushes
    printf("Test 4: Multiple operations\\n");
    if (!stack_init(&stack)) {
        return 1;
    }
    for (int i = 1; i <= 5; i++) {
        stack_push(&stack, i * 10);
    }
    printf("Size after pushes: %zu\\n", stack_size(&stack));
    while (!stack_is_empty(&stack)) {
        if (stack_pop(&stack, &value)) {
            printf("Pop: %d\\n", value);
        }
    }
    printf("Final size: %zu\\n\\n", stack_size(&stack));
    stack_free(&stack);

    // Test case 5: Peek without pop
    printf("Test 5: Peek operations\\n");
    if (!stack_init(&stack)) {
        return 1;
    }
    stack_push(&stack, 100);
    stack_push(&stack, 200);
    if (stack_peek(&stack, &value)) {
        printf("First peek: %d\\n", value);
    }
    if (stack_peek(&stack, &value)) {
        printf("Second peek: %d\\n", value);
    }
    printf("Size unchanged: %zu\\n", stack_size(&stack));
    stack_free(&stack);

    return 0;
}
