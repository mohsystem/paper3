#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

struct ListNode {
    int val;
    struct ListNode* next;
};

struct ListNode* mergeKLists(struct ListNode** lists, size_t k);

/* Helpers */
static struct ListNode* build_list(const int* arr, size_t n) {
    struct ListNode dummy = {0, NULL};
    struct ListNode* cur = &dummy;
    for (size_t i = 0; i < n; ++i) {
        struct ListNode* node = (struct ListNode*)malloc(sizeof(struct ListNode));
        if (!node) {
            // cleanup and return NULL
            struct ListNode* t = dummy.next;
            while (t) { struct ListNode* nx = t->next; free(t); t = nx; }
            return NULL;
        }
        node->val = arr[i];
        node->next = NULL;
        cur->next = node;
        cur = node;
    }
    return dummy.next;
}

static void free_list(struct ListNode* head) {
    while (head) {
        struct ListNode* n = head->next;
        free(head);
        head = n;
    }
}

static int* to_array(struct ListNode* head, size_t* out_len) {
    size_t cap = 16;
    size_t len = 0;
    int* arr = (int*)malloc(cap * sizeof(int));
    if (!arr) { *out_len = 0; return NULL; }
    while (head) {
        if (len == cap) {
            size_t ncap = cap * 2;
            int* narr = (int*)realloc(arr, ncap * sizeof(int));
            if (!narr) { free(arr); *out_len = 0; return NULL; }
            arr = narr; cap = ncap;
        }
        arr[len++] = head->val;
        head = head->next;
    }
    *out_len = len;
    return arr;
}

