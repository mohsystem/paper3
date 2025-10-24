
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct ListNode {
    int val;
    struct ListNode *next;
} ListNode;

typedef struct {
    ListNode** nodes;
    int size;
    int capacity;
} MinHeap;

MinHeap* createHeap(int capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->nodes = (ListNode**)malloc(capacity * sizeof(ListNode*));
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

void swap(ListNode** a, ListNode** b) {
    ListNode* temp = *a;
    *a = *b;
    *b = temp;
}

void heapifyUp(MinHeap* heap, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heap->nodes[idx]->val < heap->nodes[parent]->val) {
            swap(&heap->nodes[idx], &heap->nodes[parent]);
            idx = parent;
        } else {
            break;
        }
    }
}

void heapifyDown(MinHeap* heap, int idx) {
    while (1) {
        int smallest = idx;
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        
        if (left < heap->size && heap->nodes[left]->val < heap->nodes[smallest]->val) {
            smallest = left;
        }
        if (right < heap->size && heap->nodes[right]->val < heap->nodes[smallest]->val) {
            smallest = right;
        }
        if (smallest != idx) {
            swap(&heap->nodes[idx], &heap->nodes[smallest]);
            idx = smallest;
        } else {
            break;
        }
    }
}

void push(MinHeap* heap, ListNode* node) {
    if (heap->size < heap->capacity) {
        heap->nodes[heap->size] = node;
        heapifyUp(heap, heap->size);
        heap->size++;
    }
}

ListNode* pop(MinHeap* heap) {
    if (heap->size == 0) return NULL;
    ListNode* result = heap->nodes[0];
    heap->nodes[0] = heap->nodes[heap->size - 1];
    heap->size--;
    if (heap->size > 0) {
        heapifyDown(heap, 0);
    }
    return result;
}

ListNode* mergeKLists(ListNode** lists, int listsSize) {
    if (lists == NULL || listsSize == 0) {
        return NULL;
    }
    
    MinHeap* heap = createHeap(listsSize);
    
    for (int i = 0; i < listsSize; i++) {
        if (lists[i] != NULL) {
            push(heap, lists[i]);
        }
    }
    
    ListNode* dummy = (ListNode*)malloc(sizeof(ListNode));
    dummy->val = 0;
    dummy->next = NULL;
    ListNode* current = dummy;
    
    while (heap->size > 0) {
        ListNode* node = pop(heap);
        current->next = node;
        current = current->next;
        
        if (node->next != NULL) {
            push(heap, node->next);
        }
    }
    
    ListNode* result = dummy->next;
    free(dummy);
    free(heap->nodes);
    free(heap);
    return result;
}

ListNode* createList(int* arr, int size) {
    if (arr == NULL || size == 0) return NULL;
    ListNode* dummy = (ListNode*)malloc(sizeof(ListNode));
    ListNode* current = dummy;
    for (int i = 0; i < size; i++) {
        current->next = (ListNode*)malloc(sizeof(ListNode));
        current->next->val = arr[i];
        current->next->next = NULL;
        current = current->next;
    }
    ListNode* result = dummy->next;
    free(dummy);
    return result;
}

void printList(ListNode* head) {
    printf("[");
    while (head != NULL) {
        printf("%d", head->val);
        if (head->next != NULL) printf(",");
        head = head->next;
    }
    printf("]\\n");
}

int main() {
    // Test case 1
    int arr1[] = {1,4,5};
    int arr2[] = {1,3,4};
    int arr3[] = {2,6};
    ListNode* lists1[] = {createList(arr1, 3), createList(arr2, 3), createList(arr3, 2)};
    printf("Test 1: ");
    printList(mergeKLists(lists1, 3));
    
    // Test case 2
    printf("Test 2: ");
    printList(mergeKLists(NULL, 0));
    
    // Test case 3
    ListNode* lists3[] = {NULL};
    printf("Test 3: ");
    printList(mergeKLists(lists3, 1));
    
    // Test case 4
    int arr4[] = {1,2,3};
    int arr5[] = {4,5,6};
    ListNode* lists4[] = {createList(arr4, 3), createList(arr5, 3)};
    printf("Test 4: ");
    printList(mergeKLists(lists4, 2));
    
    // Test case 5
    int arr6[] = {-2,-1,0};
    int arr7[] = {-3};
    int arr8[] = {1,2};
    ListNode* lists5[] = {createList(arr6, 3), createList(arr7, 1), createList(arr8, 2)};
    printf("Test 5: ");
    printList(mergeKLists(lists5, 3));
    
    return 0;
}
