/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Singly linked list with insert, delete, and search operations.
 * 2) Security requirements: Validate indices, avoid null dereferences, and manage memory correctly.
 * 3) Secure coding generation: Defensive checks and careful allocation/deallocation.
 * 4) Code review: Verified for memory leaks, pointer validity, and size coherence.
 * 5) Secure code output: Final safe C implementation with 5 tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

typedef struct Node {
    int val;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    int size;
} SinglyLinkedList;

static void init_list(SinglyLinkedList* list) {
    if (!list) return;
    list->head = NULL;
    list->size = 0;
}

static void free_list(SinglyLinkedList* list) {
    if (!list) return;
    Node* cur = list->head;
    while (cur) {
        Node* nxt = cur->next;
        free(cur);
        cur = nxt;
    }
    list->head = NULL;
    list->size = 0;
}

static void insert_at_head(SinglyLinkedList* list, int value) {
    if (!list) return;
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return; // allocation failure: no-op
    n->val = value;
    n->next = list->head;
    list->head = n;
    list->size++;
}

static void insert_at_tail(SinglyLinkedList* list, int value) {
    if (!list) return;
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return;
    n->val = value;
    n->next = NULL;
    if (!list->head) {
        list->head = n;
    } else {
        Node* cur = list->head;
        while (cur->next) cur = cur->next;
        cur->next = n;
    }
    list->size++;
}

static bool insert_at_index(SinglyLinkedList* list, int index, int value) {
    if (!list) return false;
    if (index < 0 || index > list->size) return false;
    if (index == 0) {
        insert_at_head(list, value);
        return true;
    }
    Node* prev = list->head;
    for (int i = 0; i < index - 1; i++) {
        if (!prev) return false;
        prev = prev->next;
    }
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return false;
    n->val = value;
    n->next = prev->next;
    prev->next = n;
    list->size++;
    return true;
}

static bool delete_by_value(SinglyLinkedList* list, int value) {
    if (!list) return false;
    Node* prev = NULL;
    Node* cur = list->head;
    while (cur && cur->val != value) {
        prev = cur;
        cur = cur->next;
    }
    if (!cur) return false;
    if (!prev) list->head = cur->next;
    else prev->next = cur->next;
    free(cur);
    list->size--;
    return true;
}

static bool delete_at_index(SinglyLinkedList* list, int index) {
    if (!list) return false;
    if (index < 0 || index >= list->size) return false;
    if (index == 0) {
        if (list->head) {
            Node* tmp = list->head;
            list->head = list->head->next;
            free(tmp);
            list->size--;
            return true;
        }
        return false;
    }
    Node* prev = list->head;
    for (int i = 0; i < index - 1; i++) {
        if (!prev) return false;
        prev = prev->next;
    }
    if (!prev || !prev->next) return false;
    Node* toDel = prev->next;
    prev->next = toDel->next;
    free(toDel);
    list->size--;
    return true;
}

static int index_of(const SinglyLinkedList* list, int value) {
    if (!list) return -1;
    int idx = 0;
    Node* cur = list->head;
    while (cur) {
        if (cur->val == value) return idx;
        cur = cur->next;
        idx++;
    }
    return -1;
}

static int count_int_chars(int v) {
    // count characters needed to print integer v
    int count = 0;
    if (v == 0) return 1;
    if (v == INT_MIN) return 11; // "-2147483648"
    if (v < 0) { count++; v = -v; }
    while (v > 0) {
        count++;
        v /= 10;
    }
    return count;
}

static char* list_to_string(const SinglyLinkedList* list) {
    if (!list) {
        char* s = (char*)malloc(3);
        if (!s) return NULL;
        strcpy(s, "[]");
        return s;
    }
    // Calculate length
    int len = 2; // for '[' and ']'
    int nodes = 0;
    Node* cur = list->head;
    while (cur) {
        len += count_int_chars(cur->val);
        nodes++;
        cur = cur->next;
    }
    if (nodes > 1) {
        len += (nodes - 1) * 4; // " -> "
    }
    len += 1; // null-terminator

    char* buf = (char*)malloc((size_t)len);
    if (!buf) return NULL;
    char* p = buf;
    *p++ = '[';

    cur = list->head;
    while (cur) {
        int written = snprintf(p, (size_t)(buf + len - p), "%d", cur->val);
        if (written < 0) { free(buf); return NULL; }
        p += written;
        cur = cur->next;
        if (cur) {
            memcpy(p, " -> ", 4);
            p += 4;
        }
    }
    *p++ = ']';
    *p = '\0';
    return buf;
}

int main(void) {
    // 5 Test cases
    SinglyLinkedList list;
    init_list(&list);

    // Test 1: Insert at head and tail
    insert_at_head(&list, 3);
    insert_at_tail(&list, 5);
    insert_at_tail(&list, 7);
    char* s1 = list_to_string(&list);
    printf("Test1: %s | size=%d\n", s1 ? s1 : "[]", list.size);
    free(s1);

    // Test 2: Insert at index
    bool insIdx = insert_at_index(&list, 1, 4); // [3,4,5,7]
    char* s2 = list_to_string(&list);
    printf("Test2: insert_at_index(1,4)=%s -> %s | size=%d\n", insIdx ? "true" : "false", s2 ? s2 : "[]", list.size);
    free(s2);

    // Test 3: Search
    printf("Test3: index_of(5)=%d, index_of(9)=%d\n", index_of(&list, 5), index_of(&list, 9));

    // Test 4: Delete by value
    bool delVal1 = delete_by_value(&list, 4);
    bool delVal2 = delete_by_value(&list, 42);
    char* s4 = list_to_string(&list);
    printf("Test4: delete_by_value(4)=%s, delete_by_value(42)=%s -> %s | size=%d\n",
           delVal1 ? "true" : "false", delVal2 ? "true" : "false", s4 ? s4 : "[]", list.size);
    free(s4);

    // Test 5: Delete at index
    bool delIdx = delete_at_index(&list, 1);
    char* s5 = list_to_string(&list);
    printf("Test5: delete_at_index(1)=%s -> %s | size=%d\n", delIdx ? "true" : "false", s5 ? s5 : "[]", list.size);
    free(s5);

    free_list(&list);
    return 0;
}