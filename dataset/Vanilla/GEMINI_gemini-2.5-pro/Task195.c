#include <stdio.h>
#include <stdlib.h>

// --- Heap Data Structure Implementation ---

typedef enum { MIN_HEAP, MAX_HEAP } HeapType;

typedef struct {
    int* data;
    int size;
    int capacity;
    HeapType type;
} Heap;

// Helper functions for heap
static int parent(int i) { return (i - 1) / 2; }
static int left(int i) { return 2 * i + 1; }
static int right(int i) { return 2 * i + 2; }

static void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

static int compare(Heap* h, int a, int b) {
    return (h->type == MAX_HEAP) ? (a > b) : (a < b);
}

static void heapifyUp(Heap* h, int i) {
    while (i != 0 && compare(h, h->data[i], h->data[parent(i)])) {
        swap(&h->data[i], &h->data[parent(i)]);
        i = parent(i);
    }
}

static void heapifyDown(Heap* h, int i) {
    int l = left(i);
    int r = right(i);
    int target = i;
    if (l < h->size && compare(h, h->data[l], h->data[target])) {
        target = l;
    }
    if (r < h->size && compare(h, h->data[r], h->data[target])) {
        target = r;
    }
    if (target != i) {
        swap(&h->data[i], &h->data[target]);
        heapifyDown(h, target);
    }
}

static Heap* heapCreate(int capacity, HeapType type) {
    Heap* h = (Heap*)malloc(sizeof(Heap));
    h->data = (int*)malloc(capacity * sizeof(int));
    h->size = 0;
    h->capacity = capacity;
    h->type = type;
    return h;
}

static void heapPush(Heap* h, int value) {
    if (h->size == h->capacity) return;
    h->data[h->size] = value;
    h->size++;
    heapifyUp(h, h->size - 1);
}

static int heapPop(Heap* h) {
    if (h->size <= 0) return -1; // Should not happen
    int root = h->data[0];
    h->data[0] = h->data[h->size - 1];
    h->size--;
    heapifyDown(h, 0);
    return root;
}

static int heapPeek(Heap* h) {
    if (h->size <= 0) return -1; // Should not happen
    return h->data[0];
}

static void heapFree(Heap* h) {
    if (h) {
        free(h->data);
        free(h);
    }
}

// --- MedianFinder Implementation (emulating a class) ---

typedef struct {
    Heap* smallHalf; // Max heap
    Heap* largeHalf; // Min heap
} MedianFinder;

MedianFinder* medianFinderCreate() {
    MedianFinder* mf = (MedianFinder*)malloc(sizeof(MedianFinder));
    if (!mf) return NULL;
    // Constraint: At most 5 * 10^4 calls. Each heap needs about half + 1.
    mf->smallHalf = heapCreate(25001, MAX_HEAP);
    mf->largeHalf = heapCreate(25001, MIN_HEAP);
    return mf;
}

void medianFinderAddNum(MedianFinder* obj, int num) {
    heapPush(obj->smallHalf, num);
    
    int val = heapPop(obj->smallHalf);
    heapPush(obj->largeHalf, val);
    
    if (obj->smallHalf->size < obj->largeHalf->size) {
        val = heapPop(obj->largeHalf);
        heapPush(obj->smallHalf, val);
    }
}

double medianFinderFindMedian(MedianFinder* obj) {
    if (obj->smallHalf->size > obj->largeHalf->size) {
        return (double)heapPeek(obj->smallHalf);
    } else {
        return ((double)heapPeek(obj->smallHalf) + (double)heapPeek(obj->largeHalf)) / 2.0;
    }
}

void medianFinderFree(MedianFinder* obj) {
    if (obj) {
        heapFree(obj->smallHalf);
        heapFree(obj->largeHalf);
        free(obj);
    }
}

// C does not have a concept of a single main class named Task195.
// The main function serves as the entry point for the program.
int main() {
    // Test Case 1: Example from description
    printf("Test Case 1:\n");
    MedianFinder* mf1 = medianFinderCreate();
    medianFinderAddNum(mf1, 1);
    medianFinderAddNum(mf1, 2);
    printf("Median: %f\n", medianFinderFindMedian(mf1)); // Expected: 1.5
    medianFinderAddNum(mf1, 3);
    printf("Median: %f\n", medianFinderFindMedian(mf1)); // Expected: 2.0
    medianFinderFree(mf1);
    printf("\n");

    // Test Case 2: Negative numbers
    printf("Test Case 2:\n");
    MedianFinder* mf2 = medianFinderCreate();
    medianFinderAddNum(mf2, -1);
    printf("Median: %f\n", medianFinderFindMedian(mf2)); // Expected: -1.0
    medianFinderAddNum(mf2, -2);
    printf("Median: %f\n", medianFinderFindMedian(mf2)); // Expected: -1.5
    medianFinderAddNum(mf2, -3);
    printf("Median: %f\n", medianFinderFindMedian(mf2)); // Expected: -2.0
    medianFinderFree(mf2);
    printf("\n");
    
    // Test Case 3: Zero
    printf("Test Case 3:\n");
    MedianFinder* mf3 = medianFinderCreate();
    medianFinderAddNum(mf3, 0);
    printf("Median: %f\n", medianFinderFindMedian(mf3)); // Expected: 0.0
    medianFinderAddNum(mf3, 0);
    printf("Median: %f\n", medianFinderFindMedian(mf3)); // Expected: 0.0
    medianFinderFree(mf3);
    printf("\n");

    // Test Case 4: Alternating large and small numbers
    printf("Test Case 4:\n");
    MedianFinder* mf4 = medianFinderCreate();
    medianFinderAddNum(mf4, 10);
    medianFinderAddNum(mf4, 1);
    medianFinderAddNum(mf4, 100);
    medianFinderAddNum(mf4, 5);
    // Nums: [1, 5, 10, 100], Median: (5+10)/2 = 7.5
    printf("Median: %f\n", medianFinderFindMedian(mf4));
    medianFinderFree(mf4);
    printf("\n");

    // Test Case 5: Descending order
    printf("Test Case 5:\n");
    MedianFinder* mf5 = medianFinderCreate();
    medianFinderAddNum(mf5, 5);
    medianFinderAddNum(mf5, 4);
    medianFinderAddNum(mf5, 3);
    medianFinderAddNum(mf5, 2);
    medianFinderAddNum(mf5, 1);
    // Nums: [1, 2, 3, 4, 5], Median: 3
    printf("Median: %f\n", medianFinderFindMedian(mf5));
    medianFinderFree(mf5);
    printf("\n");

    return 0;
}