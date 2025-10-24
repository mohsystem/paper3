#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int val;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    size_t size;
} SinglyLinkedList;

void list_init(SinglyLinkedList* list) {
    if (list == NULL) return;
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

bool list_insert(SinglyLinkedList* list, int value) {
    if (list == NULL) return false;
    Node* n = (Node*)malloc(sizeof(Node));
    if (n == NULL) return false;
    n->val = value;
    n->next = NULL;
    if (list->head == NULL) {
        list->head = n;
        list->tail = n;
    } else {
        list->tail->next = n;
        list->tail = n;
    }
    list->size++;
    return true;
}

bool list_delete(SinglyLinkedList* list, int value) {
    if (list == NULL) return false;
    Node* prev = NULL;
    Node* curr = list->head;
    while (curr != NULL) {
        if (curr->val == value) {
            if (prev == NULL) {
                list->head = curr->next;
            } else {
                prev->next = curr->next;
            }
            if (curr == list->tail) {
                list->tail = prev;
            }
            free(curr);
            list->size--;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

bool list_search(const SinglyLinkedList* list, int value) {
    if (list == NULL) return false;
    const Node* curr = list->head;
    while (curr != NULL) {
        if (curr->val == value) return true;
        curr = curr->next;
    }
    return false;
}

void list_clear(SinglyLinkedList* list) {
    if (list == NULL) return;
    Node* curr = list->head;
    while (curr != NULL) {
        Node* next = curr->next;
        free(curr);
        curr = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void print_list(const SinglyLinkedList* list) {
    printf("[");
    const Node* curr = (list != NULL) ? list->head : NULL;
    int first = 1;
    while (curr != NULL) {
        if (!first) printf(", ");
        printf("%d", curr->val);
        first = 0;
        curr = curr->next;
    }
    printf("]\n");
}

int main(void) {
    // Test Case 1
    SinglyLinkedList l1;
    list_init(&l1);
    list_insert(&l1, 1);
    list_insert(&l1, 2);
    list_insert(&l1, 3);
    printf("TC1 search 2: %s\n", list_search(&l1, 2) ? "true" : "false");
    printf("TC1 delete 2: %s\n", list_delete(&l1, 2) ? "true" : "false");
    printf("TC1 search 2: %s\n", list_search(&l1, 2) ? "true" : "false");
    printf("TC1 list: "); print_list(&l1);
    list_clear(&l1);

    // Test Case 2: delete head
    SinglyLinkedList l2;
    list_init(&l2);
    list_insert(&l2, 10);
    list_insert(&l2, 20);
    printf("TC2 delete 10: %s\n", list_delete(&l2, 10) ? "true" : "false");
    printf("TC2 list: "); print_list(&l2);
    list_clear(&l2);

    // Test Case 3: delete tail
    SinglyLinkedList l3;
    list_init(&l3);
    list_insert(&l3, 7);
    list_insert(&l3, 8);
    list_insert(&l3, 9);
    printf("TC3 delete 9: %s\n", list_delete(&l3, 9) ? "true" : "false");
    printf("TC3 list: "); print_list(&l3);
    list_clear(&l3);

    // Test Case 4: delete non-existent
    SinglyLinkedList l4;
    list_init(&l4);
    list_insert(&l4, 5);
    printf("TC4 delete 6: %s\n", list_delete(&l4, 6) ? "true" : "false");
    printf("TC4 list: "); print_list(&l4);
    list_clear(&l4);

    // Test Case 5: search in empty
    SinglyLinkedList l5;
    list_init(&l5);
    printf("TC5 search 42: %s\n", list_search(&l5, 42) ? "true" : "false");
    printf("TC5 list: "); print_list(&l5);
    list_clear(&l5);

    return 0;
}