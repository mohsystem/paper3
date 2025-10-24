#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

// --- Generic Heap Implementation ---

typedef struct {
    int* data;
    size_t size;
    size_t capacity;
    bool is_min_heap;
} Heap;

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

Heap* heap_create(size_t capacity, bool is_min_heap) {
    Heap* h = (Heap*)malloc(sizeof(Heap));
    if (!h) return NULL;
    h->data = (int*)malloc(capacity * sizeof(int));
    if (!h->data) {
        free(h);
        return NULL;
    }
    h->size = 0;
    h->capacity = capacity;
    h->is_min_heap = is_min_heap;
    return h;
}

void heap_destroy(Heap* h) {
    if (h) {
        free(h->data);
        free(h);
    }
}

void heapify_up(Heap* h, size_t i) {
    if (i == 0) return;
    size_t parent = (i - 1) / 2;
    if (h->is_min_heap) {
        if (h->data[i] < h->data[parent]) {
            swap(&h->data[i], &h->data[parent]);
            heapify_up(h, parent);
        }
    } else { // max-heap
        if (h->data[i] > h->data[parent]) {
            swap(&h->data[i], &h->data[parent]);
            heapify_up(h, parent);
        }
    }
}

void heapify_down(Heap* h, size_t i) {
    size_t left = 2 * i + 1;
    size_t right = 2 * i + 2;
    size_t target = i;

    if (h->is_min_heap) {
        if (left < h->size && h->data[left] < h->data[target]) target = left;
        if (right < h->size && h->data[right] < h->data[target]) target = right;
    } else { // max-heap
        if (left < h->size && h->data[left] > h->data[target]) target = left;
        if (right < h->size && h->data[right] > h->data[target]) target = right;
    }

    if (target != i) {
        swap(&h->data[i], &h->data[target]);
        heapify_down(h, target);
    }
}

bool heap_push(Heap* h, int value) {
    if (h->size == h->capacity) {
        size_t new_capacity = (h->capacity == 0) ? 1 : h->capacity * 2;
        int* new_data = (int*)realloc(h->data, new_capacity * sizeof(int));
        if (!new_data) return false;
        h->data = new_data;
        h->capacity = new_capacity;
    }
    h->data[h->size] = value;
    heapify_up(h, h->size);
    h->size++;
    return true;
}

int heap_pop(Heap* h) {
    int root = h->data[0];
    h->size--;
    h->data[0] = h->data[h->size];
    heapify_down(h, 0);
    return root;
}

int heap_peek(const Heap* h) {
    return h->data[0];
}

// --- MedianFinder Implementation ---

typedef struct {
    Heap* lower_half; // max-heap
    Heap* upper_half; // min-heap
} MedianFinder;

MedianFinder* medianFinderCreate() {
    MedianFinder* mf = (MedianFinder*)malloc(sizeof(MedianFinder));
    if (!mf) return NULL;
    mf->lower_half = heap_create(1, false);
    mf->upper_half = heap_create(1, true);
    if (!mf->lower_half || !mf->upper_half) {
        heap_destroy(mf->lower_half);
        heap_destroy(mf->upper_half);
        free(mf);
        return NULL;
    }
    return mf;
}

void medianFinderAddNum(MedianFinder* obj, int num) {
    heap_push(obj->lower_half, num);
    heap_push(obj->upper_half, heap_pop(obj->lower_half));

    if (obj->lower_half->size < obj->upper_half->size) {
        heap_push(obj->lower_half, heap_pop(obj->upper_half));
    }
}

double medianFinderFindMedian(MedianFinder* obj) {
    if (obj->lower_half->size > obj->upper_half->size) {
        return (double)heap_peek(obj->lower_half);
    } else {
        return ((double)heap_peek(obj->lower_half) + (double)heap_peek(obj->upper_half)) / 2.0;
    }
}

void medianFinderFree(MedianFinder* obj) {
    if (obj) {
        heap_destroy(obj->lower_half);
        heap_destroy(obj->upper_half);
        free(obj);
    }
}

// --- Main function with test cases ---

int main() {
    // Test Case 1: Example from prompt
    printf("Test Case 1:\n");
    MedianFinder* mf1 = medianFinderCreate();
    medianFinderAddNum(mf1, 1);
    medianFinderAddNum(mf1, 2);
    printf("Median: %.5f\n", medianFinderFindMedian(mf1)); // Expected: 1.50000
    medianFinderAddNum(mf1, 3);
    printf("Median: %.5f\n", medianFinderFindMedian(mf1)); // Expected: 2.00000
    medianFinderFree(mf1);
    printf("\n");

    // Test Case 2: Negative numbers
    printf("Test Case 2:\n");
    MedianFinder* mf2 = medianFinderCreate();
    medianFinderAddNum(mf2, -1);
    printf("Median: %.5f\n", medianFinderFindMedian(mf2)); // Expected: -1.00000
    medianFinderAddNum(mf2, -2);
    printf("Median: %.5f\n", medianFinderFindMedian(mf2)); // Expected: -1.50000
    medianFinderAddNum(mf2, -3);
    printf("Median: %.5f\n", medianFinderFindMedian(mf2)); // Expected: -2.00000
    medianFinderFree(mf2);
    printf("\n");

    // Test Case 3: Zeros
    printf("Test Case 3:\n");
    MedianFinder* mf3 = medianFinderCreate();
    medianFinderAddNum(mf3, 0);
    medianFinderAddNum(mf3, 0);
    printf("Median: %.5f\n", medianFinderFindMedian(mf3)); // Expected: 0.00000
    medianFinderAddNum(mf3, 0);
    printf("Median: %.5f\n", medianFinderFindMedian(mf3)); // Expected: 0.00000
    medianFinderFree(mf3);
    printf("\n");

    // Test Case 4: Mixed numbers
    printf("Test Case 4:\n");
    MedianFinder* mf4 = medianFinderCreate();
    medianFinderAddNum(mf4, 6);
    printf("Median: %.5f\n", medianFinderFindMedian(mf4)); // Expected: 6.00000
    medianFinderAddNum(mf4, 10);
    printf("Median: %.5f\n", medianFinderFindMedian(mf4)); // Expected: 8.00000
    medianFinderAddNum(mf4, 2);
    printf("Median: %.5f\n", medianFinderFindMedian(mf4)); // Expected: 6.00000
    medianFinderAddNum(mf4, 6);
    printf("Median: %.5f\n", medianFinderFindMedian(mf4)); // Expected: 6.00000
    medianFinderAddNum(mf4, 5);
    printf("Median: %.5f\n", medianFinderFindMedian(mf4)); // Expected: 6.00000
    medianFinderAddNum(mf4, 0);
    printf("Median: %.5f\n", medianFinderFindMedian(mf4)); // Expected: 5.50000
    medianFinderFree(mf4);
    printf("\n");

    // Test Case 5: Descending order
    printf("Test Case 5:\n");
    MedianFinder* mf5 = medianFinderCreate();
    medianFinderAddNum(mf5, 10);
    medianFinderAddNum(mf5, 9);
    medianFinderAddNum(mf5, 8);
    medianFinderAddNum(mf5, 7);
    medianFinderAddNum(mf5, 6);
    printf("Median: %.5f\n", medianFinderFindMedian(mf5)); // Expected: 8.00000
    medianFinderAddNum(mf5, 5);
    printf("Median: %.5f\n", medianFinderFindMedian(mf5)); // Expected: 7.50000
    medianFinderFree(mf5);
    printf("\n");

    return 0;
}