
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_SIZE 50001

typedef struct {
    int* maxHeap;
    int maxHeapSize;
    int maxHeapCapacity;
    int* minHeap;
    int minHeapSize;
    int minHeapCapacity;
} MedianFinder;

void maxHeapify(int* heap, int size, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    if (left < size && heap[left] > heap[largest])
        largest = left;
    if (right < size && heap[right] > heap[largest])
        largest = right;
    
    if (largest != i) {
        int temp = heap[i];
        heap[i] = heap[largest];
        heap[largest] = temp;
        maxHeapify(heap, size, largest);
    }
}

void minHeapify(int* heap, int size, int i) {
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    if (left < size && heap[left] < heap[smallest])
        smallest = left;
    if (right < size && heap[right] < heap[smallest])
        smallest = right;
    
    if (smallest != i) {
        int temp = heap[i];
        heap[i] = heap[smallest];
        heap[smallest] = temp;
        minHeapify(heap, size, smallest);
    }
}

MedianFinder* medianFinderCreate() {
    MedianFinder* obj = (MedianFinder*)malloc(sizeof(MedianFinder));
    if (!obj) return NULL;
    
    obj->maxHeapCapacity = MAX_SIZE;
    obj->minHeapCapacity = MAX_SIZE;
    obj->maxHeap = (int*)malloc(obj->maxHeapCapacity * sizeof(int));
    obj->minHeap = (int*)malloc(obj->minHeapCapacity * sizeof(int));
    obj->maxHeapSize = 0;
    obj->minHeapSize = 0;
    
    return obj;
}

void medianFinderAddNum(MedianFinder* obj, int num) {
    if (!obj || num < -100000 || num > 100000) return;
    
    // Add to max heap
    obj->maxHeap[obj->maxHeapSize++] = num;
    for (int i = obj->maxHeapSize / 2 - 1; i >= 0; i--) {
        maxHeapify(obj->maxHeap, obj->maxHeapSize, i);
    }
    
    // Move max element to min heap
    if (obj->maxHeapSize > 0) {
        obj->minHeap[obj->minHeapSize++] = obj->maxHeap[0];
        obj->maxHeap[0] = obj->maxHeap[--obj->maxHeapSize];
        if (obj->maxHeapSize > 0) {
            maxHeapify(obj->maxHeap, obj->maxHeapSize, 0);
        }
        
        for (int i = obj->minHeapSize / 2 - 1; i >= 0; i--) {
            minHeapify(obj->minHeap, obj->minHeapSize, i);
        }
    }
    
    // Rebalance
    if (obj->minHeapSize > obj->maxHeapSize) {
        obj->maxHeap[obj->maxHeapSize++] = obj->minHeap[0];
        obj->minHeap[0] = obj->minHeap[--obj->minHeapSize];
        if (obj->minHeapSize > 0) {
            minHeapify(obj->minHeap, obj->minHeapSize, 0);
        }
        
        for (int i = obj->maxHeapSize / 2 - 1; i >= 0; i--) {
            maxHeapify(obj->maxHeap, obj->maxHeapSize, i);
        }
    }
}

double medianFinderFindMedian(MedianFinder* obj) {
    if (!obj || (obj->maxHeapSize == 0 && obj->minHeapSize == 0)) {
        return 0.0;
    }
    
    if (obj->maxHeapSize > obj->minHeapSize) {
        return obj->maxHeap[0];
    } else {
        return (obj->maxHeap[0] + obj->minHeap[0]) / 2.0;
    }
}

void medianFinderFree(MedianFinder* obj) {
    if (obj) {
        free(obj->maxHeap);
        free(obj->minHeap);
        free(obj);
    }
}

int main() {
    // Test case 1
    MedianFinder* mf1 = medianFinderCreate();
    medianFinderAddNum(mf1, 1);
    medianFinderAddNum(mf1, 2);
    printf("Test 1: %.1f\\n", medianFinderFindMedian(mf1));
    medianFinderAddNum(mf1, 3);
    printf("Test 1: %.1f\\n", medianFinderFindMedian(mf1));
    medianFinderFree(mf1);
    
    // Test case 2
    MedianFinder* mf2 = medianFinderCreate();
    medianFinderAddNum(mf2, 5);
    printf("Test 2: %.1f\\n", medianFinderFindMedian(mf2));
    medianFinderFree(mf2);
    
    // Test case 3
    MedianFinder* mf3 = medianFinderCreate();
    medianFinderAddNum(mf3, -1);
    medianFinderAddNum(mf3, -2);
    medianFinderAddNum(mf3, -3);
    printf("Test 3: %.1f\\n", medianFinderFindMedian(mf3));
    medianFinderFree(mf3);
    
    // Test case 4
    MedianFinder* mf4 = medianFinderCreate();
    medianFinderAddNum(mf4, -5);
    medianFinderAddNum(mf4, 10);
    printf("Test 4: %.1f\\n", medianFinderFindMedian(mf4));
    medianFinderFree(mf4);
    
    // Test case 5
    MedianFinder* mf5 = medianFinderCreate();
    medianFinderAddNum(mf5, 12);
    medianFinderAddNum(mf5, 10);
    medianFinderAddNum(mf5, 13);
    medianFinderAddNum(mf5, 11);
    medianFinderAddNum(mf5, 5);
    medianFinderAddNum(mf5, 15);
    printf("Test 5: %.1f\\n", medianFinderFindMedian(mf5));
    medianFinderFree(mf5);
    
    return 0;
}
