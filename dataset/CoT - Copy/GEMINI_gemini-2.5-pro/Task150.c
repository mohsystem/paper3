#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node* next;
} Node;

/**
 * @brief Inserts a new node at the beginning of the list.
 * @param head The current head of the list.
 * @param data The data for the new node.
 * @return The new head of the list.
 */
Node* insert(Node* head, int data) {
    // Security: Allocate memory and check for allocation failure.
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Failed to allocate memory for new node");
        return head; 
    }
    newNode->data = data;
    newNode->next = head;
    return newNode;
}

/**
 * @brief Deletes the first occurrence of a node with the given key.
 * @param head The current head of the list.
 * @param key The data of the node to be deleted.
 * @return The new head of the list.
 */
Node* deleteNode(Node* head, int key) {
    // Security: Handle case where list is empty.
    if (head == NULL) {
        return NULL;
    }
    
    // Case 1: The head node itself holds the key.
    if (head->data == key) {
        Node* temp = head;
        head = head->next;
        free(temp);        // Security: Free old head to prevent memory leak.
        return head;
    }
    
    Node* current = head;
    // Case 2: The key is somewhere other than the head.
    // Traverse to find the node *before* the one to be deleted.
    // Security: Check current->next to prevent dereferencing a NULL pointer.
    while (current->next != NULL && current->next->data != key) {
        current = current->next;
    }
    
    // If the key was found (current->next is the node to delete).
    if (current->next != NULL) {
        Node* nodeToDelete = current->next;
        current->next = current->next->next; // Unlink the node.
        free(nodeToDelete); // Security: Free memory.
    }
    
    return head;
}

/**
 * @brief Searches for a node with the given key.
 * @param head The head of the list.
 * @param key The data to search for.
 * @return true if the key is found, false otherwise.
 */
bool search(Node* head, int key) {
    Node* current = head;
    // Security: The loop condition prevents dereferencing a NULL pointer.
    while (current != NULL) {
        if (current->data == key) {
            return true;
        }
        current = current->next;
    }
    return false;
}

/**
 * @brief Utility function to print the linked list.
 * @param head The head of the list.
 */
void printList(Node* head) {
    Node* current = head;
    while (current != NULL) {
        printf("%d -> ", current->data);
        current = current->next;
    }
    printf("NULL\n");
}

/**
 * @brief Security: Frees all nodes in the list to prevent memory leaks.
 * @param head The head of the list to be freed.
 */
void freeList(Node* head) {
    Node* current = head;
    Node* nextNode;
    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }
}

int main() {
    Node* head = NULL;

    // Test Case 1: Insertion
    printf("Test Case 1: Insertion\n");
    head = insert(head, 30);
    head = insert(head, 20);
    head = insert(head, 10);
    printf("List after insertions: ");
    printList(head); // Expected: 10 -> 20 -> 30 -> NULL
    printf("--------------------\n");

    // Test Case 2: Search for an existing element
    printf("Test Case 2: Search for existing element (20)\n");
    printf("Found: %s\n", search(head, 20) ? "true" : "false"); // Expected: true
    printf("--------------------\n");
    
    // Test Case 3: Search for a non-existent element
    printf("Test Case 3: Search for non-existent element (50)\n");
    printf("Found: %s\n", search(head, 50) ? "true" : "false"); // Expected: false
    printf("--------------------\n");
    
    // Test Case 4: Delete an element from the middle
    printf("Test Case 4: Delete middle element (20)\n");
    head = deleteNode(head, 20);
    printf("List after deleting 20: ");
    printList(head); // Expected: 10 -> 30 -> NULL
    printf("--------------------\n");
    
    // Test Case 5: Delete head, non-existent, and from empty list
    printf("Test Case 5: Complex Deletions\n");
    head = deleteNode(head, 10); // Delete head
    printf("List after deleting head (10): ");
    printList(head); // Expected: 30 -> NULL
    head = deleteNode(head, 100); // Delete non-existent
    printf("List after attempting to delete 100: ");
    printList(head); // Expected: 30 -> NULL
    head = deleteNode(head, 30); // Delete last element
    printf("List after deleting 30: ");
    printList(head); // Expected: NULL
    head = deleteNode(head, 5); // Delete from empty list
    printf("List after deleting from empty list: ");
    printList(head); // Expected: NULL
    printf("--------------------\n");
    
    // Security: Clean up any remaining memory before exiting.
    freeList(head);

    return 0;
}