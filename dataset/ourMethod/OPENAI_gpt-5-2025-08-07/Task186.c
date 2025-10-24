#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

typedef struct ListNode {
    int val;
    struct ListNode* next;
} ListNode;

/* Min-heap of ListNode* based on node->val */
typedef struct {
    ListNode** data;
    size_t size;
    size_t capacity;
} MinHeap;

static bool heap_init(MinHeap* h, size_t capacity) {
    if (h == NULL) return false;
    h->size = 0;
    h->capacity = capacity > 0 ? capacity : 1;
    h->data = (ListNode**)calloc(h->capacity, sizeof(ListNode*));
    return h->data != NULL;
}

static void heap_free(MinHeap* h) {
    if (h && h->data) {
        free(h->data);
        h->data = NULL;
        h->size = 0;
        h->capacity = 0;
    }
}

static bool heap_resize(MinHeap* h, size_t newcap) {
    if (!h) return false;
    if (newcap < h->size) return false;
    ListNode** nd = (ListNode**)realloc(h->data, newcap * sizeof(ListNode*));
    if (!nd) return false;
    h->data = nd;
    h->capacity = newcap;
    return true;
}

static void heap_swap(ListNode** a, ListNode** b) {
    ListNode* t = *a; *a = *b; *b = t;
}

static bool heap_push(MinHeap* h, ListNode* node) {
    if (!h || !node) return false;
    if (h->size == h->capacity) {
        size_t newcap = h->capacity > (SIZE_MAX / 2) ? SIZE_MAX : (h->capacity * 2);
        if (newcap == h->capacity || !heap_resize(h, newcap)) return false;
    }
    size_t i = h->size++;
    h->data[i] = node;
    while (i > 0) {
        size_t p = (i - 1) / 2;
        if (h->data[p]->val <= h->data[i]->val) break;
        heap_swap(&h->data[p], &h->data[i]);
        i = p;
    }
    return true;
}

static ListNode* heap_pop(MinHeap* h) {
    if (!h || h->size == 0) return NULL;
    ListNode* top = h->data[0];
    h->data[0] = h->data[h->size - 1];
    h->size--;
    size_t i = 0;
    while (true) {
        size_t l = 2 * i + 1, r = 2 * i + 2, smallest = i;
        if (l < h->size && h->data[l]->val < h->data[smallest]->val) smallest = l;
        if (r < h->size && h->data[r]->val < h->data[smallest]->val) smallest = r;
        if (smallest == i) break;
        heap_swap(&h->data[i], &h->data[smallest]);
        i = smallest;
    }
    return top;
}

ListNode* merge_k_lists(ListNode** lists, size_t k) {
    MinHeap heap;
    if (!heap_init(&heap, k > 0 ? k : 1)) return NULL;
    for (size_t i = 0; i < k; i++) {
        if (lists != NULL && lists[i] != NULL) {
            (void)heap_push(&heap, lists[i]);
        }
    }
    ListNode dummy = {0, NULL};
    ListNode* tail = &dummy;
    while (heap.size > 0) {
        ListNode* node = heap_pop(&heap);
        tail->next = node;
        tail = node;
        if (node->next != NULL) {
            (void)heap_push(&heap, node->next);
        }
    }
    heap_free(&heap);
    return dummy.next;
}

/* Helpers */
ListNode* build_list(const int* arr, size_t n) {
    if (arr == NULL || n == 0) return NULL;
    ListNode dummy = {0, NULL};
    ListNode* cur = &dummy;
    for (size_t i = 0; i < n; ++i) {
        ListNode* node = (ListNode*)calloc(1, sizeof(ListNode));
        if (!node) {
            // free already allocated nodes
            ListNode* p = dummy.next;
            while (p) {
                ListNode* nxt = p->next;
                free(p);
                p = nxt;
            }
            return NULL;
        }
        node->val = arr[i];
        node->next = NULL;
        cur->next = node;
        cur = node;
    }
    return dummy.next;
}

int* list_to_array(ListNode* head, size_t* out_len) {
    if (!out_len) return NULL;
    *out_len = 0;
    size_t count = 0;
    for (ListNode* cur = head; cur != NULL; cur = cur->next) {
        if (count == SIZE_MAX) return NULL;
        count++;
    }
    if (count == 0) {
        *out_len = 0;
        return (int*)calloc(0, sizeof(int));
    }
    int* arr = (int*)malloc(count * sizeof(int));
    if (!arr) {
        *out_len = 0;
        return NULL;
    }
    size_t i = 0;
    for (ListNode* cur = head; cur != NULL; cur = cur->next) {
        arr[i++] = cur->val;
    }
    *out_len = count;
    return arr;
}

void free_list(ListNode* head) {
    while (head) {
        ListNode* nxt = head->next;
        free(head);
        head = nxt;
    }
}

void print_array(const int* arr, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(",");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    // Test 1: Example 1
    {
        int a1[] = {1,4,5}, a2[] = {1,3,4}, a3[] = {2,6};
        ListNode* l1 = build_list(a1, sizeof(a1)/sizeof(a1[0]));
        ListNode* l2 = build_list(a2, sizeof(a2)/sizeof(a2[0]));
        ListNode* l3 = build_list(a3, sizeof(a3)/sizeof(a3[0]));
        ListNode* lists1[] = { l1, l2, l3 };
        ListNode* merged = merge_k_lists(lists1, sizeof(lists1)/sizeof(lists1[0]));
        size_t len = 0;
        int* arr = list_to_array(merged, &len);
        print_array(arr, len);
        free(arr);
        free_list(merged);
    }
    // Test 2: Example 2 - empty list of lists
    {
        ListNode** lists2 = NULL;
        size_t k2 = 0;
        ListNode* merged = merge_k_lists(lists2, k2);
        size_t len = 0;
        int* arr = list_to_array(merged, &len);
        print_array(arr, len);
        free(arr);
        free_list(merged);
    }
    // Test 3: Example 3 - lists = [[]]
    {
        ListNode* empty = build_list(NULL, 0);
        ListNode* lists3[] = { empty };
        ListNode* merged = merge_k_lists(lists3, 1);
        size_t len = 0;
        int* arr = list_to_array(merged, &len);
        print_array(arr, len);
        free(arr);
        free_list(merged);
        // empty is already part of merged (NULL), nothing to free separately
    }
    // Test 4: negatives and duplicates
    {
        int a1[] = {-2,-1,3}, a2[] = {1,1,1}, a3[] = {2};
        ListNode* l1 = build_list(a1, sizeof(a1)/sizeof(a1[0]));
        ListNode* l2 = build_list(a2, sizeof(a2)/sizeof(a2[0]));
        ListNode* l3 = build_list(a3, sizeof(a3)/sizeof(a3[0]));
        ListNode* lists4[] = { l1, l2, l3 };
        ListNode* merged = merge_k_lists(lists4, sizeof(lists4)/sizeof(lists4[0]));
        size_t len = 0;
        int* arr = list_to_array(merged, &len);
        print_array(arr, len);
        free(arr);
        free_list(merged);
    }
    // Test 5: single list
    {
        int a1[] = {0,5,10};
        ListNode* l1 = build_list(a1, sizeof(a1)/sizeof(a1[0]));
        ListNode* lists5[] = { l1 };
        ListNode* merged = merge_k_lists(lists5, 1);
        size_t len = 0;
        int* arr = list_to_array(merged, &len);
        print_array(arr, len);
        free(arr);
        free_list(merged);
    }
    return 0;
}