
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_LIST_SIZE 1000000  // Prevent excessive memory usage
#define MAX_STRING_LENGTH 256  // Maximum string length for data

// Node structure for singly linked list
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Singly linked list structure with size tracking
typedef struct {
    Node* head;
    size_t size;
} SinglyLinkedList;

// Initialize list - all pointers set to NULL, size to 0
// Security: Proper initialization prevents undefined behavior
void initList(SinglyLinkedList* list) {
    if (list == NULL) {
        return;
    }
    list->head = NULL;
    list->size = 0;
}

// Insert at beginning - O(1)
// Security: Validates list pointer, checks size limit, validates malloc return
bool insert(SinglyLinkedList* list, int value) {
    // Validate input pointer
    if (list == NULL) {
        fprintf(stderr, "Error: NULL list pointer\\n");
        return false;
    }
    
    // Prevent excessive memory usage
    if (list->size >= MAX_LIST_SIZE) {
        fprintf(stderr, "Error: List size limit reached\\n");
        return false;
    }
    
    // Allocate memory with validation
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return false;
    }
    
    // Initialize node - prevents use of uninitialized memory
    newNode->data = value;
    newNode->next = list->head;
    
    list->head = newNode;
    
    // Check for size overflow before incrementing
    if (list->size == SIZE_MAX) {
        fprintf(stderr, "Error: Size counter overflow\\n");
        free(newNode);
        list->head = list->head->next;
        return false;
    }
    list->size++;
    
    return true;
}

// Search for value - O(n)
// Security: Bounds checking with size validation to prevent infinite loops
bool search(const SinglyLinkedList* list, int value) {
    // Validate input pointer
    if (list == NULL) {
        fprintf(stderr, "Error: NULL list pointer\\n");
        return false;
    }
    
    if (list->size == 0 || list->head == NULL) {
        return false;
    }
    
    Node* current = list->head;
    size_t count = 0;
    
    // Iterate with bounds checking to prevent infinite loops
    while (current != NULL && count < list->size) {
        if (current->data == value) {
            return true;
        }
        current = current->next;
        count++;
        
        // Additional safety check for potential corruption
        if (count > MAX_LIST_SIZE) {
            fprintf(stderr, "Error: List traversal exceeded safe limit\\n");
            return false;
        }
    }
    
    return false;
}

// Delete first occurrence of value - O(n)
// Security: Safe pointer manipulation with NULL checks and proper memory freeing
bool deleteValue(SinglyLinkedList* list, int value) {
    // Validate input pointer
    if (list == NULL) {
        fprintf(stderr, "Error: NULL list pointer\\n");
        return false;
    }
    
    if (list->size == 0 || list->head == NULL) {
        return false;
    }
    
    // Check if head contains the value
    if (list->head->data == value) {
        Node* temp = list->head;
        list->head = list->head->next;
        
        // Clear sensitive data before freeing (if applicable)
        temp->data = 0;
        temp->next = NULL;
        free(temp);
        temp = NULL;  // Prevent use after free
        
        list->size--;
        return true;
    }
    
    Node* current = list->head;
    size_t count = 0;
    
    // Traverse with bounds checking
    while (current != NULL && current->next != NULL && count < list->size) {
        if (current->next->data == value) {
            Node* temp = current->next;
            current->next = current->next->next;
            
            // Clear and free memory safely
            temp->data = 0;
            temp->next = NULL;
            free(temp);
            temp = NULL;
            
            list->size--;
            return true;
        }
        current = current->next;
        count++;
        
        // Additional safety check
        if (count > MAX_LIST_SIZE) {
            fprintf(stderr, "Error: List traversal exceeded safe limit\\n");
            return false;
        }
    }
    
    return false;
}

// Display list contents
// Security: Bounds checking during traversal
void display(const SinglyLinkedList* list) {
    if (list == NULL) {
        fprintf(stderr, "Error: NULL list pointer\\n");
        return;
    }
    
    printf("List: ");
    Node* current = list->head;
    size_t count = 0;
    
    while (current != NULL && count < list->size) {
        printf("%d -> ", current->data);
        current = current->next;
        count++;
        
        // Safety check
        if (count > MAX_LIST_SIZE) {
            fprintf(stderr, "\\nError: Display traversal exceeded safe limit\\n");
            return;
        }
    }
    printf("NULL\\n");
}

// Free all memory - O(n)
// Security: Proper cleanup with NULL checks and pointer clearing
void freeList(SinglyLinkedList* list) {
    if (list == NULL) {
        return;
    }
    
    Node* current = list->head;
    size_t count = 0;
    
    while (current != NULL && count < MAX_LIST_SIZE) {
        Node* temp = current;
        current = current->next;
        
        // Clear data and free
        temp->data = 0;
        temp->next = NULL;
        free(temp);
        
        count++;
    }
    
    list->head = NULL;
    list->size = 0;
}

int main(void) {
    // Test case 1: Basic insert and search operations
    printf("Test Case 1: Basic operations\\n");
    SinglyLinkedList list1;
    initList(&list1);
    insert(&list1, 10);
    insert(&list1, 20);
    insert(&list1, 30);
    display(&list1);
    printf("Search 20: %s\\n", search(&list1, 20) ? "Found" : "Not Found");
    printf("Search 40: %s\\n", search(&list1, 40) ? "Found" : "Not Found");
    freeList(&list1);
    printf("\\n");
    
    // Test case 2: Delete operations
    printf("Test Case 2: Delete operations\\n");
    SinglyLinkedList list2;
    initList(&list2);
    insert(&list2, 5);
    insert(&list2, 15);
    insert(&list2, 25);
    insert(&list2, 35);
    display(&list2);
    printf("Delete 15: %s\\n", deleteValue(&list2, 15) ? "Success" : "Failed");
    display(&list2);
    printf("Delete 35: %s\\n", deleteValue(&list2, 35) ? "Success" : "Failed");
    display(&list2);
    freeList(&list2);
    printf("\\n");
    
    // Test case 3: Empty list operations
    printf("Test Case 3: Empty list operations\\n");
    SinglyLinkedList list3;
    initList(&list3);
    printf("Search in empty list: %s\\n", search(&list3, 10) ? "Found" : "Not Found");
    printf("Delete from empty list: %s\\n", deleteValue(&list3, 10) ? "Success" : "Failed");
    insert(&list3, 100);
    display(&list3);
    freeList(&list3);
    printf("\\n");
    
    // Test case 4: Multiple operations
    printf("Test Case 4: Multiple sequential operations\\n");
    SinglyLinkedList list4;
    initList(&list4);
    for (int i = 1; i <= 10; i++) {
        insert(&list4, i * 10);
    }
    display(&list4);
    printf("Size: %zu\\n", list4.size);
    deleteValue(&list4, 50);
    deleteValue(&list4, 10);
    deleteValue(&list4, 100);
    display(&list4);
    printf("Size after deletions: %zu\\n", list4.size);
    freeList(&list4);
    printf("\\n");
    
    // Test case 5: Boundary conditions
    printf("Test Case 5: Boundary conditions\\n");
    SinglyLinkedList list5;
    initList(&list5);
    insert(&list5, 1);
    printf("Single element: ");
    display(&list5);
    printf("Delete single element: %s\\n", deleteValue(&list5, 1) ? "Success" : "Failed");
    printf("After delete: ");
    display(&list5);
    printf("Size: %zu\\n", list5.size);
    freeList(&list5);
    
    return 0;
}
