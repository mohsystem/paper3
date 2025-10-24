#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct Node {
    int value;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    size_t size;
} SinglyLinkedList;

typedef struct {
    int ok;
    int value;
} RemoveResult;

static int list_init(SinglyLinkedList* list) {
    if (list == NULL) return 0;
    list->head = NULL;
    list->size = 0;
    return 1;
}

static void list_destroy(SinglyLinkedList* list) {
    if (list == NULL) return;
    Node* cur = list->head;
    while (cur) {
        Node* nxt = cur->next;
        free(cur);
        cur = nxt;
    }
    list->head = NULL;
    list->size = 0;
}

static int list_insert_head(SinglyLinkedList* list, int value) {
    if (list == NULL) return 0;
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return 0;
    n->value = value;
    n->next = list->head;
    list->head = n;
    list->size++;
    return 1;
}

static int list_insert_tail(SinglyLinkedList* list, int value) {
    if (list == NULL) return 0;
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return 0;
    n->value = value;
    n->next = NULL;
    if (list->head == NULL) {
        list->head = n;
        list->size = 1;
        return 1;
    }
    Node* cur = list->head;
    while (cur->next) cur = cur->next;
    cur->next = n;
    list->size++;
    return 1;
}

static int list_insert_index(SinglyLinkedList* list, size_t index, int value) {
    if (list == NULL) return 0;
    if (index > list->size) return 0;
    if (index == 0) return list_insert_head(list, value);
    Node* prev = list->head;
    for (size_t i = 0; i < index - 1; i++) {
        prev = prev->next;
    }
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return 0;
    n->value = value;
    n->next = prev->next;
    prev->next = n;
    list->size++;
    return 1;
}

static int list_delete_value(SinglyLinkedList* list, int value, int* removedFlag) {
    if (list == NULL || removedFlag == NULL) return 0;
    Node* prev = NULL;
    Node* cur = list->head;
    while (cur && cur->value != value) {
        prev = cur;
        cur = cur->next;
    }
    if (!cur) {
        *removedFlag = 0;
        return 1;
    }
    if (!prev) list->head = cur->next;
    else prev->next = cur->next;
    free(cur);
    list->size--;
    *removedFlag = 1;
    return 1;
}

static RemoveResult list_delete_index(SinglyLinkedList* list, size_t index) {
    RemoveResult res;
    res.ok = 0;
    res.value = 0;
    if (list == NULL) return res;
    if (index >= list->size) return res;
    Node* prev = NULL;
    Node* cur = list->head;
    for (size_t i = 0; i < index; i++) {
        prev = cur;
        cur = cur->next;
    }
    if (!cur) return res;
    int val = cur->value;
    if (!prev) list->head = cur->next;
    else prev->next = cur->next;
    free(cur);
    list->size--;
    res.ok = 1;
    res.value = val;
    return res;
}

static int list_search(const SinglyLinkedList* list, int value) {
    if (list == NULL) return -1;
    const Node* cur = list->head;
    int idx = 0;
    while (cur) {
        if (cur->value == value) return idx;
        cur = cur->next;
        idx++;
    }
    return -1;
}

static size_t int_len_safe(int v) {
    long long t = (long long)v;
    size_t digits = 0;
    if (t == 0) return 1;
    if (t < 0) {
        digits++; // for '-'
        t = -t;
    }
    while (t > 0) {
        digits++;
        t /= 10;
    }
    return digits;
}

