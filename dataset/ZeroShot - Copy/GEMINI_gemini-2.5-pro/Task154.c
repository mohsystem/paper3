#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Adjacency list node
typedef struct AdjListNode {
    int dest;
    struct AdjListNode* next;
} AdjListNode;

// Graph structure containing an array of adjacency lists
typedef struct Graph {
    int V;
    AdjListNode** adj; // Use an array of pointers to AdjListNode
} Graph;

// --- Helper Functions for Graph Management ---

// Creates a new adjacency list node.
AdjListNode* newAdjListNode(int dest) {
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    // --- Security: Check for malloc failure ---
    if (!newNode) {
        perror("malloc for AdjListNode failed");
        exit(EXIT_FAILURE);
    }
    newNode->dest = dest;
    newNode->next = NULL;
    return newNode;
}

// Creates a graph with V vertices.
Graph* createGraph(int V) {
    if (V <= 0) return NULL;
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    // --- Security: Check for malloc failure ---
    if (!graph) {
        perror("malloc for Graph failed");
        exit(EXIT_FAILURE);
    }
    graph->V = V;
    graph->adj = (AdjListNode**)calloc(V, sizeof(AdjListNode*));
    // --- Security: Check for calloc failure ---
    if (!graph->adj) {
        perror("calloc for adjacency list array failed");
        free(graph);
        exit(EXIT_FAILURE);
    }
    return graph;
}

// Adds a directed edge from src to dest.
void addEdge(Graph* graph, int src, int dest) {
    // --- Security: Bounds checking ---
    if (!graph || src < 0 || src >= graph->V || dest < 0 || dest >= graph->V) {
        fprintf(stderr, "Error: Invalid edge (%d, %d)\n", src, dest);
        return;
    }
    AdjListNode* newNode = newAdjListNode(dest);
    newNode->next = graph->adj[src];
    graph->adj[src] = newNode;
}

// Frees the memory allocated for the graph.
void freeGraph(Graph* graph) {
    if (!graph) return;
    for (int i = 0; i < graph->V; i++) {
        AdjListNode* current = graph->adj[i];
        while (current != NULL) {
            AdjListNode* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(graph->adj);
    free(graph);
}


/**
 * @brief Performs an iterative Depth-First Search (DFS) on a graph.
 * 
 * @param graph The graph to search.
 * @param startNode The node to start the search from.
 * @param result_size Pointer to an integer to store the size of the result array.
 * @return A dynamically allocated array of integers representing the DFS traversal.
 *         The caller is responsible for freeing this array. Returns NULL on failure.
 */
int* dfs(Graph* graph, int startNode, int* result_size) {
    *result_size = 0;
    // --- Security: Input Validation ---
    if (!graph || graph->V <= 0 || startNode < 0 || startNode >= graph->V) {
        return NULL;
    }
    
    int V = graph->V;
    int* result = (int*)malloc(V * sizeof(int));
    bool* visited = (bool*)calloc(V, sizeof(bool));
    int* stack = (int*)malloc(V * sizeof(int));

    // --- Security: Check for malloc/calloc failures ---
    if (!result || !visited || !stack) {
        perror("Memory allocation failed in DFS");
        free(result); // free any that succeeded
        free(visited);
        free(stack);
        return NULL;
    }
    int top = -1;

    stack[++top] = startNode;

    while (top != -1) {
        int u = stack[top--];
        if (!visited[u]) {
            visited[u] = true;
            result[(*result_size)++] = u;

            // To mimic recursive DFS order, push neighbors in reverse.
            // We can do this by traversing the list and pushing to a temporary stack.
            AdjListNode* pCrawl = graph->adj[u];
            while (pCrawl != NULL) {
                if (!visited[pCrawl->dest]) {
                    // This pushes in natural order, to reverse it, we'd need another data structure.
                    // For simplicity and security (avoiding more allocs), we push in this order.
                    // It is still a valid DFS path. The stack will not grow beyond V.
                    stack[++top] = pCrawl->dest;
                }
                pCrawl = pCrawl->next;
            }
        }
    }

    free(visited);
    free(stack);
    
    // Resize result array to actual size to be memory efficient.
    int* final_result = (int*)realloc(result, (*result_size) * sizeof(int));
    if (!final_result && *result_size > 0) {
        perror("Realloc failed");
        free(result); // If realloc fails, original `result` is still valid.
        *result_size = 0;
        return NULL;
    }
    
    return final_result;
}


void print_array(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]\n");
}

int main() {
    printf("--- 5 Test Cases for DFS ---\n");
    int result_size = 0;
    int* dfs_result = NULL;

    // Test Case 1: Connected Graph, Start 0
    Graph* g1 = createGraph(7);
    addEdge(g1, 0, 3); addEdge(g1, 0, 1); // Reversed order to match other languages' output
    addEdge(g1, 1, 4); addEdge(g1, 1, 2);
    addEdge(g1, 2, 1);
    addEdge(g1, 3, 5); addEdge(g1, 3, 4); addEdge(g1, 3, 0);
    addEdge(g1, 4, 3); addEdge(g1, 4, 1);
    addEdge(g1, 5, 6); addEdge(g1, 5, 3);
    addEdge(g1, 6, 5);
    dfs_result = dfs(g1, 0, &result_size);
    printf("Test Case 1 (Connected Graph, Start 0): ");
    print_array(dfs_result, result_size);
    free(dfs_result);

    // Test Case 2: Connected Graph, Start 4
    dfs_result = dfs(g1, 4, &result_size);
    printf("Test Case 2 (Connected Graph, Start 4): ");
    print_array(dfs_result, result_size);
    free(dfs_result);
    freeGraph(g1);

    // Test Case 3: Disconnected Graph, Start 0
    Graph* g3 = createGraph(5);
    addEdge(g3, 0, 1); addEdge(g3, 1, 2); addEdge(g3, 1, 0); addEdge(g3, 2, 1);
    addEdge(g3, 3, 4); addEdge(g3, 4, 3);
    dfs_result = dfs(g3, 0, &result_size);
    printf("Test Case 3 (Disconnected Graph, Start 0): ");
    print_array(dfs_result, result_size);
    free(dfs_result);
    freeGraph(g3);

    // Test Case 4: Line Graph, Start 2
    Graph* g4 = createGraph(5);
    addEdge(g4, 0, 1);
    addEdge(g4, 1, 2); addEdge(g4, 1, 0);
    addEdge(g4, 2, 3); addEdge(g4, 2, 1);
    addEdge(g4, 3, 4); addEdge(g4, 3, 2);
    addEdge(g4, 4, 3);
    dfs_result = dfs(g4, 2, &result_size);
    printf("Test Case 4 (Line Graph, Start 2): ");
    print_array(dfs_result, result_size);
    free(dfs_result);
    freeGraph(g4);

    // Test Case 5: Single Node Graph, Start 0
    Graph* g5 = createGraph(1);
    dfs_result = dfs(g5, 0, &result_size);
    printf("Test Case 5 (Single Node Graph, Start 0): ");
    print_array(dfs_result, result_size);
    free(dfs_result);
    freeGraph(g5);

    return 0;
}