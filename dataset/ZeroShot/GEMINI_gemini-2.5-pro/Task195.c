#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// --- Heap Implementation ---
#define MAX_HEAP 1
#define MIN_HEAP 0

typedef struct {
    int* data;
    size_t size;
    size_t capacity;
    int type; // MAX_HEAP or MIN_HEAP
} Heap;

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

Heap* createHeap(size_t capacity, int type) {
    Heap* h = (Heap*)malloc(sizeof(Heap));
    if (!h) return NULL;
    h->data = (int*)malloc(capacity * sizeof(int));
    if (!h->data) {
        free(h);
        return NULL;
    }
    h->size = 0;
    h->capacity = capacity;
    h->type = type;
    return h;
}

void destroyHeap(Heap* h) {
    if (h) {
        free(h->data);
        free(h);
    }
}

void heapifyUp(Heap* h, size_t index) {
    if (index == 0) return;
    size_t parent_index = (index - 1) / 2;
    bool should_swap = (h->type == MAX_HEAP) ? 
                       (h->data[index] > h->data[parent_index]) : 
                       (h->data[index] < h->data[parent_index]);
    if (should_swap) {
        swap(&h->data[index], &h->data[parent_index]);
        heapifyUp(h, parent_index);
    }
}

void heapifyDown(Heap* h, size_t index) {
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;
    size_t target = index;

    if (h->type == MAX_HEAP) {
        if (left < h->size && h->data[left] > h->data[target]) target = left;
        if (right < h->size && h->data[right] > h->data[target]) target = right;
    } else { // MIN_HEAP
        if (left < h->size && h->data[left] < h->data[target]) target = left;
        if (right < h->size && h->data[right] < h->data[target]) target = right;
    }

    if (target != index) {
        swap(&h->data[index], &h->data[target]);
        heapifyDown(h, target);
    }
}

void heapInsert(Heap* h, int value) {
    if (h->size == h->capacity) {
        h->capacity *= 2;
        int* new_data = (int*)realloc(h->data, h->capacity * sizeof(int));
        if (!new_data) {
            // Handle realloc failure
            return;
        }
        h->data = new_data;
    }
    h->data[h->size] = value;
    heapifyUp(h, h->size);
    h->size++;
}

int heapExtract(Heap* h) {
    if (h->size == 0) return -1; // Should not happen based on constraints
    int root = h->data[0];
    h->data[0] = h->data[h->size - 1];
    h->size--;
    heapifyDown(h, 0);
    return root;
}

int heapPeek(Heap* h) {
    if (h->size == 0) return -1; // Should not happen based on constraints
    return h->data[0];
}

// --- MedianFinder Implementation ---
typedef struct {
    Heap* lowerHalf; // Max-heap
    Heap* upperHalf; // Min-heap
} MedianFinder;

MedianFinder* medianFinderCreate() {
    MedianFinder* mf = (MedianFinder*)malloc(sizeof(MedianFinder));
    if (!mf) return NULL;
    // Initial capacity can be small, it will grow as needed
    mf->lowerHalf = createHeap(10, MAX_HEAP);
    mf->upperHalf = createHeap(10, MIN_HEAP);
    if (!mf->lowerHalf || !mf->upperHalf) {
        destroyHeap(mf->lowerHalf);
        destroyHeap(mf->upperHalf);
        free(mf);
        return NULL;
    }
    return mf;
}

void medianFinderDestroy(MedianFinder* obj) {
    if (obj) {
        destroyHeap(obj->lowerHalf);
        destroyHeap(obj->upperHalf);
        free(obj);
    }
}

void medianFinderAddNum(MedianFinder* obj, int num) {
    heapInsert(obj->lowerHalf, num);
    heapInsert(obj->upperHalf, heapExtract(obj->lowerHalf));

    if (obj->upperHalf->size > obj->lowerHalf->size) {
        heapInsert(obj->lowerHalf, heapExtract(obj->upperHalf));
    }
}

double medianFinderFindMedian(MedianFinder* obj) {
    if (obj->lowerHalf->size > obj->upperHalf->size) {
        return (double)heapPeek(obj->lowerHalf);
    } else {
        return ((double)heapPeek(obj->lowerHalf) + heapPeek(obj->upperHalf)) / 2.0;
    }
}

// --- Main function for testing ---
int main() {
    // Test Case 1
    printf("Test Case 1:\n");
    MedianFinder* mf1 = medianFinderCreate();
    medianFinderAddNum(mf1, 1);
    medianFinderAddNum(mf1, 2);
    printf("Median: %.1f\n", medianFinderFindMedian(mf1)); // Output: 1.5
    medianFinderAddNum(mf1, 3);
    printf("Median: %.1f\n", medianFinderFindMedian(mf1)); // Output: 2.0
    medianFinderDestroy(mf1);
    printf("\n");

    // Test Case 2
    printf("Test Case 2:\n");
    MedianFinder* mf2 = medianFinderCreate();
    medianFinderAddNum(mf2, 6);
    printf("Median: %.1f\n", medianFinderFindMedian(mf2)); // Output: 6.0
    medianFinderAddNum(mf2, 10);
    printf("Median: %.1f\n", medianFinderFindMedian(mf2)); // Output: 8.0
    medianFinderDestroy(mf2);
    printf("\n");

    // Test Case 3
    printf("Test Case 3:\n");
    MedianFinder* mf3 = medianFinderCreate();
    medianFinderAddNum(mf3, 1);
    medianFinderAddNum(mf3, 2);
    medianFinderAddNum(mf3, 3);
    printf("Median: %.1f\n", medianFinderFindMedian(mf3)); // Output: 2.0
    medianFinderDestroy(mf3);
    printf("\n");
    
    // Test Case 4
    printf("Test Case 4:\n");
    MedianFinder* mf4 = medianFinderCreate();
    medianFinderAddNum(mf4, -1);
    medianFinderAddNum(mf4, -2);
    printf("Median: %.1f\n", medianFinderFindMedian(mf4)); // Output: -1.5
    medianFinderAddNum(mf4, -3);
    printf("Median: %.1f\n", medianFinderFindMedian(mf4)); // Output: -2.0
    medianFinderAddNum(mf4, -4);
    printf("Median: %.1f\n", medianFinderFindMedian(mf4)); // Output: -2.5
    medianFinderDestroy(mf4);
    printf("\n");

    // Test Case 5
    printf("Test Case 5:\n");
    MedianFinder* mf5 = medianFinderCreate();
    medianFinderAddNum(mf5, 0);
    printf("Median: %.1f\n", medianFinderFindMedian(mf5)); // Output: 0.0
    medianFinderDestroy(mf5);
    printf("\n");
    
    return 0;
}