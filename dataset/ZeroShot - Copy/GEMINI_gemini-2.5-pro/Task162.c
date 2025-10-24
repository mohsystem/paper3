#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

// --- Data Structures ---

// Min-Heap node
typedef struct MinHeapNode {
    int vertex;
    long long distance;
} MinHeapNode;

// Min-Heap
typedef struct MinHeap {
    int size;
    int capacity;
    int *pos;       // To map vertex number to heap index for decreaseKey
    MinHeapNode **array;
} MinHeap;

// Edge in the graph
typedef struct Edge {
    int dest;
    int weight;
} Edge;

// Adjacency list (using a dynamic array)
typedef struct AdjList {
    Edge* edges;
    int count;
    int capacity;
} AdjList;

// --- Min-Heap Helper Functions ---

MinHeapNode* newMinHeapNode(int vertex, long long distance) {
    MinHeapNode* node = (MinHeapNode*)malloc(sizeof(MinHeapNode));
    if (!node) { perror("malloc failed"); exit(EXIT_FAILURE); }
    node->vertex = vertex;
    node->distance = distance;
    return node;
}

MinHeap* createMinHeap(int capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    if (!minHeap) { perror("malloc failed"); exit(EXIT_FAILURE); }
    minHeap->pos = (int*)malloc(capacity * sizeof(int));
    minHeap->array = (MinHeapNode**)malloc(capacity * sizeof(MinHeapNode*));
    if (!minHeap->pos || !minHeap->array) { perror("malloc failed"); exit(EXIT_FAILURE); }
    minHeap->size = 0;
    minHeap->capacity = capacity;
    return minHeap;
}