static char* list_to_string(const SinglyLinkedList* list) {
    if (list == NULL) {
        char* s = (char*)malloc(3);
        if (!s) return NULL;
        strcpy(s, "[]");
        return s;
    }
    if (list->size == 0) {
        char* s = (char*)malloc(3);
        if (!s) return NULL;
        strcpy(s, "[]");
        return s;
    }
    // First pass: compute required length
    size_t total = 2; // '[' and ']'
    size_t commas = (list->size > 0) ? (list->size - 1) : 0;
    total += commas; // commas
    const Node* cur = list->head;
    while (cur) {
        total += int_len_safe(cur->value);
        cur = cur->next;
    }
    total += 1; // null terminator

    char* buf = (char*)malloc(total);
    if (!buf) return NULL;
    size_t pos = 0;
    buf[pos++] = '[';

    cur = list->head;
    size_t remaining = total - pos;
    while (cur) {
        int written = snprintf(buf + pos, remaining, "%d", cur->value);
        if (written < 0 || (size_t)written >= remaining) { // safety check
            free(buf);
            return NULL;
        }
        pos += (size_t)written;
        remaining = total - pos;
        cur = cur->next;
        if (cur) {
            if (remaining < 2) { // need at least room for ',' and closing
                free(buf);
                return NULL;
            }
            buf[pos++] = ',';
            remaining = total - pos;
        }
    }
    if (remaining < 2) { // need space for ']' and '\0'
        free(buf);
        return NULL;
    }
    buf[pos++] = ']';
    buf[pos] = '\0';
    return buf;
}

int main(void) {
    SinglyLinkedList list;
    if (!list_init(&list)) {
        printf("Initialization failed\n");
        return 1;
    }

    // Test 1: Insert at tail and basic search/delete
    list_insert_tail(&list, 1);
    list_insert_tail(&list, 2);
    list_insert_tail(&list, 3);
    char* s1 = list_to_string(&list);
    printf("Test1 list: %s\n", s1 ? s1 : "[]");
    free(s1);
    printf("Test1 search(2): %d\n", list_search(&list, 2));
    int removedFlag = 0;
    if (!list_delete_value(&list, 2, &removedFlag)) {
        printf("Test1 delete_value(2): error\n");
    } else {
        char* s2 = list_to_string(&list);
        printf("Test1 delete_value(2): %s => %s search(2): %d\n",
               removedFlag ? "true" : "false",
               s2 ? s2 : "[]",
               list_search(&list, 2));
        free(s2);
    }

    // Test 2: Insert at head
    int headOk = list_insert_head(&list, 10);
    char* s3 = list_to_string(&list);
    printf("Test2 insert_at_head(10): %s => %s\n", headOk ? "true" : "false", s3 ? s3 : "[]");
    free(s3);

    // Test 3: Insert at index valid and invalid
    int insIdxOk = list_insert_index(&list, 1, 99);
    char* s4 = list_to_string(&list);
    printf("Test3 insert_at_index(1,99): %s => %s\n", insIdxOk ? "true" : "false", s4 ? s4 : "[]");
    free(s4);
    int insIdxBad = list_insert_index(&list, list.size + 2, 5);
    char* s5 = list_to_string(&list);
    printf("Test3 insert_at_index(size+2,5): %s => %s\n", insIdxBad ? "true" : "false", s5 ? s5 : "[]");
    free(s5);

    // Test 4: Delete at index valid and invalid
    RemoveResult rem0 = list_delete_index(&list, 0);
    char* s6 = list_to_string(&list);
    if (rem0.ok) {
        printf("Test4 delete_at_index(0): %d => %s\n", rem0.value, s6 ? s6 : "[]");
    } else {
        printf("Test4 delete_at_index(0): null => %s\n", s6 ? s6 : "[]");
    }
    free(s6);
    RemoveResult remBad = list_delete_index(&list, list.size); // out of bounds
    char* s7 = list_to_string(&list);
    if (remBad.ok) {
        printf("Test4 delete_at_index(size): %d => %s\n", remBad.value, s7 ? s7 : "[]");
    } else {
        printf("Test4 delete_at_index(size): null => %s\n", s7 ? s7 : "[]");
    }
    free(s7);

    // Test 5: Search non-existing
    printf("Test5 search(100): %d\n", list_search(&list, 100));

    list_destroy(&list);
    return 0;
}