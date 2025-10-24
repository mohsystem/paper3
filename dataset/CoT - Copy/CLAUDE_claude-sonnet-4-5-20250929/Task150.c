
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
        fprintf(stderr, "Memory allocation failed\\n");
        exit(1);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

LinkedList* createLinkedList() {
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    if (list == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        exit(1);
    }
    list->head = NULL;
    return list;
}

void insert(LinkedList* list, int data) {
    if (list == NULL) return;
    
    Node* newNode = createNode(data);
    newNode->next = list->head;
    list->head = newNode;
}

void insertAtEnd(LinkedList* list, int data) {
    if (list == NULL) return;
    
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
    if (list == NULL || list->head == NULL) {
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
    if (list == NULL) return false;
    
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
    if (list == NULL || list->head == NULL) {
        printf("List is empty\\n");
        return;
    }
    
    Node* current = list->head;
    while (current != NULL) {
        printf("%d -> ", current->data);
        current = current->next;
    }
    printf("NULL\\n");
}

void freeList(LinkedList* list) {
    if (list == NULL) return;
    
    Node* current = list->head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
    free(list);
}

int main() {
    printf("Test Case 1: Insert at beginning and display\\n");
    LinkedList* list1 = createLinkedList();
    insert(list1, 10);
    insert(list1, 20);
    insert(list1, 30);
    display(list1);
    freeList(list1);
    
    printf("\\nTest Case 2: Search operations\\n");
    LinkedList* list2 = createLinkedList();
    insert(list2, 5);
    insert(list2, 15);
    insert(list2, 25);
    printf("Search 15: %s\\n", search(list2, 15) ? "true" : "false");
    printf("Search 100: %s\\n", search(list2, 100) ? "true" : "false");
    freeList(list2);
    
    printf("\\nTest Case 3: Delete operations\\n");
    LinkedList* list3 = createLinkedList();
    insert(list3, 1);
    insert(list3, 2);
    insert(list3, 3);
    insert(list3, 4);
    printf("Before delete:\\n");
    display(list3);
    deleteNode(list3, 3);
    printf("After deleting 3:\\n");
    display(list3);
    freeList(list3);
    
    printf("\\nTest Case 4: Insert at end\\n");
    LinkedList* list4 = createLinkedList();
    insertAtEnd(list4, 100);
    insertAtEnd(list4, 200);
    insertAtEnd(list4, 300);
    display(list4);
    freeList(list4);
    
    printf("\\nTest Case 5: Operations on empty list\\n");
    LinkedList* list5 = createLinkedList();
    printf("Search in empty list: %s\\n", search(list5, 10) ? "true" : "false");
    printf("Delete from empty list: %s\\n", deleteNode(list5, 10) ? "true" : "false");
    display(list5);
    freeList(list5);
    
    return 0;
}
