
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define MAX_NODES 100
#define MAX_EDGES 1000

typedef struct {
    int destination;
    int weight;
} Edge;

typedef struct {
    Edge edges[MAX_EDGES];
    int edge_count;
} AdjList;

typedef struct {
    int vertex;
    int distance;
} HeapNode;

typedef struct {
    HeapNode nodes[MAX_NODES * MAX_EDGES];
    int size;
} MinHeap;

void swap(HeapNode* a, HeapNode* b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void minHeapify(MinHeap* heap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    
    if (left < heap->size && heap->nodes[left].distance < heap->nodes[smallest].distance)
        smallest = left;
    if (right < heap->size && heap->nodes[right].distance < heap->nodes[smallest].distance)
        smallest = right;
    
    if (smallest != idx) {
        swap(&heap->nodes[idx], &heap->nodes[smallest]);
        minHeapify(heap, smallest);
    }
}

void insertHeap(MinHeap* heap, int vertex, int distance) {
    if (heap->size >= MAX_NODES * MAX_EDGES) return;
    
    int i = heap->size++;
    heap->nodes[i].vertex = vertex;
    heap->nodes[i].distance = distance;
    
    while (i > 0 && heap->nodes[(i - 1) / 2].distance > heap->nodes[i].distance) {
        swap(&heap->nodes[i], &heap->nodes[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

HeapNode extractMin(MinHeap* heap) {
    HeapNode minNode = heap->nodes[0];
    heap->nodes[0] = heap->nodes[--heap->size];
    minHeapify(heap, 0);
    return minNode;
}

int isHeapEmpty(MinHeap* heap) {
    return heap->size == 0;
}

void dijkstra(AdjList graph[], int num_nodes, int start, int distances[]) {
    if (start < 0 || start >= num_nodes) return;
    
    int visited[MAX_NODES] = {0};
    MinHeap heap = {.size = 0};
    
    for (int i = 0; i < num_nodes; i++) {
        distances[i] = INT_MAX;
    }
    distances[start] = 0;
    insertHeap(&heap, start, 0);
    
    while (!isHeapEmpty(&heap)) {
        HeapNode current = extractMin(&heap);
        int currentVertex = current.vertex;
        
        if (visited[currentVertex]) continue;
        visited[currentVertex] = 1;
        
        for (int i = 0; i < graph[currentVertex].edge_count; i++) {
            Edge edge = graph[currentVertex].edges[i];
            
            if (edge.weight < 0) continue;
            
            int neighbor = edge.destination;
            if (neighbor < 0 || neighbor >= num_nodes) continue;
            
            if (distances[currentVertex] != INT_MAX &&
                distances[currentVertex] + edge.weight < INT_MAX) {
                int newDistance = distances[currentVertex] + edge.weight;
                
                if (newDistance < distances[neighbor]) {
                    distances[neighbor] = newDistance;
                    insertHeap(&heap, neighbor, newDistance);
                }
            }
        }
    }
}

int main() {
    // Test Case 1: Simple graph
    AdjList graph1[4];
    memset(graph1, 0, sizeof(graph1));
    graph1[0].edges[0] = (Edge){1, 4};
    graph1[0].edges[1] = (Edge){2, 1};
    graph1[0].edge_count = 2;
    graph1[1].edges[0] = (Edge){3, 1};
    graph1[1].edge_count = 1;
    graph1[2].edges[0] = (Edge){1, 2};
    graph1[2].edges[1] = (Edge){3, 5};
    graph1[2].edge_count = 2;
    graph1[3].edge_count = 0;
    
    int distances1[4];
    dijkstra(graph1, 4, 0, distances1);
    printf("Test Case 1:\\n");
    for (int i = 0; i < 4; i++) {
        printf("%d: %d\\n", i, distances1[i]);
    }
    
    // Test Case 2: Disconnected graph
    AdjList graph2[4];
    memset(graph2, 0, sizeof(graph2));
    graph2[0].edges[0] = (Edge){1, 2};
    graph2[0].edge_count = 1;
    graph2[1].edge_count = 0;
    graph2[2].edges[0] = (Edge){3, 1};
    graph2[2].edge_count = 1;
    graph2[3].edge_count = 0;
    
    int distances2[4];
    dijkstra(graph2, 4, 0, distances2);
    printf("\\nTest Case 2:\\n");
    for (int i = 0; i < 4; i++) {
        printf("%d: %d\\n", i, distances2[i]);
    }
    
    // Test Case 3: Single node
    AdjList graph3[1];
    memset(graph3, 0, sizeof(graph3));
    graph3[0].edge_count = 0;
    
    int distances3[1];
    dijkstra(graph3, 1, 0, distances3);
    printf("\\nTest Case 3:\\n");
    printf("0: %d\\n", distances3[0]);
    
    // Test Case 4: Multiple paths
    AdjList graph4[4];
    memset(graph4, 0, sizeof(graph4));
    graph4[0].edges[0] = (Edge){1, 10};
    graph4[0].edges[1] = (Edge){2, 5};
    graph4[0].edge_count = 2;
    graph4[1].edges[0] = (Edge){3, 1};
    graph4[1].edge_count = 1;
    graph4[2].edges[0] = (Edge){1, 3};
    graph4[2].edges[1] = (Edge){3, 9};
    graph4[2].edge_count = 2;
    graph4[3].edge_count = 0;
    
    int distances4[4];
    dijkstra(graph4, 4, 0, distances4);
    printf("\\nTest Case 4:\\n");
    for (int i = 0; i < 4; i++) {
        printf("%d: %d\\n", i, distances4[i]);
    }
    
    // Test Case 5: Larger graph
    AdjList graph5[6];
    memset(graph5, 0, sizeof(graph5));
    graph5[0].edges[0] = (Edge){1, 7};
    graph5[0].edges[1] = (Edge){2, 9};
    graph5[0].edges[2] = (Edge){5, 14};
    graph5[0].edge_count = 3;
    graph5[1].edges[0] = (Edge){0, 7};
    graph5[1].edges[1] = (Edge){2, 10};
    graph5[1].edges[2] = (Edge){3, 15};
    graph5[1].edge_count = 3;
    graph5[2].edges[0] = (Edge){0, 9};
    graph5[2].edges[1] = (Edge){1, 10};
    graph5[2].edges[2] = (Edge){3, 11};
    graph5[2].edges[3] = (Edge){5, 2};
    graph5[2].edge_count = 4;
    graph5[3].edges[0] = (Edge){1, 15};
    graph5[3].edges[1] = (Edge){2, 11};
    graph5[3].edges[2] = (Edge){4, 6};
    graph5[3].edge_count = 3;
    graph5[4].edges[0] = (Edge){3, 6};
    graph5[4].edges[1] = (Edge){5, 9};
    graph5[4].edge_count = 2;
    graph5[5].edges[0] = (Edge){0, 14};
    graph5[5].edges[1] = (Edge){2, 2};
    graph5[5].edges[2] = (Edge){4, 9};
    graph5[5].edge_count = 3;
    
    int distances5[6];
    dijkstra(graph5, 6, 0, distances5);
    printf("\\nTest Case 5:\\n");
    for (int i = 0; i < 6; i++) {
        printf("%d: %d\\n", i, distances5[i]);
    }
    
    return 0;
}
