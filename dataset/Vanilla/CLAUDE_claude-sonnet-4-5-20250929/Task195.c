
#include <stdio.h>
#include <stdlib.h>

// C does not have built-in heap/priority queue data structures
// This implementation uses dynamic arrays and manual heap operations

typedef struct {
    int* maxHeap;
    int maxHeapSize;
    int maxHeapCapacity;
    int* minHeap;
    int minHeapSize;
    int minHeapCapacity;
} MedianFinder;

void maxHeapifyUp(int* heap, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heap[parent] < heap[idx]) {
            int temp = heap[parent];
            heap[parent] = heap[idx];
            heap[idx] = temp;
            idx = parent;
        } else {
            break;
        }
    }
}

void maxHeapifyDown(int* heap, int size, int idx) {
    while (1) {
        int largest = idx;
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        
        if (left < size && heap[left] > heap[largest])
            largest = left;
        if (right < size && heap[right] > heap[largest])
            largest = right;
        
        if (largest != idx) {
            int temp = heap[idx];
            heap[idx] = heap[largest];
            heap[largest] = temp;
            idx = largest;
        } else {
            break;
        }
    }
}

void minHeapifyUp(int* heap, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heap[parent] > heap[idx]) {
            int temp = heap[parent];
            heap[parent] = heap[idx];
            heap[idx] = temp;
            idx = parent;
        } else {
            break;
        }
    }
}

void minHeapifyDown(int* heap, int size, int idx) {
    while (1) {
        int smallest = idx;
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        
        if (left < size && heap[left] < heap[smallest])
            smallest = left;
        if (right < size && heap[right] < heap[smallest])
            smallest = right;
        
        if (smallest != idx) {
            int temp = heap[idx];
            heap[idx] = heap[smallest];
            heap[smallest] = temp;
            idx = smallest;
        } else {
            break;
        }
    }
}

MedianFinder* medianFinderCreate() {
    MedianFinder* obj = (MedianFinder*)malloc(sizeof(MedianFinder));
    obj->maxHeapCapacity = 1000;
    obj->minHeapCapacity = 1000;
    obj->maxHeap = (int*)malloc(obj->maxHeapCapacity * sizeof(int));
    obj->minHeap = (int*)malloc(obj->minHeapCapacity * sizeof(int));
    obj->maxHeapSize = 0;
    obj->minHeapSize = 0;
    return obj;
}

void medianFinderAddNum(MedianFinder* obj, int num) {
    if (obj->maxHeapSize == 0 || num <= obj->maxHeap[0]) {
        if (obj->maxHeapSize == obj->maxHeapCapacity) {
            obj->maxHeapCapacity *= 2;
            obj->maxHeap = (int*)realloc(obj->maxHeap, obj->maxHeapCapacity * sizeof(int));
        }
        obj->maxHeap[obj->maxHeapSize] = num;
        maxHeapifyUp(obj->maxHeap, obj->maxHeapSize);
        obj->maxHeapSize++;
    } else {
        if (obj->minHeapSize == obj->minHeapCapacity) {
            obj->minHeapCapacity *= 2;
            obj->minHeap = (int*)realloc(obj->minHeap, obj->minHeapCapacity * sizeof(int));
        }
        obj->minHeap[obj->minHeapSize] = num;
        minHeapifyUp(obj->minHeap, obj->minHeapSize);
        obj->minHeapSize++;
    }
    
    // Balance heaps
    if (obj->maxHeapSize > obj->minHeapSize + 1) {
        if (obj->minHeapSize == obj->minHeapCapacity) {
            obj->minHeapCapacity *= 2;
            obj->minHeap = (int*)realloc(obj->minHeap, obj->minHeapCapacity * sizeof(int));
        }
        obj->minHeap[obj->minHeapSize] = obj->maxHeap[0];
        minHeapifyUp(obj->minHeap, obj->minHeapSize);
        obj->minHeapSize++;
        
        obj->maxHeap[0] = obj->maxHeap[obj->maxHeapSize - 1];
        obj->maxHeapSize--;
        maxHeapifyDown(obj->maxHeap, obj->maxHeapSize, 0);
    } else if (obj->minHeapSize > obj->maxHeapSize) {
        if (obj->maxHeapSize == obj->maxHeapCapacity) {
            obj->maxHeapCapacity *= 2;
            obj->maxHeap = (int*)realloc(obj->maxHeap, obj->maxHeapCapacity * sizeof(int));
        }
        obj->maxHeap[obj->maxHeapSize] = obj->minHeap[0];
        maxHeapifyUp(obj->maxHeap, obj->maxHeapSize);
        obj->maxHeapSize++;
        
        obj->minHeap[0] = obj->minHeap[obj->minHeapSize - 1];
        obj->minHeapSize--;
        minHeapifyDown(obj->minHeap, obj->minHeapSize, 0);
    }
}

double medianFinderFindMedian(MedianFinder* obj) {
    if (obj->maxHeapSize == obj->minHeapSize) {
        return (obj->maxHeap[0] + obj->minHeap[0]) / 2.0;
    } else {
        return obj->maxHeap[0];
    }
}

void medianFinderFree(MedianFinder* obj) {
    free(obj->maxHeap);
    free(obj->minHeap);
    free(obj);
}

int main() {
    // Test case 1
    MedianFinder* mf1 = medianFinderCreate();
    medianFinderAddNum(mf1, 1);
    medianFinderAddNum(mf1, 2);
    printf("Test 1: %f\\n", medianFinderFindMedian(mf1)); // 1.5
    medianFinderAddNum(mf1, 3);
    printf("Test 1: %f\\n", medianFinderFindMedian(mf1)); // 2.0
    medianFinderFree(mf1);

    // Test case 2
    MedianFinder* mf2 = medianFinderCreate();
    medianFinderAddNum(mf2, 5);
    printf("Test 2: %f\\n", medianFinderFindMedian(mf2)); // 5.0
    medianFinderFree(mf2);

    // Test case 3
    MedianFinder* mf3 = medianFinderCreate();
    medianFinderAddNum(mf3, 10);
    medianFinderAddNum(mf3, 20);
    medianFinderAddNum(mf3, 30);
    medianFinderAddNum(mf3, 40);
    printf("Test 3: %f\\n", medianFinderFindMedian(mf3)); // 25.0
    medianFinderFree(mf3);

    // Test case 4
    MedianFinder* mf4 = medianFinderCreate();
    medianFinderAddNum(mf4, -1);
    medianFinderAddNum(mf4, -2);
    medianFinderAddNum(mf4, -3);
    printf("Test 4: %f\\n", medianFinderFindMedian(mf4)); // -2.0
    medianFinderFree(mf4);

    // Test case 5
    MedianFinder* mf5 = medianFinderCreate();
    medianFinderAddNum(mf5, 6);
    medianFinderAddNum(mf5, 10);
    medianFinderAddNum(mf5, 2);
    medianFinderAddNum(mf5, 6);
    medianFinderAddNum(mf5, 5);
    printf("Test 5: %f\\n", medianFinderFindMedian(mf5)); // 6.0
    medianFinderFree(mf5);

    return 0;
}
