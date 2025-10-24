
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

// Security: Define ListNode structure with proper field ordering
typedef struct ListNode {
    int val;
    struct ListNode *next;
} ListNode;

// Security: Helper structure for min-heap with bounds checking
typedef struct {
    ListNode** data;
    int size;
    int capacity;
} MinHeap;

// Security: Create heap with validated capacity
MinHeap* createHeap(int capacity) {
    // Security: Validate capacity to prevent integer overflow
    if (capacity <= 0 || capacity > 10000) {
        return NULL;
    }
    
    // Security: Allocate heap structure and check for failure
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (!heap) {
        return NULL;
    }
    
    // Security: Allocate data array with overflow check
    heap->data = (ListNode**)calloc(capacity, sizeof(ListNode*));
    if (!heap->data) {
        free(heap);
        return NULL;
    }
    
    // Security: Initialize all fields
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

// Security: Free heap memory safely
void freeHeap(MinHeap* heap) {
    if (heap) {
        if (heap->data) {
            free(heap->data);
        }
        free(heap);
    }
}

// Security: Swap with null checks
void swap(ListNode** a, ListNode** b) {
    if (!a || !b) return;
    ListNode* temp = *a;
    *a = *b;
    *b = temp;
}

// Security: Heapify down with bounds checking
void heapifyDown(MinHeap* heap, int idx) {
    if (!heap || !heap->data || idx < 0 || idx >= heap->size) {
        return;
    }
    
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    
    // Security: Bounds check before array access
    if (left < heap->size && heap->data[left] && heap->data[smallest] &&
        heap->data[left]->val < heap->data[smallest]->val) {
        smallest = left;
    }
    
    // Security: Bounds check before array access
    if (right < heap->size && heap->data[right] && heap->data[smallest] &&
        heap->data[right]->val < heap->data[smallest]->val) {
        smallest = right;
    }
    
    if (smallest != idx) {
        swap(&heap->data[idx], &heap->data[smallest]);
        heapifyDown(heap, smallest);
    }
}

// Security: Heapify up with bounds checking
void heapifyUp(MinHeap* heap, int idx) {
    if (!heap || !heap->data || idx <= 0 || idx >= heap->size) {
        return;
    }
    
    int parent = (idx - 1) / 2;
    
    // Security: Bounds check and null check before comparison
    if (parent >= 0 && heap->data[idx] && heap->data[parent] &&
        heap->data[idx]->val < heap->data[parent]->val) {
        swap(&heap->data[idx], &heap->data[parent]);
        heapifyUp(heap, parent);
    }
}

// Security: Push with capacity check
bool heapPush(MinHeap* heap, ListNode* node) {
    // Security: Validate inputs
    if (!heap || !heap->data || !node || heap->size >= heap->capacity) {
        return false;
    }
    
    // Security: Add node at end and heapify up
    heap->data[heap->size] = node;
    heapifyUp(heap, heap->size);
    heap->size++;
    return true;
}

// Security: Pop with empty check
ListNode* heapPop(MinHeap* heap) {
    // Security: Validate heap and check if empty
    if (!heap || !heap->data || heap->size <= 0) {
        return NULL;
    }
    
    ListNode* minNode = heap->data[0];
    heap->size--;
    
    if (heap->size > 0) {
        heap->data[0] = heap->data[heap->size];
        heapifyDown(heap, 0);
    }
    
    return minNode;
}

// Security: Main merge function with comprehensive validation
ListNode* mergeKLists(ListNode** lists, int listsSize) {
    // Security: Validate input parameters
    if (!lists || listsSize < 0 || listsSize > 10000) {
        return NULL;
    }
    
    if (listsSize == 0) {
        return NULL;
    }
    
    // Security: Create heap with validated size
    MinHeap* heap = createHeap(listsSize);
    if (!heap) {
        return NULL;
    }
    
    // Security: Add non-null list heads to heap with bounds checking
    for (int i = 0; i < listsSize; i++) {
        if (lists[i] != NULL) {
            if (!heapPush(heap, lists[i])) {
                freeHeap(heap);
                return NULL;
            }
        }
    }
    
    // Security: Create dummy head for result list
    ListNode dummy;
    dummy.val = 0;
    dummy.next = NULL;
    ListNode* tail = &dummy;
    
    // Security: Process heap with iteration limit to prevent infinite loops
    int iterations = 0;
    const int MAX_ITERATIONS = 10000; // Per problem constraints
    
    while (heap->size > 0 && iterations < MAX_ITERATIONS) {
        ListNode* minNode = heapPop(heap);
        
        // Security: Validate popped node
        if (!minNode) {
            break;
        }
        
        // Security: Append to result list
        tail->next = minNode;
        tail = tail->next;
        
        // Security: If current list has more nodes, add to heap
        if (minNode->next != NULL) {
            if (!heapPush(heap, minNode->next)) {
                freeHeap(heap);
                return dummy.next;
            }
        }
        
        iterations++;
    }
    
    freeHeap(heap);
    return dummy.next;
}

// Security: Create list with bounds checking
ListNode* createList(int* values, int size) {
    // Security: Validate inputs
    if (!values || size <= 0 || size > 500) {
        return NULL;
    }
    
    // Security: Allocate head with null check
    ListNode* head = (ListNode*)malloc(sizeof(ListNode));
    if (!head) {
        return NULL;
    }
    head->val = values[0];
    head->next = NULL;
    
    ListNode* current = head;
    // Security: Bounds-checked iteration
    for (int i = 1; i < size; i++) {
        current->next = (ListNode*)malloc(sizeof(ListNode));
        if (!current->next) {
            // Security: Clean up on allocation failure
            while (head) {
                ListNode* temp = head;
                head = head->next;
                free(temp);
            }
            return NULL;
        }
        current->next->val = values[i];
        current->next->next = NULL;
        current = current->next;
    }
    return head;
}

// Security: Free list with cycle detection
void freeList(ListNode* head) {
    int count = 0;
    const int MAX_NODES = 10000;
    
    while (head && count < MAX_NODES) {
        ListNode* temp = head;
        head = head->next;
        free(temp);
        count++;
    }
}

// Security: Print list with bounds checking
void printList(ListNode* head) {
    printf("[");
    bool first = true;
    int count = 0;
    const int MAX_NODES = 10000;
    
    while (head && count < MAX_NODES) {
        if (!first) printf(",");
        printf("%d", head->val);
        first = false;
        head = head->next;
        count++;
    }
    printf("]\\n");
}

int main() {
    // Test case 1
    int arr1[] = {1, 4, 5};
    int arr2[] = {1, 3, 4};
    int arr3[] = {2, 6};
    ListNode* lists1[] = {
        createList(arr1, 3),
        createList(arr2, 3),
        createList(arr3, 2)
    };
    ListNode* result1 = mergeKLists(lists1, 3);
    printf("Test 1: ");
    printList(result1);
    freeList(result1);
    
    // Test case 2: empty input
    ListNode* result2 = mergeKLists(NULL, 0);
    printf("Test 2: ");
    printList(result2);
    
    // Test case 3: single empty list
    ListNode* lists3[] = {NULL};
    ListNode* result3 = mergeKLists(lists3, 1);
    printf("Test 3: ");
    printList(result3);
    
    // Test case 4: single list
    int arr4[] = {1, 2, 3};
    ListNode* lists4[] = {createList(arr4, 3)};
    ListNode* result4 = mergeKLists(lists4, 1);
    printf("Test 4: ");
    printList(result4);
    freeList(result4);
    
    // Test case 5: negative numbers
    int arr5[] = {-2, -1, 0};
    int arr6[] = {-3, 1, 2};
    ListNode* lists5[] = {
        createList(arr5, 3),
        createList(arr6, 3)
    };
    ListNode* result5 = mergeKLists(lists5, 2);
    printf("Test 5: ");
    printList(result5);
    freeList(result5);
    
    return 0;
}
