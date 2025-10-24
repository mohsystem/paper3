
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

// Security: Define maximum capacity per constraints
#define MAX_ELEMENTS 50000
#define INPUT_MIN -100000
#define INPUT_MAX 100000

// Min heap structure
typedef struct {
    int* data;
    size_t size;
    size_t capacity;
} MinHeap;

// Max heap structure
typedef struct {
    int* data;
    size_t size;
    size_t capacity;
} MaxHeap;

// MedianFinder structure
typedef struct {
    MaxHeap* maxHeap;  // Stores smaller half
    MinHeap* minHeap;  // Stores larger half
    size_t totalCount;
} MedianFinder;

// Security: Initialize heap with bounds checking
MinHeap* createMinHeap(size_t capacity) {
    if (capacity == 0 || capacity > MAX_ELEMENTS) {
        return NULL;
    }
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (!heap) return NULL;
    
    heap->data = (int*)calloc(capacity, sizeof(int));
    if (!heap->data) {
        free(heap);
        return NULL;
    }
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

MaxHeap* createMaxHeap(size_t capacity) {
    if (capacity == 0 || capacity > MAX_ELEMENTS) {
        return NULL;
    }
    MaxHeap* heap = (MaxHeap*)malloc(sizeof(MaxHeap));
    if (!heap) return NULL;
    
    heap->data = (int*)calloc(capacity, sizeof(int));
    if (!heap->data) {
        free(heap);
        return NULL;
    }
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

// Helper functions for min heap operations
void minHeapSwap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void minHeapifyUp(MinHeap* heap, size_t idx) {
    if (!heap || idx >= heap->size) return;
    
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (heap->data[idx] < heap->data[parent]) {
            minHeapSwap(&heap->data[idx], &heap->data[parent]);
            idx = parent;
        } else {
            break;
        }
    }
}

void minHeapifyDown(MinHeap* heap, size_t idx) {
    if (!heap || idx >= heap->size) return;
    
    while (true) {
        size_t smallest = idx;
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        
        if (left < heap->size && heap->data[left] < heap->data[smallest]) {
            smallest = left;
        }
        if (right < heap->size && heap->data[right] < heap->data[smallest]) {
            smallest = right;
        }
        if (smallest != idx) {
            minHeapSwap(&heap->data[idx], &heap->data[smallest]);
            idx = smallest;
        } else {
            break;
        }
    }
}

bool minHeapPush(MinHeap* heap, int val) {
    if (!heap || heap->size >= heap->capacity) return false;
    
    heap->data[heap->size] = val;
    minHeapifyUp(heap, heap->size);
    heap->size++;
    return true;
}

bool minHeapPop(MinHeap* heap, int* val) {
    if (!heap || heap->size == 0) return false;
    
    if (val) *val = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    if (heap->size > 0) {
        minHeapifyDown(heap, 0);
    }
    return true;
}

int minHeapTop(MinHeap* heap) {
    if (!heap || heap->size == 0) return 0;
    return heap->data[0];
}

// Helper functions for max heap operations
void maxHeapifyUp(MaxHeap* heap, size_t idx) {
    if (!heap || idx >= heap->size) return;
    
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (heap->data[idx] > heap->data[parent]) {
            minHeapSwap(&heap->data[idx], &heap->data[parent]);
            idx = parent;
        } else {
            break;
        }
    }
}

void maxHeapifyDown(MaxHeap* heap, size_t idx) {
    if (!heap || idx >= heap->size) return;
    
    while (true) {
        size_t largest = idx;
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        
        if (left < heap->size && heap->data[left] > heap->data[largest]) {
            largest = left;
        }
        if (right < heap->size && heap->data[right] > heap->data[largest]) {
            largest = right;
        }
        if (largest != idx) {
            minHeapSwap(&heap->data[idx], &heap->data[largest]);
            idx = largest;
        } else {
            break;
        }
    }
}

bool maxHeapPush(MaxHeap* heap, int val) {
    if (!heap || heap->size >= heap->capacity) return false;
    
    heap->data[heap->size] = val;
    maxHeapifyUp(heap, heap->size);
    heap->size++;
    return true;
}

bool maxHeapPop(MaxHeap* heap, int* val) {
    if (!heap || heap->size == 0) return false;
    
    if (val) *val = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    if (heap->size > 0) {
        maxHeapifyDown(heap, 0);
    }
    return true;
}

int maxHeapTop(MaxHeap* heap) {
    if (!heap || heap->size == 0) return 0;
    return heap->data[0];
}

