#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Adjacency list node
typedef struct AdjListNode {
    int dest;
    struct AdjListNode* next;
} AdjListNode;

// Graph structure using an array of adjacency lists
typedef struct Graph {
    int V;
    AdjListNode** array;
} Graph;

// Create a new adjacency list node
AdjListNode* newAdjListNode(int dest) {
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    newNode->dest = dest;
    newNode->next = NULL;
    return newNode;
}

// Create a graph with V vertices
Graph* createGraph(int V) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->V = V;
    graph->array = (AdjListNode**)calloc(V, sizeof(AdjListNode*));
    return graph;
}

// Add an edge to a directed graph
void addEdge(Graph* graph, int src, int dest) {
    if (src >= graph->V || dest >= graph->V) return;
    AdjListNode* newNode = newAdjListNode(dest);
    newNode->next = graph->array[src];
    graph->array[src] = newNode;
}

// Free the graph memory
void freeGraph(Graph* graph) {
    if (!graph) return;
    for (int v = 0; v < graph->V; ++v) {
        AdjListNode* pCrawl = graph->array[v];
        while (pCrawl) {
            AdjListNode* temp = pCrawl;
            pCrawl = pCrawl->next;
            free(temp);
        }
    }
    free(graph->array);
    free(graph);
}


// DFS utility function (recursive)
void DFSUtil(Graph* graph, int v, bool* visited, int* result, int* resultIndex) {
    visited[v] = true;
    result[*resultIndex] = v;
    (*resultIndex)++;

    AdjListNode* pCrawl = graph->array[v];
    while (pCrawl != NULL) {
        int neighbor = pCrawl->dest;
        if (!visited[neighbor]) {
            DFSUtil(graph, neighbor, visited, result, resultIndex);
        }
        pCrawl = pCrawl->next;
    }
}

/**
 * Performs Depth First Search traversal starting from a given node.
 * @param graph The graph to search.
 * @param startNode The starting node for the DFS.
 * @param result_size Pointer to an integer to store the size of the result array.
 * @return A dynamically allocated array with the DFS traversal.
 *         The caller is responsible for freeing this memory.
 */
int* performDFS(Graph* graph, int startNode, int* result_size) {
    *result_size = 0;
    if (startNode >= graph->V) {
        printf("Start node is out of bounds.\n");
        return NULL;
    }
    
    bool* visited = (bool*)calloc(graph->V, sizeof(bool));
    if (!visited) return NULL;
    
    int* result = (int*)malloc(graph->V * sizeof(int));
    if (!result) {
        free(visited);
        return NULL;
    }

    int resultIndex = 0;
    DFSUtil(graph, startNode, visited, result, &resultIndex);
    
    *result_size = resultIndex;
    
    free(visited);
    // Optionally realloc to save space, but not strictly necessary for correctness
    int* final_result = (int*)realloc(result, resultIndex * sizeof(int));
    return final_result ? final_result : result;
}

void printArray(const int* arr, int size) {
    if (!arr) return;
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main() {
    int result_size;
    int* result;

    // Test Case 1: Simple linear graph
    printf("Test Case 1:\n");
    Graph* g1 = createGraph(4);
    addEdge(g1, 2, 3); // Edges added in reverse for consistent output
    addEdge(g1, 1, 2);
    addEdge(g1, 0, 1);
    result = performDFS(g1, 0, &result_size);
    printf("DFS starting from node 0: ");
    printArray(result, result_size);
    free(result);
    freeGraph(g1);

    // Test Case 2: Graph with a fork
    printf("\nTest Case 2:\n");
    Graph* g2 = createGraph(5);
    addEdge(g2, 1, 4);
    addEdge(g2, 1, 3);
    addEdge(g2, 0, 2);
    addEdge(g2, 0, 1);
    result = performDFS(g2, 0, &result_size);
    printf("DFS starting from node 0: ");
    printArray(result, result_size);
    free(result);
    freeGraph(g2);

    // Test Case 3: Graph with a cycle
    printf("\nTest Case 3:\n");
    Graph* g3 = createGraph(4);
    addEdge(g3, 2, 3);
    addEdge(g3, 2, 0);
    addEdge(g3, 1, 2);
    addEdge(g3, 0, 1);
    result = performDFS(g3, 0, &result_size);
    printf("DFS starting from node 0: ");
    printArray(result, result_size);
    free(result);
    freeGraph(g3);

    // Test Case 4: Disconnected graph
    printf("\nTest Case 4:\n");
    Graph* g4 = createGraph(5);
    addEdge(g4, 3, 4);
    addEdge(g4, 0, 2);
    addEdge(g4, 0, 1);
    result = performDFS(g4, 0, &result_size);
    printf("DFS starting from node 0: ");
    printArray(result, result_size);
    free(result);
    result = performDFS(g4, 3, &result_size);
    printf("DFS starting from node 3: ");
    printArray(result, result_size);
    free(result);
    freeGraph(g4);

    // Test Case 5: Single-node graph
    printf("\nTest Case 5:\n");
    Graph* g5 = createGraph(1);
    result = performDFS(g5, 0, &result_size);
    printf("DFS starting from node 0: ");
    printArray(result, result_size);
    free(result);
    freeGraph(g5);

    return 0;
}