static void print_array(const int* arr, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(",");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

/* Min-heap of ListNode* based on node->val */
struct Heap {
    struct ListNode** data;
    size_t size;
    size_t cap;
};

static void heap_init(struct Heap* h, size_t cap) {
    if (cap == 0) cap = 1;
    h->data = (struct ListNode**)malloc(cap * sizeof(struct ListNode*));
    h->size = 0;
    h->cap = h->data ? cap : 0;
}

static void heap_free(struct Heap* h) {
    free(h->data);
    h->data = NULL;
    h->size = 0;
    h->cap = 0;
}

static void heap_swap(struct ListNode** a, struct ListNode** b) {
    struct ListNode* t = *a; *a = *b; *b = t;
}

static void heap_sift_up(struct Heap* h, size_t idx) {
    while (idx > 0) {
        size_t p = (idx - 1) / 2;
        if (h->data[p]->val <= h->data[idx]->val) break;
        heap_swap(&h->data[p], &h->data[idx]);
        idx = p;
    }
}

static void heap_sift_down(struct Heap* h, size_t idx) {
    while (1) {
        size_t l = idx * 2 + 1, r = idx * 2 + 2, m = idx;
        if (l < h->size && h->data[l]->val < h->data[m]->val) m = l;
        if (r < h->size && h->data[r]->val < h->data[m]->val) m = r;
        if (m == idx) break;
        heap_swap(&h->data[m], &h->data[idx]);
        idx = m;
    }
}

static bool heap_push(struct Heap* h, struct ListNode* node) {
    if (h->size == h->cap) {
        size_t ncap = h->cap ? h->cap * 2 : 1;
        struct ListNode** ndata = (struct ListNode**)realloc(h->data, ncap * sizeof(struct ListNode*));
        if (!ndata) return false;
        h->data = ndata; h->cap = ncap;
    }
    h->data[h->size] = node;
    heap_sift_up(h, h->size);
    h->size++;
    return true;
}

static struct ListNode* heap_pop(struct Heap* h) {
    if (h->size == 0) return NULL;
    struct ListNode* top = h->data[0];
    h->size--;
    if (h->size > 0) {
        h->data[0] = h->data[h->size];
        heap_sift_down(h, 0);
    }
    return top;
}

struct ListNode* mergeKLists(struct ListNode** lists, size_t k) {
    if (lists == NULL || k == 0) return NULL;
    struct Heap heap;
    heap_init(&heap, k);
    if (heap.cap == 0) return NULL;
    for (size_t i = 0; i < k; ++i) {
        if (lists[i] != NULL) {
            if (!heap_push(&heap, lists[i])) {
                heap_free(&heap);
                return NULL;
            }
        }
    }
    struct ListNode dummy = {0, NULL};
    struct ListNode* tail = &dummy;
    while (heap.size > 0) {
        struct ListNode* node = heap_pop(&heap);
        tail->next = node;
        tail = node;
        if (node->next != NULL) {
            if (!heap_push(&heap, node->next)) {
                heap_free(&heap);
                return dummy.next; // return partial result
            }
        }
    }
    tail->next = NULL;
    heap_free(&heap);
    return dummy.next;
}

int main(void) {
    // Test 1
    {
        int a1[] = {1,4,5}, a2[] = {1,3,4}, a3[] = {2,6};
        struct ListNode* lists1[3];
        lists1[0] = build_list(a1, sizeof(a1)/sizeof(a1[0]));
        lists1[1] = build_list(a2, sizeof(a2)/sizeof(a2[0]));
        lists1[2] = build_list(a3, sizeof(a3)/sizeof(a3[0]));
        struct ListNode* merged = mergeKLists(lists1, 3);
        size_t len = 0; int* arr = to_array(merged, &len);
        print_array(arr ? arr : (int[]){}, arr ? len : 0);
        free(arr);
        free_list(merged);
    }
    // Test 2
    {
        struct ListNode** lists2 = NULL;
        struct ListNode* merged = mergeKLists(lists2, 0);
        size_t len = 0; int* arr = to_array(merged, &len);
        print_array(arr ? arr : (int[]){}, arr ? len : 0);
        free(arr);
        free_list(merged);
    }
    // Test 3
    {
        struct ListNode* lists3[1];
        lists3[0] = build_list(NULL, 0);
        struct ListNode* merged = mergeKLists(lists3, 1);
        size_t len = 0; int* arr = to_array(merged, &len);
        print_array(arr ? arr : (int[]){}, arr ? len : 0);
        free(arr);
        free_list(merged);
    }
    // Test 4
    {
        int a1[] = {-10,-5,0,5}, a2[] = {-6,-3,2,2,7};
        struct ListNode* lists4[3];
        lists4[0] = build_list(a1, sizeof(a1)/sizeof(a1[0]));
        lists4[1] = build_list(a2, sizeof(a2)/sizeof(a2[0]));
        lists4[2] = build_list(NULL, 0);
        struct ListNode* merged = mergeKLists(lists4, 3);
        size_t len = 0; int* arr = to_array(merged, &len);
        print_array(arr ? arr : (int[]){}, arr ? len : 0);
        free(arr);
        free_list(merged);
    }
    // Test 5
    {
        int b1[] = {1}, b2[] = {}, b3[] = {}, b4[] = {0}, b5[] = {1,1,1}, b6[] = {-1,2};
        struct ListNode* lists5[6];
        lists5[0] = build_list(b1, sizeof(b1)/sizeof(b1[0]));
        lists5[1] = build_list(b2, sizeof(b2)/sizeof(b2[0]));
        lists5[2] = build_list(b3, sizeof(b3)/sizeof(b3[0]));
        lists5[3] = build_list(b4, sizeof(b4)/sizeof(b4[0]));
        lists5[4] = build_list(b5, sizeof(b5)/sizeof(b5[0]));
        lists5[5] = build_list(b6, sizeof(b6)/sizeof(b6[0]));
        struct ListNode* merged = mergeKLists(lists5, 6);
        size_t len = 0; int* arr = to_array(merged, &len);
        print_array(arr ? arr : (int[]){}, arr ? len : 0);
        free(arr);
        free_list(merged);
    }
    return 0;
}