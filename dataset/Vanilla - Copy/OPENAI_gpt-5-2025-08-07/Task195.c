#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* maxHeap; int maxSize; int maxCap; // max-heap (lower half)
    int* minHeap; int minSize; int minCap; // min-heap (upper half)
} MedianFinder;

static void ensureCapacityInt(int** arr, int* cap, int needed) {
    if (needed <= *cap) return;
    int newCap = (*cap == 0) ? 4 : *cap;
    while (newCap < needed) newCap <<= 1;
    int* tmp = (int*)realloc(*arr, newCap * sizeof(int));
    if (!tmp) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    *arr = tmp;
    *cap = newCap;
}

static void maxHeapPush(MedianFinder* obj, int x) {
    ensureCapacityInt(&obj->maxHeap, &obj->maxCap, obj->maxSize + 1);
    int i = obj->maxSize++;
    while (i > 0) {
        int p = (i - 1) >> 1;
        if (obj->maxHeap[p] >= x) break;
        obj->maxHeap[i] = obj->maxHeap[p];
        i = p;
    }
    obj->maxHeap[i] = x;
}

static int maxHeapTop(MedianFinder* obj) {
    return obj->maxSize ? obj->maxHeap[0] : 0;
}

static int maxHeapPop(MedianFinder* obj) {
    int ret = obj->maxHeap[0];
    int last = obj->maxHeap[--obj->maxSize];
    if (obj->maxSize > 0) {
        int i = 0;
        while (1) {
            int l = (i << 1) + 1;
            int r = l + 1;
            if (l >= obj->maxSize) break;
            int largest = l;
            if (r < obj->maxSize && obj->maxHeap[r] > obj->maxHeap[l]) largest = r;
            if (last >= obj->maxHeap[largest]) break;
            obj->maxHeap[i] = obj->maxHeap[largest];
            i = largest;
        }
        obj->maxHeap[i] = last;
    }
    return ret;
}

static void minHeapPush(MedianFinder* obj, int x) {
    ensureCapacityInt(&obj->minHeap, &obj->minCap, obj->minSize + 1);
    int i = obj->minSize++;
    while (i > 0) {
        int p = (i - 1) >> 1;
        if (obj->minHeap[p] <= x) break;
        obj->minHeap[i] = obj->minHeap[p];
        i = p;
    }
    obj->minHeap[i] = x;
}

static int minHeapTop(MedianFinder* obj) {
    return obj->minSize ? obj->minHeap[0] : 0;
}

static int minHeapPop(MedianFinder* obj) {
    int ret = obj->minHeap[0];
    int last = obj->minHeap[--obj->minSize];
    if (obj->minSize > 0) {
        int i = 0;
        while (1) {
            int l = (i << 1) + 1;
            int r = l + 1;
            if (l >= obj->minSize) break;
            int smallest = l;
            if (r < obj->minSize && obj->minHeap[r] < obj->minHeap[l]) smallest = r;
            if (last <= obj->minHeap[smallest]) break;
            obj->minHeap[i] = obj->minHeap[smallest];
            i = smallest;
        }
        obj->minHeap[i] = last;
    }
    return ret;
}

MedianFinder* medianFinderCreate() {
    MedianFinder* obj = (MedianFinder*)calloc(1, sizeof(MedianFinder));
    return obj;
}

void medianFinderAddNum(MedianFinder* obj, int num) {
    if (obj->maxSize == 0 || num <= maxHeapTop(obj)) {
        maxHeapPush(obj, num);
    } else {
        minHeapPush(obj, num);
    }

    // Balance: maxSize >= minSize and diff <= 1
    if (obj->maxSize < obj->minSize) {
        int moved = minHeapPop(obj);
        maxHeapPush(obj, moved);
    } else if (obj->maxSize - obj->minSize > 1) {
        int moved = maxHeapPop(obj);
        minHeapPush(obj, moved);
    }
}

double medianFinderFindMedian(MedianFinder* obj) {
    if (obj->maxSize == obj->minSize) {
        return ((double)maxHeapTop(obj) + (double)minHeapTop(obj)) / 2.0;
    }
    return (double)maxHeapTop(obj);
}

void medianFinderFree(MedianFinder* obj) {
    if (!obj) return;
    free(obj->maxHeap);
    free(obj->minHeap);
    free(obj);
}

int main() {
    // Test case 1 (Example)
    MedianFinder* mf1 = medianFinderCreate();
    medianFinderAddNum(mf1, 1);
    medianFinderAddNum(mf1, 2);
    printf("%.6f\n", medianFinderFindMedian(mf1)); // 1.5
    medianFinderAddNum(mf1, 3);
    printf("%.6f\n", medianFinderFindMedian(mf1)); // 2.0
    medianFinderFree(mf1);

    // Test case 2
    MedianFinder* mf2 = medianFinderCreate();
    medianFinderAddNum(mf2, 2);
    medianFinderAddNum(mf2, 3);
    medianFinderAddNum(mf2, 4);
    printf("%.6f\n", medianFinderFindMedian(mf2)); // 3.0
    medianFinderFree(mf2);

    // Test case 3
    MedianFinder* mf3 = medianFinderCreate();
    medianFinderAddNum(mf3, 2);
    medianFinderAddNum(mf3, 3);
    printf("%.6f\n", medianFinderFindMedian(mf3)); // 2.5
    medianFinderFree(mf3);

    // Test case 4 (negatives)
    MedianFinder* mf4 = medianFinderCreate();
    medianFinderAddNum(mf4, -1);
    medianFinderAddNum(mf4, -2);
    medianFinderAddNum(mf4, -3);
    medianFinderAddNum(mf4, -4);
    medianFinderAddNum(mf4, -5);
    printf("%.6f\n", medianFinderFindMedian(mf4)); // -3.0
    medianFinderFree(mf4);

    // Test case 5 (dynamic)
    MedianFinder* mf5 = medianFinderCreate();
    medianFinderAddNum(mf5, 5);
    printf("%.6f\n", medianFinderFindMedian(mf5)); // 5.0
    medianFinderAddNum(mf5, 15);
    printf("%.6f\n", medianFinderFindMedian(mf5)); // 10.0
    medianFinderAddNum(mf5, 1);
    printf("%.6f\n", medianFinderFindMedian(mf5)); // 5.0
    medianFinderAddNum(mf5, 3);
    printf("%.6f\n", medianFinderFindMedian(mf5)); // 4.0
    medianFinderFree(mf5);

    return 0;
}