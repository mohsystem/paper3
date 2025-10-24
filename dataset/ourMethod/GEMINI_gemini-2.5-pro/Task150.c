#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// A linked list node
struct Node {
    int data;
    struct Node* next;
};

// Function to insert a new node at the beginning of the list
void insertNode(struct Node** head_ref, int new_data) {
    // 1. allocate node
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
    if (new_node == NULL) {
        // Handle memory allocation failure
        perror("Failed to allocate memory for new node");
        return;
    }

    // 2. put in the data
    new_node->data = new_data;

    // 3. Make next of new node as head
    new_node->next = (*head_ref);

    // 4. move the head to point to the new node
    (*head_ref) = new_node;
}

// Function to delete the first occurrence of key in linked list
void deleteNode(struct Node** head_ref, int key) {
    // Store head node
    struct Node *temp = *head_ref, *prev = NULL;

    // If head node itself holds the key to be deleted
    if (temp != NULL && temp->data == key) {
        *head_ref = temp->next; // Changed head
        free(temp);             // free old head
        return;
    }

    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && temp->data != key) {
        prev = temp;
        temp = temp->next;
    }

    // If key was not present in linked list
    if (temp == NULL) return;

    // Unlink the node from linked list
    if (prev != NULL) {
        prev->next = temp->next;
    }

    free(temp); // Free memory
}

// Function to search for a key in the list
bool searchNode(struct Node* head, int key) {
    struct Node* current = head;
    while (current != NULL) {
        if (current->data == key) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// Function to print the linked list
void printList(struct Node* node) {
    while (node != NULL) {
        printf("%d -> ", node->data);
        node = node->next;
    }
    printf("NULL\n");
}

// Function to free the entire list
void freeList(struct Node** head_ref) {
    struct Node* current = *head_ref;
    struct Node* next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    *head_ref = NULL;
}

int main() {
    struct Node* head = NULL;

    // Test Case 1: Insertion
    printf("--- Test Case 1: Insertion ---\n");
    insertNode(&head, 3);
    insertNode(&head, 2);
    insertNode(&head, 1);
    printf("Created list: ");
    printList(head);
    printf("\n");

    // Test Case 2: Search for an existing element
    printf("--- Test Case 2: Search (existing) ---\n");
    int keyToSearch1 = 2;
    printf("Searching for %d: %s\n", keyToSearch1, searchNode(head, keyToSearch1) ? "Found" : "Not Found");
    printf("\n");

    // Test Case 3: Search for a non-existing element
    printf("--- Test Case 3: Search (non-existing) ---\n");
    int keyToSearch2 = 4;
    printf("Searching for %d: %s\n", keyToSearch2, searchNode(head, keyToSearch2) ? "Found" : "Not Found");
    printf("\n");

    // Test Case 4: Delete an element from the middle
    printf("--- Test Case 4: Deletion (middle) ---\n");
    int keyToDelete1 = 2;
    printf("Deleting %d\n", keyToDelete1);
    deleteNode(&head, keyToDelete1);
    printf("List after deletion: ");
    printList(head);
    printf("\n");

    // Test Case 5: Delete the head element
    printf("--- Test Case 5: Deletion (head) ---\n");
    int keyToDelete2 = 1;
    printf("Deleting %d\n", keyToDelete2);
    deleteNode(&head, keyToDelete2);
    printf("List after deletion: ");
    printList(head);
    
    // Clean up the remaining list
    freeList(&head);

    return 0;
}