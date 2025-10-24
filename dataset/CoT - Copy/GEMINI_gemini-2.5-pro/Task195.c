#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Heap Data Structure Implementation ---

typedef struct {
    int* data;
    size_t size;
    size_t capacity;
    int is_max_heap; // 1 for max-heap, 0 for min-heap
} Heap;

// Forward declarations
void swap(int* a, int* b);
void heapifyUp(Heap* h, size_t index);
void heapifyDown(Heap* h, size_t index);

Heap* createHeap(size_t capacity, int is_max_heap) {
    Heap* h = (Heap*)malloc(sizeof(Heap));
    if (!h) return NULL;
    h->data = (int*)malloc(capacity * sizeof(int));
    if (!h->data) {
        free(h);
        return NULL;
    }
    h->size = 0;
    h->capacity = capacity;
    h->is_max_heap = is_max_heap;
    return h;
}

void destroyHeap(Heap* h) {
    if (h) {
        free(h->data);
        free(h);
    }
}

void resizeHeap(Heap* h) {
    if (h->size == h->capacity) {
        size_t new_capacity = h->capacity == 0 ? 1 : h->capacity * 2;
        int* temp = (int*)realloc(h->data, new_capacity * sizeof(int));
        if (!temp) {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(EXIT_FAILURE);
        }
        h->data = temp;
        h->capacity = new_capacity;
    }
}

size_t parent(size_t i) { return (i - 1) / 2; }
size_t leftChild(size_t i) { return 2 * i + 1; }
size_t rightChild(size_t i) { return 2 * i + 2; }

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Comparison logic: returns true if relation holds (parent > child for max-heap)
int compare(int parent_val, int child_val, int is_max_heap) {
    if (is_max_heap) {
        return parent_val >= child_val;
    } else {
        return parent_val <= child_val;
    }
}

void heapifyUp(Heap* h, size_t index) {
    if (index > 0 && !compare(h->data[parent(index)], h->data[index], h->is_max_heap)) {
        swap(&h->data[parent(index)], &h->data[index]);
        heapifyUp(h, parent(index));
    }
}

void heapifyDown(Heap* h, size_t index) {
    size_t target = index;
    size_t l = leftChild(index);
    size_t r = rightChild(index);

    if (l < h->size && !compare(h->data[target], h->data[l], h->is_max_heap)) {
        target = l;
    }
    if (r < h->size && !compare(h->data[target], h->data[r], h->is_max_heap)) {
        target = r;
    }

    if (target != index) {
        swap(&h->data[index], &h->data[target]);
        heapifyDown(h, target);
    }
}

void heapPush(Heap* h, int value) {
    resizeHeap(h);
    h->data[h->size] = value;
    h->size++;
    heapifyUp(h, h->size - 1);
}

int heapPop(Heap* h) {
    if (h->size == 0) exit(EXIT_FAILURE);
    int root = h->data[0];
    h->data[0] = h->data[h->size - 1];
    h->size--;
    if(h->size > 0) heapifyDown(h, 0);
    return root;
}

int heapPeek(Heap* h) {
    if (h->size == 0) exit(EXIT_FAILURE);
    return h->data[0];
}

// --- MedianFinder Implementation ---

typedef struct {
    Heap* smallHalf; // max-heap
    Heap* largeHalf; // min-heap
} MedianFinder;

MedianFinder* medianFinderCreate() {
    MedianFinder* obj = (MedianFinder*)malloc(sizeof(MedianFinder));
    if (!obj) return NULL;
    obj->smallHalf = createHeap(10, 1); // is_max_heap = true
    obj->largeHalf = createHeap(10, 0); // is_max_heap = false
    if (!obj->smallHalf || !obj->largeHalf) {
        destroyHeap(obj->smallHalf);
        destroyHeap(obj->largeHalf);
        free(obj);
        return NULL;
    }
    return obj;
}

void medianFinderDestroy(MedianFinder* obj) {
    if (obj) {
        destroyHeap(obj->smallHalf);
        destroyHeap(obj->largeHalf);
        free(obj);
    }
}

void medianFinderAddNum(MedianFinder* obj, int num) {
    heapPush(obj->smallHalf, num);
    heapPush(obj->largeHalf, heapPop(obj->smallHalf));
    if (obj->largeHalf->size > obj->smallHalf->size) {
        heapPush(obj->smallHalf, heapPop(obj->largeHalf));
    }
}

double medianFinderFindMedian(MedianFinder* obj) {
    if (obj->smallHalf->size > obj->largeHalf->size) {
        return (double)heapPeek(obj->smallHalf);
    } else {
        return (heapPeek(obj->smallHalf) + heapPeek(obj->largeHalf)) / 2.0;
    }
}

// --- Main function with test cases ---

int main() {
    // Test Case 1: Example from prompt
    printf("Test Case 1:\n");
    MedianFinder* mf1 = medianFinderCreate();
    medianFinderAddNum(mf1, 1);
    medianFinderAddNum(mf1, 2);
    printf("Median: %.1f\n", medianFinderFindMedian(mf1)); // Output: 1.5
    medianFinderAddNum(mf1, 3);
    printf("Median: %.1f\n", medianFinderFindMedian(mf1)); // Output: 2.0
    medianFinderDestroy(mf1);
    printf("\n");

    // Test Case 2: Negative numbers
    printf("Test Case 2:\n");
    MedianFinder* mf2 = medianFinderCreate();
    medianFinderAddNum(mf2, -1);
    medianFinderAddNum(mf2, -2);
    printf("Median: %.1f\n", medianFinderFindMedian(mf2)); // Output: -1.5
    medianFinderAddNum(mf2, -3);
    printf("Median: %.1f\n", medianFinderFindMedian(mf2)); // Output: -2.0
    medianFinderDestroy(mf2);
    printf("\n");

    // Test Case 3: Mixed numbers and duplicates
    printf("Test Case 3:\n");
    MedianFinder* mf3 = medianFinderCreate();
    medianFinderAddNum(mf3, 6);
    printf("Median: %.1f\n", medianFinderFindMedian(mf3)); // Output: 6.0
    medianFinderAddNum(mf3, 10);
    printf("Median: %.1f\n", medianFinderFindMedian(mf3)); // Output: 8.0
    medianFinderAddNum(mf3, 2);
    printf("Median: %.1f\n", medianFinderFindMedian(mf3)); // Output: 6.0
    medianFinderAddNum(mf3, 6);
    printf("Median: %.1f\n", medianFinderFindMedian(mf3)); // Output: 6.0
    medianFinderDestroy(mf3);
    printf("\n");

    // Test Case 4: Zeros
    printf("Test Case 4:\n");
    MedianFinder* mf4 = medianFinderCreate();
    medianFinderAddNum(mf4, 0);
    printf("Median: %.1f\n", medianFinderFindMedian(mf4)); // Output: 0.0
    medianFinderAddNum(mf4, 0);
    printf("Median: %.1f\n", medianFinderFindMedian(mf4)); // Output: 0.0
    medianFinderDestroy(mf4);
    printf("\n");

    // Test Case 5: Descending order
    printf("Test Case 5:\n");
    MedianFinder* mf5 = medianFinderCreate();
    medianFinderAddNum(mf5, 5);
    medianFinderAddNum(mf5, 4);
    medianFinderAddNum(mf5, 3);
    medianFinderAddNum(mf5, 2);
    medianFinderAddNum(mf5, 1);
    printf("Median: %.1f\n", medianFinderFindMedian(mf5)); // Output: 3.0
    medianFinderDestroy(mf5);

    return 0;
}