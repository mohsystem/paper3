
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct LinkedList {
    Node* head;
} LinkedList;

Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("Memory allocation failed\\n");
        exit(1);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

LinkedList* createList() {
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    if (list == NULL) {
        printf("Memory allocation failed\\n");
        exit(1);
    }
    list->head = NULL;
    return list;
}

void insert(LinkedList* list, int data) {
    Node* newNode = createNode(data);
    if (list->head == NULL) {
        list->head = newNode;
        return;
    }
    
    Node* current = list->head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
}

bool deleteNode(LinkedList* list, int data) {
    if (list->head == NULL) {
        return false;
    }
    
    if (list->head->data == data) {
        Node* temp = list->head;
        list->head = list->head->next;
        free(temp);
        return true;
    }
    
    Node* current = list->head;
    while (current->next != NULL) {
        if (current->next->data == data) {
            Node* temp = current->next;
            current->next = current->next->next;
            free(temp);
            return true;
        }
        current = current->next;
    }
    return false;
}

bool search(LinkedList* list, int data) {
    Node* current = list->head;
    while (current != NULL) {
        if (current->data == data) {
            return true;
        }
        current = current->next;
    }
    return false;
}

void display(LinkedList* list) {
    Node* current = list->head;
    while (current != NULL) {
        printf("%d -> ", current->data);
        current = current->next;
    }
    printf("NULL\\n");
}

void freeList(LinkedList* list) {
    Node* current = list->head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
    free(list);
}

int main() {
    // Test Case 1: Insert and display
    printf("Test Case 1: Insert elements\\n");
    LinkedList* list1 = createList();
    insert(list1, 10);
    insert(list1, 20);
    insert(list1, 30);
    display(list1);
    freeList(list1);
    
    // Test Case 2: Search operations
    printf("\\nTest Case 2: Search operations\\n");
    LinkedList* list2 = createList();
    insert(list2, 5);
    insert(list2, 15);
    insert(list2, 25);
    printf("Search 15: %s\\n", search(list2, 15) ? "true" : "false");
    printf("Search 100: %s\\n", search(list2, 100) ? "true" : "false");
    freeList(list2);
    
    // Test Case 3: Delete operations
    printf("\\nTest Case 3: Delete operations\\n");
    LinkedList* list3 = createList();
    insert(list3, 1);
    insert(list3, 2);
    insert(list3, 3);
    insert(list3, 4);
    printf("Before delete: ");
    display(list3);
    deleteNode(list3, 3);
    printf("After deleting 3: ");
    display(list3);
    freeList(list3);
    
    // Test Case 4: Delete head element
    printf("\\nTest Case 4: Delete head element\\n");
    LinkedList* list4 = createList();
    insert(list4, 100);
    insert(list4, 200);
    insert(list4, 300);
    printf("Before delete: ");
    display(list4);
    deleteNode(list4, 100);
    printf("After deleting head: ");
    display(list4);
    freeList(list4);
    
    // Test Case 5: Operations on empty list
    printf("\\nTest Case 5: Operations on empty list\\n");
    LinkedList* list5 = createList();
    printf("Delete from empty list: %s\\n", deleteNode(list5, 10) ? "true" : "false");
    printf("Search in empty list: %s\\n", search(list5, 10) ? "true" : "false");
    insert(list5, 50);
    printf("After inserting 50: ");
    display(list5);
    freeList(list5);
    
    return 0;
}
