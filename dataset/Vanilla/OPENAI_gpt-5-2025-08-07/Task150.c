#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    int val;
    struct Node* next;
} Node;

typedef struct LinkedList {
    Node* head;
} LinkedList;

void init_list(LinkedList* list) {
    list->head = NULL;
}

// Insert at end
void insert_value(LinkedList* list, int value) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return;
    n->val = value;
    n->next = NULL;
    if (list->head == NULL) {
        list->head = n;
        return;
    }
    Node* cur = list->head;
    while (cur->next) cur = cur->next;
    cur->next = n;
}

// Delete first occurrence; return 1 if deleted, 0 otherwise
int delete_value(LinkedList* list, int value) {
    Node* cur = list->head;
    Node* prev = NULL;
    while (cur) {
        if (cur->val == value) {
            if (prev == NULL) {
                list->head = cur->next;
            } else {
                prev->next = cur->next;
            }
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

// Search for value; return 1 if found, 0 otherwise
int search_value(LinkedList* list, int value) {
    Node* cur = list->head;
    while (cur) {
        if (cur->val == value) return 1;
        cur = cur->next;
    }
    return 0;
}

// Utility: convert list to string "[a, b, c]"; caller must free
char* list_to_string(const LinkedList* list) {
    // First count nodes
    int count = 0;
    const Node* cur = list->head;
    while (cur) { count++; cur = cur->next; }
    // Allocate approximate capacity: 16 chars per int + 2 for ", " + brackets + null
    int cap = count * 18 + 3;
    if (cap < 3) cap = 3;
    char* s = (char*)malloc(cap);
    if (!s) return NULL;
    int pos = 0;
    s[pos++] = '[';
    s[pos] = '\0';
    cur = list->head;
    while (cur) {
        int written = snprintf(s + pos, cap - pos, "%d", cur->val);
        if (written < 0) { s[0] = '\0'; return s; }
        pos += written;
        if (cur->next) {
            if (pos + 2 >= cap) { // grow if needed
                cap *= 2;
                char* ns = (char*)realloc(s, cap);
                if (!ns) { free(s); return NULL; }
                s = ns;
            }
            s[pos++] = ',';
            s[pos++] = ' ';
            s[pos] = '\0';
        }
        cur = cur->next;
    }
    if (pos + 2 >= cap) {
        cap += 2;
        char* ns = (char*)realloc(s, cap);
        if (!ns) { free(s); return NULL; }
        s = ns;
    }
    s[pos++] = ']';
    s[pos] = '\0';
    return s;
}

void free_list(LinkedList* list) {
    Node* cur = list->head;
    while (cur) {
        Node* nxt = cur->next;
        free(cur);
        cur = nxt;
    }
    list->head = NULL;
}

int main() {
    // Test 1: Insert sequence
    LinkedList list;
    init_list(&list);
    insert_value(&list, 10);
    insert_value(&list, 20);
    insert_value(&list, 30);
    char* s1 = list_to_string(&list);
    printf("Test1 list after inserts: %s\n", s1 ? s1 : "[]");
    free(s1);

    // Test 2: Search existing
    printf("Test2 search 20: %d\n", search_value(&list, 20));

    // Test 3: Delete middle
    int del20 = delete_value(&list, 20);
    char* s3 = list_to_string(&list);
    printf("Test3 delete 20: %d, list: %s\n", del20, s3 ? s3 : "[]");
    free(s3);

    // Test 4: Delete head and tail
    int del10 = delete_value(&list, 10);
    int del30 = delete_value(&list, 30);
    char* s4 = list_to_string(&list);
    printf("Test4 delete 10: %d, delete 30: %d, list: %s\n", del10, del30, s4 ? s4 : "[]");
    free(s4);

    // Test 5: Delete on empty and search non-existing
    printf("Test5 delete 99 on empty: %d, search 99: %d\n",
           delete_value(&list, 99), search_value(&list, 99));

    free_list(&list);
    return 0;
}