#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Node structure for the linked list
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Function to create a new node. Securely checks for malloc failure.
Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Failed to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

// Inserts a new node at the end of the list.
// Takes a double pointer to head to modify the head in case the list is empty.
void insertNode(Node** head_ref, int data) {
    Node* newNode = createNode(data);
    if (*head_ref == NULL) {
        *head_ref = newNode;
        return;
    }
    Node* last = *head_ref;
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = newNode;
}

// Deletes the first occurrence of a key in the list.
// Takes a double pointer to head to modify the head if necessary.
void deleteNode(Node** head_ref, int key) {
    Node* temp = *head_ref;
    Node* prev = NULL;

    if (temp != NULL && temp->data == key) {
        *head_ref = temp->next;
        free(temp);
        return;
    }

    while (temp != NULL && temp->data != key) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        return;
    }
    
    // prev is guaranteed to not be null here due to the head check
    prev->next = temp->next;
    free(temp);
}

// Searches for a key in the list.
bool searchNode(Node* head, int key) {
    Node* current = head;
    while (current != NULL) {
        if (current->data == key) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// Utility function to print the linked list.
void printList(Node* node) {
    if (node == NULL) {
        printf("List is empty.\n");
        return;
    }
    while (node != NULL) {
        printf("%d -> ", node->data);
        node = node->next;
    }
    printf("NULL\n");
}

// Utility function to free the entire list to prevent memory leaks.
void freeList(Node** head_ref) {
    Node* current = *head_ref;
    Node* nextNode;
    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }
    *head_ref = NULL;
}

// Main function with test cases.
int main() {
    Node* head = NULL;

    // Test Case 1: Insertion
    printf("--- Test Case 1: Insertion ---\n");
    insertNode(&head, 10);
    insertNode(&head, 20);
    insertNode(&head, 30);
    insertNode(&head, 40);
    printf("List after inserting 10, 20, 30, 40: ");
    printList(head);
    printf("\n");

    // Test Case 2: Search for an existing element
    printf("--- Test Case 2: Search (Existing) ---\n");
    printf("Searching for 30: %s\n", searchNode(head, 30) ? "Found" : "Not Found");
    printf("\n");

    // Test Case 3: Search for a non-existing element
    printf("--- Test Case 3: Search (Non-Existing) ---\n");
    printf("Searching for 99: %s\n", searchNode(head, 99) ? "Found" : "Not Found");
    printf("\n");

    // Test Case 4: Delete an element from the middle
    printf("--- Test Case 4: Deletion (Middle) ---\n");
    printf("List before deleting 20: ");
    printList(head);
    deleteNode(&head, 20);
    printf("List after deleting 20: ");
    printList(head);
    printf("\n");

    // Test Case 5: Delete the head element
    printf("--- Test Case 5: Deletion (Head) ---\n");
    printf("List before deleting 10: ");
    printList(head);
    deleteNode(&head, 10);
    printf("List after deleting 10: ");
    printList(head);
    
    // Clean up all allocated memory before exiting
    freeList(&head);
    printf("\nList freed to prevent memory leaks.\n");

    return 0;
}