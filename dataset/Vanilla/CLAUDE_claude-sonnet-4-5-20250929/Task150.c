
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
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

LinkedList* createLinkedList() {
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    list->head = NULL;
    return list;
}

void insert(LinkedList* list, int data) {
    Node* newNode = createNode(data);
    if (list->head == NULL) {
        list->head = newNode;
    } else {
        Node* current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
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
    printf("[");
    while (current != NULL) {
        printf("%d", current->data);
        if (current->next != NULL) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]\\n");
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
    // Test Case 1: Basic insert and search
    printf("Test Case 1: Basic insert and search\\n");
    LinkedList* list1 = createLinkedList();
    insert(list1, 10);
    insert(list1, 20);
    insert(list1, 30);
    display(list1);
    printf("Search 20: %s\\n", search(list1, 20) ? "true" : "false");
    printf("Search 40: %s\\n", search(list1, 40) ? "true" : "false");
    printf("\\n");
    freeList(list1);
    
    // Test Case 2: Delete from middle
    printf("Test Case 2: Delete from middle\\n");
    LinkedList* list2 = createLinkedList();
    insert(list2, 5);
    insert(list2, 15);
    insert(list2, 25);
    insert(list2, 35);
    printf("Before delete: ");
    display(list2);
    deleteNode(list2, 15);
    printf("After delete 15: ");
    display(list2);
    printf("\\n");
    freeList(list2);
    
    // Test Case 3: Delete head
    printf("Test Case 3: Delete head\\n");
    LinkedList* list3 = createLinkedList();
    insert(list3, 100);
    insert(list3, 200);
    insert(list3, 300);
    printf("Before delete: ");
    display(list3);
    deleteNode(list3, 100);
    printf("After delete head: ");
    display(list3);
    printf("\\n");
    freeList(list3);
    
    // Test Case 4: Delete non-existent element
    printf("Test Case 4: Delete non-existent element\\n");
    LinkedList* list4 = createLinkedList();
    insert(list4, 1);
    insert(list4, 2);
    insert(list4, 3);
    printf("List: ");
    display(list4);
    printf("Delete 99: %s\\n", deleteNode(list4, 99) ? "true" : "false");
    printf("\\n");
    freeList(list4);
    
    // Test Case 5: Operations on empty list
    printf("Test Case 5: Operations on empty list\\n");
    LinkedList* list5 = createLinkedList();
    printf("Search in empty list: %s\\n", search(list5, 10) ? "true" : "false");
    printf("Delete from empty list: %s\\n", deleteNode(list5, 10) ? "true" : "false");
    insert(list5, 50);
    printf("After insert: ");
    display(list5);
    freeList(list5);
    
    return 0;
}
