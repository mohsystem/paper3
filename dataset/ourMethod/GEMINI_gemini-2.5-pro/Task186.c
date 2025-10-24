#include <stdio.h>
#include <stdlib.h>

struct ListNode {
    int val;
    struct ListNode *next;
};

// --- Min-Heap Implementation for ListNode* ---
typedef struct {
    struct ListNode** data;
    int size;
    int capacity;
} MinHeap;

MinHeap* createMinHeap(int capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (!heap) return NULL;
    heap->data = (struct ListNode**)malloc(capacity * sizeof(struct ListNode*));
    if (!heap->data) {
        free(heap);
        return NULL;
    }
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

void swap(struct ListNode** a, struct ListNode** b) {
    struct ListNode* temp = *a;
    *a = *b;
    *b = temp;
}

void minHeapify(MinHeap* heap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap->size && heap->data[left]->val < heap->data[smallest]->val) {
        smallest = left;
    }
    if (right < heap->size && heap->data[right]->val < heap->data[smallest]->val) {
        smallest = right;
    }
    if (smallest != idx) {
        swap(&heap->data[idx], &heap->data[smallest]);
        minHeapify(heap, smallest);
    }
}

struct ListNode* popMinHeap(MinHeap* heap) {
    if (heap->size == 0) return NULL;
    struct ListNode* root = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    minHeapify(heap, 0);
    return root;
}

void pushMinHeap(MinHeap* heap, struct ListNode* node) {
    if (heap->size == heap->capacity) return; // Capacity should be sufficient
    heap->size++;
    int i = heap->size - 1;
    heap->data[i] = node;
    while (i != 0 && heap->data[(i - 1) / 2]->val > heap->data[i]->val) {
        swap(&heap->data[i], &heap->data[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void destroyMinHeap(MinHeap* heap) {
    if (heap) {
        free(heap->data);
        free(heap);
    }
}
// --- End of Min-Heap Implementation ---

struct ListNode* mergeKLists(struct ListNode** lists, int listsSize) {
    if (lists == NULL || listsSize == 0) {
        return NULL;
    }
    MinHeap* minHeap = createMinHeap(listsSize);
    if (!minHeap) return NULL;

    for (int i = 0; i < listsSize; i++) {
        if (lists[i] != NULL) {
            pushMinHeap(minHeap, lists[i]);
        }
    }

    struct ListNode dummy;
    dummy.next = NULL;
    struct ListNode* current = &dummy;

    while (minHeap->size > 0) {
        struct ListNode* minNode = popMinHeap(minHeap);
        current->next = minNode;
        current = current->next;
        if (minNode->next != NULL) {
            pushMinHeap(minHeap, minNode->next);
        }
    }

    destroyMinHeap(minHeap);
    return dummy.next;
}

// --- Helper Functions for Testing ---
struct ListNode* createLinkedList(const int* arr, int size) {
    if (arr == NULL || size == 0) {
        return NULL;
    }
    struct ListNode* head = (struct ListNode*)malloc(sizeof(struct ListNode));
    if (!head) return NULL;
    head->val = arr[0];
    head->next = NULL;

    struct ListNode* current = head;
    for (int i = 1; i < size; ++i) {
        current->next = (struct ListNode*)malloc(sizeof(struct ListNode));
        if (!current->next) {
            struct ListNode* temp;
            while(head) { temp = head; head = head->next; free(temp); }
            return NULL;
        }
        current = current->next;
        current->val = arr[i];
        current->next = NULL;
    }
    return head;
}

void printLinkedList(struct ListNode* head) {
    printf("[");
    while (head) {
        printf("%d", head->val);
        if (head->next) printf(", ");
        head = head->next;
    }
    printf("]\n");
}

void freeLinkedList(struct ListNode* head) {
    struct ListNode* temp;
    while (head) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

int main() {
    // Test Case 1
    int arr1[] = {1, 4, 5}; int s1 = sizeof(arr1)/sizeof(arr1[0]);
    int arr2[] = {1, 3, 4}; int s2 = sizeof(arr2)/sizeof(arr2[0]);
    int arr3[] = {2, 6};   int s3 = sizeof(arr3)/sizeof(arr3[0]);
    struct ListNode* lists1[] = {createLinkedList(arr1, s1), createLinkedList(arr2, s2), createLinkedList(arr3, s3)};
    printf("Test Case 1:\n");
    struct ListNode* result1 = mergeKLists(lists1, 3);
    printLinkedList(result1);
    freeLinkedList(result1);

    // Test Case 2
    printf("Test Case 2:\n");
    struct ListNode* result2 = mergeKLists(NULL, 0);
    printLinkedList(result2);
    freeLinkedList(result2);

    // Test Case 3
    struct ListNode* lists3[] = {createLinkedList(NULL, 0)};
    printf("Test Case 3:\n");
    struct ListNode* result3 = mergeKLists(lists3, 1);
    printLinkedList(result3);
    freeLinkedList(result3);

    // Test Case 4
    int arr5[] = {1}; int s5 = sizeof(arr5)/sizeof(arr5[0]);
    struct ListNode* lists4[] = {createLinkedList(NULL, 0), createLinkedList(arr5, s5)};
    printf("Test Case 4:\n");
    struct ListNode* result4 = mergeKLists(lists4, 2);
    printLinkedList(result4);
    freeLinkedList(result4);

    // Test Case 5
    int arr6[] = {9}; int s6 = sizeof(arr6)/sizeof(arr6[0]);
    int arr7[] = {1, 5, 10}; int s7 = sizeof(arr7)/sizeof(arr7[0]);
    int arr8[] = {2, 3, 7, 12}; int s8 = sizeof(arr8)/sizeof(arr8[0]);
    struct ListNode* lists5[] = {createLinkedList(arr6, s6), createLinkedList(arr7, s7), createLinkedList(arr8, s8)};
    printf("Test Case 5:\n");
    struct ListNode* result5 = mergeKLists(lists5, 3);
    printLinkedList(result5);
    freeLinkedList(result5);

    return 0;
}