// Security: Initialize MedianFinder with proper memory checks
MedianFinder* medianFinderCreate() {
    MedianFinder* obj = (MedianFinder*)malloc(sizeof(MedianFinder));
    if (!obj) return NULL;
    
    obj->maxHeap = createMaxHeap(MAX_ELEMENTS);
    obj->minHeap = createMinHeap(MAX_ELEMENTS);
    
    if (!obj->maxHeap || !obj->minHeap) {
        if (obj->maxHeap) {
            free(obj->maxHeap->data);
            free(obj->maxHeap);
        }
        if (obj->minHeap) {
            free(obj->minHeap->data);
            free(obj->minHeap);
        }
        free(obj);
        return NULL;
    }
    
    obj->totalCount = 0;
    return obj;
}

// Security: Validate input range and prevent overflow
bool medianFinderAddNum(MedianFinder* obj, int num) {
    if (!obj) return false;
    
    // Security: Validate input is within constraints
    if (num < INPUT_MIN || num > INPUT_MAX) {
        return false;
    }
    
    // Security: Prevent exceeding maximum elements
    if (obj->totalCount >= MAX_ELEMENTS) {
        return false;
    }
    
    // Add to appropriate heap
    if (obj->maxHeap->size == 0 || num <= maxHeapTop(obj->maxHeap)) {
        if (!maxHeapPush(obj->maxHeap, num)) return false;
    } else {
        if (!minHeapPush(obj->minHeap, num)) return false;
    }
    
    // Rebalance heaps
    if (obj->maxHeap->size > obj->minHeap->size + 1) {
        int val = maxHeapTop(obj->maxHeap);
        maxHeapPop(obj->maxHeap, NULL);
        minHeapPush(obj->minHeap, val);
    } else if (obj->minHeap->size > obj->maxHeap->size) {
        int val = minHeapTop(obj->minHeap);
        minHeapPop(obj->minHeap, NULL);
        maxHeapPush(obj->maxHeap, val);
    }
    
    obj->totalCount++;
    return true;
}

// Security: Check for empty structure before finding median
double medianFinderFindMedian(MedianFinder* obj) {
    if (!obj || obj->totalCount == 0) {
        return 0.0;
    }
    
    if (obj->maxHeap->size > obj->minHeap->size) {
        return (double)maxHeapTop(obj->maxHeap);
    }
    
    // Use double to prevent integer overflow
    return ((double)maxHeapTop(obj->maxHeap) + (double)minHeapTop(obj->minHeap)) / 2.0;
}

// Security: Properly free all allocated memory
void medianFinderFree(MedianFinder* obj) {
    if (!obj) return;
    
    if (obj->maxHeap) {
        if (obj->maxHeap->data) {
            free(obj->maxHeap->data);
        }
        free(obj->maxHeap);
    }
    
    if (obj->minHeap) {
        if (obj->minHeap->data) {
            free(obj->minHeap->data);
        }
        free(obj->minHeap);
    }
    
    free(obj);
}

int main() {
    // Test case 1: Example from problem
    {
        MedianFinder* mf = medianFinderCreate();
        if (mf) {
            medianFinderAddNum(mf, 1);
            medianFinderAddNum(mf, 2);
            double median1 = medianFinderFindMedian(mf);
            medianFinderAddNum(mf, 3);
            double median2 = medianFinderFindMedian(mf);
            printf("Test 1: %.1f (expected 1.5), %.1f (expected 2.0)\\n", median1, median2);
            medianFinderFree(mf);
        }
    }
    
    // Test case 2: Single element
    {
        MedianFinder* mf = medianFinderCreate();
        if (mf) {
            medianFinderAddNum(mf, 5);
            double median = medianFinderFindMedian(mf);
            printf("Test 2: %.1f (expected 5.0)\\n", median);
            medianFinderFree(mf);
        }
    }
    
    // Test case 3: Negative numbers
    {
        MedianFinder* mf = medianFinderCreate();
        if (mf) {
            medianFinderAddNum(mf, -1);
            medianFinderAddNum(mf, -2);
            medianFinderAddNum(mf, -3);
            double median = medianFinderFindMedian(mf);
            printf("Test 3: %.1f (expected -2.0)\\n", median);
            medianFinderFree(mf);
        }
    }
    
    // Test case 4: Mixed positive and negative
    {
        MedianFinder* mf = medianFinderCreate();
        if (mf) {
            medianFinderAddNum(mf, -1);
            medianFinderAddNum(mf, 0);
            medianFinderAddNum(mf, 1);
            medianFinderAddNum(mf, 2);
            double median = medianFinderFindMedian(mf);
            printf("Test 4: %.1f (expected 0.5)\\n", median);
            medianFinderFree(mf);
        }
    }
    
    // Test case 5: Large values at boundaries
    {
        MedianFinder* mf = medianFinderCreate();
        if (mf) {
            medianFinderAddNum(mf, -100000);
            medianFinderAddNum(mf, 100000);
            double median = medianFinderFindMedian(mf);
            printf("Test 5: %.1f (expected 0.0)\\n", median);
            medianFinderFree(mf);
        }
    }
    
    return 0;
}
