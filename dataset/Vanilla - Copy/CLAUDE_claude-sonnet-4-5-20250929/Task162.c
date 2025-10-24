
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

typedef struct Edge {
    int target;
    int weight;
    struct Edge* next;
} Edge;

typedef struct {
    int vertex;
    int distance;
} HeapNode;

typedef struct {
    HeapNode* array;
    int size;
    int capacity;
} MinHeap;

MinHeap* createMinHeap(int capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->array = (HeapNode*)malloc(capacity * sizeof(HeapNode));
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

void swap(HeapNode* a, HeapNode* b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void minHeapify(MinHeap* heap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    
    if (left < heap->size && heap->array[left].distance < heap->array[smallest].distance)
        smallest = left;
    if (right < heap->size && heap->array[right].distance < heap->array[smallest].distance)
        smallest = right;
    
    if (smallest != idx) {
        swap(&heap->array[idx], &heap->array[smallest]);
        minHeapify(heap, smallest);
    }
}

HeapNode extractMin(MinHeap* heap) {
    HeapNode root = heap->array[0];
    heap->array[0] = heap->array[heap->size - 1];
    heap->size--;
    minHeapify(heap, 0);
    return root;
}

void insertHeap(MinHeap* heap, int vertex, int distance) {
    heap->size++;
    int i = heap->size - 1;
    heap->array[i].vertex = vertex;
    heap->array[i].distance = distance;
    
    while (i > 0 && heap->array[(i - 1) / 2].distance > heap->array[i].distance) {
        swap(&heap->array[i], &heap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void addEdge(Edge** graph, int src, int dest, int weight) {
    Edge* newEdge = (Edge*)malloc(sizeof(Edge));
    newEdge->target = dest;
    newEdge->weight = weight;
    newEdge->next = graph[src];
    graph[src] = newEdge;
}

int* dijkstra(Edge** graph, int start, int n) {
    int* distances = (int*)malloc(n * sizeof(int));
    bool* visited = (bool*)malloc(n * sizeof(bool));
    
    for (int i = 0; i < n; i++) {
        distances[i] = INT_MAX;
        visited[i] = false;
    }
    distances[start] = 0;
    
    MinHeap* heap = createMinHeap(n * 100);
    insertHeap(heap, start, 0);
    
    while (heap->size > 0) {
        HeapNode current = extractMin(heap);
        int u = current.vertex;
        
        if (visited[u]) continue;
        visited[u] = true;
        
        Edge* edge = graph[u];
        while (edge != NULL) {
            int v = edge->target;
            int weight = edge->weight;
            
            if (!visited[v] && distances[u] != INT_MAX && 
                distances[u] + weight < distances[v]) {
                distances[v] = distances[u] + weight;
                insertHeap(heap, v, distances[v]);
            }
            edge = edge->next;
        }
    }
    
    free(visited);
    free(heap->array);
    free(heap);
    return distances;
}

int main() {
    // Test Case 1: Simple graph
    int n1 = 5;
    Edge** graph1 = (Edge**)calloc(n1, sizeof(Edge*));
    addEdge(graph1, 0, 1, 4);
    addEdge(graph1, 0, 2, 1);
    addEdge(graph1, 2, 1, 2);
    addEdge(graph1, 1, 3, 1);
    addEdge(graph1, 2, 3, 5);
    addEdge(graph1, 3, 4, 3);
    
    printf("Test Case 1:\\n");
    printf("Start node: 0\\n");
    printf("Shortest distances: ");
    int* result1 = dijkstra(graph1, 0, n1);
    for (int i = 0; i < n1; i++) {
        if (result1[i] == INT_MAX) printf("INF ");
        else printf("%d ", result1[i]);
    }
    printf("\\n");
    free(result1);
    
    // Test Case 2: Disconnected node
    int n2 = 4;
    Edge** graph2 = (Edge**)calloc(n2, sizeof(Edge*));
    addEdge(graph2, 0, 1, 5);
    addEdge(graph2, 1, 2, 3);
    
    printf("\\nTest Case 2:\\n");
    printf("Start node: 0\\n");
    printf("Shortest distances: ");
    int* result2 = dijkstra(graph2, 0, n2);
    for (int i = 0; i < n2; i++) {
        if (result2[i] == INT_MAX) printf("INF ");
        else printf("%d ", result2[i]);
    }
    printf("\\n");
    free(result2);
    
    // Test Case 3: Complete graph
    int n3 = 3;
    Edge** graph3 = (Edge**)calloc(n3, sizeof(Edge*));
    addEdge(graph3, 0, 1, 2);
    addEdge(graph3, 0, 2, 5);
    addEdge(graph3, 1, 2, 1);
    
    printf("\\nTest Case 3:\\n");
    printf("Start node: 0\\n");
    printf("Shortest distances: ");
    int* result3 = dijkstra(graph3, 0, n3);
    for (int i = 0; i < n3; i++) {
        if (result3[i] == INT_MAX) printf("INF ");
        else printf("%d ", result3[i]);
    }
    printf("\\n");
    free(result3);
    
    // Test Case 4: Single node
    int n4 = 1;
    Edge** graph4 = (Edge**)calloc(n4, sizeof(Edge*));
    
    printf("\\nTest Case 4:\\n");
    printf("Start node: 0\\n");
    printf("Shortest distances: ");
    int* result4 = dijkstra(graph4, 0, n4);
    for (int i = 0; i < n4; i++) {
        if (result4[i] == INT_MAX) printf("INF ");
        else printf("%d ", result4[i]);
    }
    printf("\\n");
    free(result4);
    
    // Test Case 5: Multiple paths
    int n5 = 6;
    Edge** graph5 = (Edge**)calloc(n5, sizeof(Edge*));
    addEdge(graph5, 0, 1, 7);
    addEdge(graph5, 0, 2, 9);
    addEdge(graph5, 0, 5, 14);
    addEdge(graph5, 1, 2, 10);
    addEdge(graph5, 1, 3, 15);
    addEdge(graph5, 2, 3, 11);
    addEdge(graph5, 2, 5, 2);
    addEdge(graph5, 3, 4, 6);
    addEdge(graph5, 4, 5, 9);
    
    printf("\\nTest Case 5:\\n");
    printf("Start node: 0\\n");
    printf("Shortest distances: ");
    int* result5 = dijkstra(graph5, 0, n5);
    for (int i = 0; i < n5; i++) {
        if (result5[i] == INT_MAX) printf("INF ");
        else printf("%d ", result5[i]);
    }
    printf("\\n");
    free(result5);
    
    return 0;
}
