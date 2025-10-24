#include <stdio.h>
#include <stdlib.h>

struct Node {
    int data;
    struct Node* next;
};

// Function to insert a new node at the end
void insert(struct Node** head_ref, int new_data) {
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
    if (new_node == NULL) {
        perror("Unable to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    struct Node* last = *head_ref;
    new_node->data = new_data;
    new_node->next = NULL;

    if (*head_ref == NULL) {
        *head_ref = new_node;
        return;
    }
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = new_node;
}

// Function to delete a node with a given key
void deleteNode(struct Node** head_ref, int key) {
    struct Node *temp = *head_ref, *prev = NULL;

    if (temp != NULL && temp->data == key) {
        *head_ref = temp->next;
        free(temp);
        return;
    }
    
    while (temp != NULL && temp->data != key) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;

    prev->next = temp->next;
    free(temp);
}

// Function to search for a node with a given key
// Returns 1 if found, 0 otherwise
int search(struct Node* head, int key) {
    struct Node* current = head;
    while (current != NULL) {
        if (current->data == key) {
            return 1;
        }
        current = current->next;
    }
    return 0;
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
    insert(&head, 10);
    insert(&head, 20);
    insert(&head, 30);
    insert(&head, 40);
    insert(&head, 50);
    printf("Created Linked list: ");
    printList(head);
    printf("\n");

    // Test Case 2: Search for an existing element
    printf("--- Test Case 2: Search (Existing) ---\n");
    printf("Searching for 30...\n");
    if (search(head, 30)) {
        printf("Element 30 found in the list.\n");
    } else {
        printf("Element 30 not found in the list.\n");
    }
    printf("\n");

    // Test Case 3: Search for a non-existing element
    printf("--- Test Case 3: Search (Not Existing) ---\n");
    printf("Searching for 100...\n");
    if (search(head, 100)) {
        printf("Element 100 found in the list.\n");
    } else {
        printf("Element 100 not found in the list.\n");
    }
    printf("\n");

    // Test Case 4: Deletion of a middle element
    printf("--- Test Case 4: Deletion (Middle) ---\n");
    printf("Deleting element 30...\n");
    deleteNode(&head, 30);
    printf("List after deleting 30: ");
    printList(head);
    printf("\n");

    // Test Case 5: Deletion of head and tail elements
    printf("--- Test Case 5: Deletion (Head and Tail) ---\n");
    printf("Deleting element 10 (head)...\n");
    deleteNode(&head, 10);
    printf("List after deleting 10: ");
    printList(head);

    printf("Deleting element 50 (tail)...\n");
    deleteNode(&head, 50);
    printf("List after deleting 50: ");
    printList(head);

    // Free the remaining list to prevent memory leaks
    freeList(&head);

    return 0;
}