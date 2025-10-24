#include <stdio.h>
#include <stdlib.h>

struct ListNode {
    int val;
    struct ListNode* next;
};

struct ListNode* mergeKLists(struct ListNode** lists, int k) {
    // Min-heap of ListNode* by node->val
    if (k == 0) return NULL;

    // Heap structure
    struct ListNode** heap = (struct ListNode**)malloc(sizeof(struct ListNode*) * (k + 1));
    int heapSize = 0;

    // Helpers
    auto void swap(struct ListNode** a, struct ListNode** b) {
        struct ListNode* tmp = *a; *a = *b; *b = tmp;
    }
    auto void siftUp(int idx) {
        while (idx > 1) {
            int p = idx / 2;
            if (heap[p]->val <= heap[idx]->val) break;
            swap(&heap[p], &heap[idx]);
            idx = p;
        }
    }
    auto void siftDown(int idx) {
        while (1) {
            int l = idx * 2, r = l + 1, smallest = idx;
            if (l <= heapSize && heap[l]->val < heap[smallest]->val) smallest = l;
            if (r <= heapSize && heap[r]->val < heap[smallest]->val) smallest = r;
            if (smallest == idx) break;
            swap(&heap[smallest], &heap[idx]);
            idx = smallest;
        }
    }
    auto void push(struct ListNode* node) {
        heap[++heapSize] = node;
        siftUp(heapSize);
    }
    auto struct ListNode* pop() {
        if (heapSize == 0) return NULL;
        struct ListNode* top = heap[1];
        heap[1] = heap[heapSize--];
        if (heapSize > 0) siftDown(1);
        return top;
    }

    // Initialize heap
    for (int i = 0; i < k; i++) {
        if (lists[i] != NULL) push(lists[i]);
    }

    struct ListNode dummy; dummy.val = 0; dummy.next = NULL;
    struct ListNode* tail = &dummy;

    while (heapSize > 0) {
        struct ListNode* n = pop();
        tail->next = n;
        tail = n;
        if (n->next) push(n->next);
    }
    tail->next = NULL;

    free(heap);
    return dummy.next;
}

// Helpers
struct ListNode* createList(const int* arr, int n) {
    struct ListNode dummy; dummy.val = 0; dummy.next = NULL;
    struct ListNode* t = &dummy;
    for (int i = 0; i < n; i++) {
        struct ListNode* node = (struct ListNode*)malloc(sizeof(struct ListNode));
        node->val = arr[i];
        node->next = NULL;
        t->next = node;
        t = node;
    }
    return dummy.next;
}

void freeList(struct ListNode* head) {
    while (head) {
        struct ListNode* n = head->next;
        free(head);
        head = n;
    }
}

void printList(struct ListNode* head) {
    printf("[");
    int first = 1;
    while (head) {
        if (!first) printf(",");
        printf("%d", head->val);
        first = 0;
        head = head->next;
    }
    printf("]\n");
}

int main() {
    // Test 1
    int a1[] = {1,4,5}, a2[] = {1,3,4}, a3[] = {2,6};
    int* arrs1[] = {a1, a2, a3};
    int sizes1[] = {3,3,2};
    int k1 = 3;
    struct ListNode** lists1 = (struct ListNode**)malloc(sizeof(struct ListNode*) * k1);
    for (int i = 0; i < k1; i++) lists1[i] = createList(arrs1[i], sizes1[i]);
    struct ListNode* merged1 = mergeKLists(lists1, k1);
    printList(merged1);
    freeList(merged1);
    free(lists1);

    // Test 2: empty list of lists
    int k2 = 0;
    struct ListNode** lists2 = NULL;
    struct ListNode* merged2 = mergeKLists(lists2, k2);
    printList(merged2);
    // no free needed

    // Test 3: one empty list
    int k3 = 1;
    struct ListNode** lists3 = (struct ListNode**)malloc(sizeof(struct ListNode*) * k3);
    lists3[0] = createList(NULL, 0);
    struct ListNode* merged3 = mergeKLists(lists3, k3);
    printList(merged3);
    freeList(merged3);
    free(lists3);

    // Test 4
    int b1[] = {-10,-5,0,5}, b2[] = {1,2,3}, b3[] = {-10,-10,100};
    int* arrs4[] = {b1, b2, b3};
    int sizes4[] = {4,3,3};
    int k4 = 3;
    struct ListNode** lists4 = (struct ListNode**)malloc(sizeof(struct ListNode*) * k4);
    for (int i = 0; i < k4; i++) lists4[i] = createList(arrs4[i], sizes4[i]);
    struct ListNode* merged4 = mergeKLists(lists4, k4);
    printList(merged4);
    freeList(merged4);
    for (int i = 0; i < k4; i++) freeList(lists4[i]); // already freed via merged, but safe to skip; to avoid double free, comment this line if needed
    free(lists4);

    // Test 5
    int c1[] = {1}, c2[] = {0}, c3[] = {-1,2,2}, c5[] = {3,3,3};
    int* arrs5[] = {c1, c2, c3, NULL, c5};
    int sizes5[] = {1,1,3,0,3};
    int k5 = 5;
    struct ListNode** lists5 = (struct ListNode**)malloc(sizeof(struct ListNode*) * k5);
    for (int i = 0; i < k5; i++) lists5[i] = (sizes5[i] == 0 ? createList(NULL, 0) : createList(arrs5[i], sizes5[i]));
    struct ListNode* merged5 = mergeKLists(lists5, k5);
    printList(merged5);
    freeList(merged5);
    free(lists5);

    return 0;
}