void swapMinHeapNode(MinHeapNode** a, MinHeapNode** b) {
    MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->distance < minHeap->array[smallest]->distance)
        smallest = left;
    if (right < minHeap->size && minHeap->array[right]->distance < minHeap->array[smallest]->distance)
        smallest = right;

    if (smallest != idx) {
        minHeap->pos[minHeap->array[smallest]->vertex] = idx;
        minHeap->pos[minHeap->array[idx]->vertex] = smallest;
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

MinHeapNode* extractMin(MinHeap* minHeap) {
    if (minHeap->size == 0) return NULL;
    MinHeapNode* root = minHeap->array[0];
    MinHeapNode* lastNode = minHeap->array[minHeap->size - 1];
    minHeap->array[0] = lastNode;
    minHeap->pos[root->vertex] = minHeap->size - 1;
    minHeap->pos[lastNode->vertex] = 0;
    --minHeap->size;
    minHeapify(minHeap, 0);
    return root;
}

void decreaseKey(MinHeap* minHeap, int v, long long dist) {
    int i = minHeap->pos[v];
    minHeap->array[i]->distance = dist;
    while (i > 0 && minHeap->array[i]->distance < minHeap->array[(i - 1) / 2]->distance) {
        minHeap->pos[minHeap->array[i]->vertex] = (i - 1) / 2;
        minHeap->pos[minHeap->array[(i - 1) / 2]->vertex] = i;
        swapMinHeapNode(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

// --- Graph Helper Functions ---

void addEdge(AdjList* graph, int src, int dest, int weight) {
    if (graph[src].count == graph[src].capacity) {
        graph[src].capacity *= 2;
        Edge* new_edges = (Edge*)realloc(graph[src].edges, graph[src].capacity * sizeof(Edge));
        if (!new_edges) { perror("realloc failed"); exit(EXIT_FAILURE); }
        graph[src].edges = new_edges;
    }
    graph[src].edges[graph[src].count].dest = dest;
    graph[src].edges[graph[src].count].weight = weight;
    graph[src].count++;
}

void freeGraph(AdjList* graph, int numNodes) {
    if (!graph) return;
    for (int i = 0; i < numNodes; ++i) {
        free(graph[i].edges);
    }
    free(graph);
}

// --- Dijkstra's Algorithm Implementation ---

long long* dijkstra(AdjList* graph, int numNodes, int startNode) {
    if (graph == NULL || numNodes <= 0 || startNode < 0 || startNode >= numNodes) {
        fprintf(stderr, "Error: Invalid input for Dijkstra's algorithm.\n");
        return NULL;
    }

    long long* distances = (long long*)malloc(numNodes * sizeof(long long));
    if (!distances) { perror("malloc failed"); exit(EXIT_FAILURE); }

    MinHeap* minHeap = createMinHeap(numNodes);
    for (int v = 0; v < numNodes; ++v) {
        distances[v] = LLONG_MAX;
        minHeap->array[v] = newMinHeapNode(v, distances[v]);
        minHeap->pos[v] = v;
    }

    distances[startNode] = 0;
    decreaseKey(minHeap, startNode, 0);
    minHeap->size = numNodes;

    while (minHeap->size > 0) {
        MinHeapNode* minHeapNode = extractMin(minHeap);
        int u = minHeapNode->vertex;
        
        for (int i = 0; i < graph[u].count; ++i) {
            Edge edge = graph[u].edges[i];
            int v = edge.dest;
            int weight = edge.weight;
            
            if (weight < 0) {
                fprintf(stderr, "Error: Graph contains a negative edge weight.\n");
                // Cleanup before returning NULL
                free(minHeapNode);
                while(minHeap->size > 0) free(extractMin(minHeap));
                free(minHeap->pos); free(minHeap->array); free(minHeap);
                free(distances);
                return NULL;
            }

            if (minHeap->pos[v] < minHeap->size && distances[u] != LLONG_MAX && 
                distances[u] + weight < distances[v]) {
                distances[v] = distances[u] + weight;
                decreaseKey(minHeap, v, distances[v]);
            }
        }
        free(minHeapNode);
    }

    free(minHeap->pos);
    free(minHeap->array);
    free(minHeap);
    return distances;
}

// --- Main and Test Functions ---

void printArr(long long dist[], int n, int startNode) {
    printf("Shortest distances from node %d:\n", startNode);
    for (int i = 0; i < n; ++i) {
        printf("  to node %d: ", i);
        if (dist[i] == LLONG_MAX) {
            printf("Infinity\n");
        } else {
            printf("%lld\n", dist[i]);
        }
    }
}

AdjList* createAndInitGraph(int numNodes) {
    AdjList* graph = (AdjList*)malloc(numNodes * sizeof(AdjList));
    if (!graph) { perror("malloc failed"); exit(EXIT_FAILURE); }
    for (int i = 0; i < numNodes; ++i) {
        graph[i].capacity = 2;
        graph[i].count = 0;
        graph[i].edges = (Edge*)malloc(graph[i].capacity * sizeof(Edge));
        if (!graph[i].edges) { perror("malloc failed"); exit(EXIT_FAILURE); }
    }
    return graph;
}

void runTest(AdjList* graph, int numNodes, int startNode, int testCaseNum) {
    printf("\nTest Case %d:\n", testCaseNum);
    long long* distances = dijkstra(graph, numNodes, startNode);
    if (distances) {
        printArr(distances, numNodes, startNode);
        free(distances);
    }
}

int main() {
    printf("C Dijkstra's Algorithm Test Cases:\n");

    // Test Case 1: Simple graph
    int numNodes1 = 5;
    AdjList* graph1 = createAndInitGraph(numNodes1);
    addEdge(graph1, 0, 1, 10); addEdge(graph1, 0, 4, 3);
    addEdge(graph1, 1, 2, 2);
    addEdge(graph1, 2, 3, 9);
    addEdge(graph1, 4, 1, 4); addEdge(graph1, 4, 2, 8); addEdge(graph1, 4, 3, 2);
    runTest(graph1, numNodes1, 0, 1);
    freeGraph(graph1, numNodes1);

    // Test Case 2: Disconnected graph
    int numNodes2 = 5;
    AdjList* graph2 = createAndInitGraph(numNodes2);
    addEdge(graph2, 0, 1, 1); addEdge(graph2, 1, 0, 1);
    addEdge(graph2, 2, 3, 2); addEdge(graph2, 3, 2, 2);
    runTest(graph2, numNodes2, 0, 2);
    freeGraph(graph2, numNodes2);

    // Test Case 3: Linear graph
    int numNodes3 = 4;
    AdjList* graph3 = createAndInitGraph(numNodes3);
    addEdge(graph3, 0, 1, 5); addEdge(graph3, 1, 2, 5); addEdge(graph3, 2, 3, 5);
    runTest(graph3, numNodes3, 0, 3);
    freeGraph(graph3, numNodes3);
    
    // Test Case 4: Graph with a cycle
    int numNodes4 = 4;
    AdjList* graph4 = createAndInitGraph(numNodes4);
    addEdge(graph4, 0, 1, 1);
    addEdge(graph4, 1, 2, 2); addEdge(graph4, 1, 3, 4);
    addEdge(graph4, 2, 0, 3);
    runTest(graph4, numNodes4, 0, 4);
    freeGraph(graph4, numNodes4);

    // Test Case 5: Start node with no outgoing edges
    int numNodes5 = 3;
    AdjList* graph5 = createAndInitGraph(numNodes5);
    addEdge(graph5, 1, 2, 10);
    runTest(graph5, numNodes5, 0, 5);
    freeGraph(graph5, numNodes5);

    return 0;
}