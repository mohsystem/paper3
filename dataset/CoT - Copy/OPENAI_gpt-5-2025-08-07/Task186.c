/* Chain-of-Through secure implementation
   1) Problem: Merge k sorted linked lists.
   2) Security: Check allocations, handle NULL, avoid buffer overruns.
   3) Secure coding: Implement bounded min-heap for ListNode* with size k.
   4) Review: Heap ops only touch valid indices; prints guarded.
   5) Final secure output with 5 tests. */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Definition for singly-linked list. */
struct ListNode {
    int val;
    struct ListNode* next;
};

/* Min-heap for ListNode* by node->val */
struct MinHeap {
    struct ListNode** data;
    size_t size;
    size_t cap;
};

static bool heap_init(struct MinHeap* h, size_t cap) {
    if (cap == 0) cap = 1;
    h->data = (struct ListNode**)malloc(sizeof(struct ListNode*) * cap);
    if (!h->data) return false;
    h->size = 0;
    h->cap = cap;
    return true;
}

static void heap_free(struct MinHeap* h) {
    if (h && h->data) {
        free(h->data);
        h->data = NULL;
        h->size = h->cap = 0;
    }
}

static void heap_swap(struct ListNode** a, struct ListNode** b) {
    struct ListNode* t = *a; *a = *b; *b = t;
}

static void heap_sift_up(struct MinHeap* h, size_t i) {
    while (i > 0) {
        size_t p = (i - 1) / 2;
        if (h->data[p]->val <= h->data[i]->val) break;
        heap_swap(&h->data[p], &h->data[i]);
        i = p;
    }
}

static void heap_sift_down(struct MinHeap* h, size_t i) {
    while (true) {
        size_t l = 2 * i + 1, r = 2 * i + 2, smallest = i;
        if (l < h->size && h->data[l]->val < h->data[smallest]->val) smallest = l;
        if (r < h->size && h->data[r]->val < h->data[smallest]->val) smallest = r;
        if (smallest == i) break;
        heap_swap(&h->data[i], &h->data[smallest]);
        i = smallest;
    }
}

static bool heap_push(struct MinHeap* h, struct ListNode* node) {
    if (h->size == h->cap) {
        size_t ncap = h->cap * 2;
        struct ListNode** nd = (struct ListNode**)realloc(h->data, sizeof(struct ListNode*) * ncap);
        if (!nd) return false;
        h->data = nd;
        h->cap = ncap;
    }
    h->data[h->size] = node;
    heap_sift_up(h, h->size);
    h->size++;
    return true;
}

static struct ListNode* heap_pop(struct MinHeap* h) {
    if (h->size == 0) return NULL;
    struct ListNode* top = h->data[0];
    h->data[0] = h->data[h->size - 1];
    h->size--;
    if (h->size > 0) heap_sift_down(h, 0);
    return top;
}

/* Merge k sorted lists */
struct ListNode* mergeKLists(struct ListNode** lists, int listsSize) {
    if (listsSize <= 0 || lists == NULL) return NULL;
    struct MinHeap h;
    if (!heap_init(&h, (size_t)listsSize)) {
        return NULL; /* allocation failure -> return NULL safely */
    }
    for (int i = 0; i < listsSize; ++i) {
        if (lists[i] != NULL) heap_push(&h, lists[i]);
    }
    struct ListNode dummy = {0, NULL};
    struct ListNode* tail = &dummy;
    while (h.size > 0) {
        struct ListNode* cur = heap_pop(&h);
        tail->next = cur;
        tail = cur;
        if (cur->next) heap_push(&h, cur->next);
    }
    heap_free(&h);
    return dummy.next;
}

/* Utilities */
struct ListNode* buildList(const int* arr, size_t len) {
    if (arr == NULL || len == 0) return NULL;
    struct ListNode dummy = {0, NULL};
    struct ListNode* t = &dummy;
    for (size_t i = 0; i < len; ++i) {
        struct ListNode* node = (struct ListNode*)malloc(sizeof(struct ListNode));
        if (!node) {
            // On allocation failure, free built nodes and return NULL
            struct ListNode* cur = dummy.next;
            while (cur) { struct ListNode* n = cur->next; free(cur); cur = n; }
            return NULL;
        }
        node->val = arr[i];
        node->next = NULL;
        t->next = node;
        t = node;
    }
    return dummy.next;
}

void printList(struct ListNode* head) {
    printf("[");
    int first = 1;
    for (struct ListNode* cur = head; cur; cur = cur->next) {
        if (!first) printf(",");
        printf("%d", cur->val);
        first = 0;
    }
    printf("]\n");
}

void freeList(struct ListNode* head) {
    while (head) {
        struct ListNode* n = head->next;
        free(head);
        head = n;
    }
}

int main(void) {
    // Test case 1: [[1,4,5],[1,3,4],[2,6]]
    {
        int a1[] = {1,4,5};
        int a2[] = {1,3,4};
        int a3[] = {2,6};
        struct ListNode* lists1[3];
        lists1[0] = buildList(a1, 3);
        lists1[1] = buildList(a2, 3);
        lists1[2] = buildList(a3, 2);
        struct ListNode* merged1 = mergeKLists(lists1, 3);
        printList(merged1);
        freeList(merged1);
    }
    // Test case 2: []
    {
        struct ListNode** lists2 = NULL;
        struct ListNode* merged2 = mergeKLists(lists2, 0);
        printList(merged2);
        freeList(merged2);
    }
    // Test case 3: [[]]
    {
        struct ListNode* lists3[1];
        lists3[0] = NULL;
        struct ListNode* merged3 = mergeKLists(lists3, 1);
        printList(merged3);
        freeList(merged3);
    }
    // Test case 4: [[-10,-5,0,3],[1,2,2],[]]
    {
        int b1[] = {-10,-5,0,3};
        int b2[] = {1,2,2};
        struct ListNode* lists4[3];
        lists4[0] = buildList(b1, 4);
        lists4[1] = buildList(b2, 3);
        lists4[2] = NULL;
        struct ListNode* merged4 = mergeKLists(lists4, 3);
        printList(merged4);
        freeList(merged4);
    }
    // Test case 5: [[1],[0],[5],[3],[3],[4]]
    {
        int c1[] = {1}, c2[] = {0}, c3[] = {5}, c4[] = {3}, c5[] = {3}, c6[] = {4};
        struct ListNode* lists5[6];
        lists5[0] = buildList(c1, 1);
        lists5[1] = buildList(c2, 1);
        lists5[2] = buildList(c3, 1);
        lists5[3] = buildList(c4, 1);
        lists5[4] = buildList(c5, 1);
        lists5[5] = buildList(c6, 1);
        struct ListNode* merged5 = mergeKLists(lists5, 6);
        printList(merged5);
        freeList(merged5);
    }
    return 0;
}