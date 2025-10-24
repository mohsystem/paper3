#include <stdio.h>
#include <stdlib.h>

// Adjacency list node
typedef struct AdjListNode {
    int dest;
    struct AdjListNode* next;
} AdjListNode;

// Adjacency list
typedef struct AdjList {
    AdjListNode* head;
} AdjList;

// Graph structure
typedef struct Graph {
    int V;
    AdjList* array;
} Graph;

// Function to create a new adjacency list node
AdjListNode* newAdjListNode(int dest) {
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (newNode == NULL) {
        perror("Memory allocation failed for AdjListNode");
        exit(EXIT_FAILURE);
    }
    newNode->dest = dest;
    newNode->next = NULL;
    return newNode;
}

// Function to create a graph of V vertices
Graph* createGraph(int V) {
    if (V < 0) {
        fprintf(stderr, "Number of vertices must be non-negative\n");
        return NULL;
    }
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (graph == NULL) {
        perror("Memory allocation failed for Graph");
        exit(EXIT_FAILURE);
    }
    graph->V = V;
    graph->array = (AdjList*)malloc(V * sizeof(AdjList));
    if (graph->array == NULL) {
        perror("Memory allocation failed for Adjacency List Array");
        free(graph);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < V; ++i) {
        graph->array[i].head = NULL;
    }
    return graph;
}

// Function to add an edge to an undirected graph
void addEdge(Graph* graph, int src, int dest) {
    if (!graph || src < 0 || src >= graph->V || dest < 0 || dest >= graph->V) {
        fprintf(stderr, "Error: Invalid edge or graph.\n");
        return;
    }
    // Add an edge from src to dest
    AdjListNode* newNode = newAdjListNode(dest);
    newNode->next = graph->array[src].head;
    graph->array[src].head = newNode;

    // Since graph is undirected, add an edge from dest to src also
    newNode = newAdjListNode(src);
    newNode->next = graph->array[dest].head;
    graph->array[dest].head = newNode;
}

// Recursive utility function for DFS
void dfsUtil(Graph* graph, int v, int* visited, int* result, int* result_index) {
    visited[v] = 1;
    result[(*result_index)++] = v;

    AdjListNode* pCrawl = graph->array[v].head;
    while (pCrawl != NULL) {
        int neighbor = pCrawl->dest;
        if (!visited[neighbor]) {
            dfsUtil(graph, neighbor, visited, result, result_index);
        }
        pCrawl = pCrawl->next;
    }
}

// The main DFS function
void dfs(Graph* graph, int startNode) {
    if (!graph) {
        fprintf(stderr, "Error: Graph is NULL.\n");
        return;
    }
    if (startNode < 0 || startNode >= graph->V) {
        fprintf(stderr, "Error: Start node is out of bounds.\n");
        return;
    }

    int* visited = (int*)calloc(graph->V, sizeof(int));
    int* result = (int*)malloc(graph->V * sizeof(int));
    if (visited == NULL || result == NULL) {
        perror("Memory allocation for DFS failed");
        free(visited);
        free(result);
        return;
    }
    
    int result_index = 0;
    dfsUtil(graph, startNode, visited, result, &result_index);

    // Print the result
    for (int i = 0; i < result_index; i++) {
        printf("%d ", result[i]);
    }
    printf("\n");

    free(visited);
    free(result);
}

// Function to free the graph memory
void freeGraph(Graph* graph) {
    if (!graph) return;
    for (int v = 0; v < graph->V; ++v) {
        AdjListNode* pCrawl = graph->array[v].head;
        while (pCrawl) {
            AdjListNode* temp = pCrawl;
            pCrawl = pCrawl->next;
            free(temp);
        }
    }
    free(graph->array);
    free(graph);
}


int main() {
    int numVertices = 8;
    Graph* g = createGraph(numVertices);
    if (!g) return 1;

    // Create a sample graph
    addEdge(g, 0, 1);
    addEdge(g, 0, 2);
    addEdge(g, 1, 3);
    addEdge(g, 1, 4);
    addEdge(g, 2, 5);
    addEdge(g, 2, 6);
    addEdge(g, 3, 7);
    // Node 7 is connected to 3

    printf("Depth First Traversal (starting from various nodes):\n");

    // Test Case 1
    int startNode1 = 0;
    printf("Test Case 1 (Start from %d): ", startNode1);
    dfs(g, startNode1);

    // Test Case 2
    int startNode2 = 3;
    printf("Test Case 2 (Start from %d): ", startNode2);
    dfs(g, startNode2);

    // Test Case 3
    int startNode3 = 2;
    printf("Test Case 3 (Start from %d): ", startNode3);
    dfs(g, startNode3);

    // Test Case 4
    int startNode4 = 6;
    printf("Test Case 4 (Start from %d): ", startNode4);
    dfs(g, startNode4);
    
    // Test Case 5
    int startNode5 = 5;
    printf("Test Case 5 (Start from %d): ", startNode5);
    dfs(g, startNode5);

    freeGraph(g);

    return 0;
}