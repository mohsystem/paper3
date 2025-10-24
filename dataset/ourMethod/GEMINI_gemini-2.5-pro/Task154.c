#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int numVertices;
    int* adjListSizes;
    int** adjLists;
} Graph;

Graph* createGraph(int vertices) {
    if (vertices < 0) {
        fprintf(stderr, "Error: Number of vertices must be non-negative.\n");
        return NULL;
    }
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (graph == NULL) {
        perror("Failed to allocate memory for graph");
        return NULL;
    }
    graph->numVertices = vertices;

    graph->adjLists = (int**)malloc(vertices * sizeof(int*));
    graph->adjListSizes = (int*)calloc(vertices, sizeof(int));

    if (graph->adjLists == NULL || graph->adjListSizes == NULL) {
        perror("Failed to allocate memory for adjacency lists info");
        free(graph->adjLists);
        free(graph->adjListSizes);
        free(graph);
        return NULL;
    }

    for (int i = 0; i < vertices; i++) {
        graph->adjLists[i] = NULL;
    }

    return graph;
}

void freeGraph(Graph* graph) {
    if (graph == NULL) return;
    for (int i = 0; i < graph->numVertices; i++) {
        free(graph->adjLists[i]);
    }
    free(graph->adjLists);
    free(graph->adjListSizes);
    free(graph);
}

void addEdge(Graph* graph, int src, int dest) {
    if (graph == NULL || src < 0 || src >= graph->numVertices || dest < 0 || dest >= graph->numVertices) {
        fprintf(stderr, "Error: Invalid vertex for edge (%d, %d).\n", src, dest);
        return;
    }

    int newSize = graph->adjListSizes[src] + 1;
    int* newAdjList = (int*)realloc(graph->adjLists[src], newSize * sizeof(int));
    if (newAdjList == NULL) {
        perror("Failed to reallocate memory for adjacency list");
        return;
    }
    
    graph->adjLists[src] = newAdjList;
    graph->adjLists[src][graph->adjListSizes[src]] = dest;
    graph->adjListSizes[src]++;
}

void dfsUtil(Graph* graph, int v, bool* visited, int* result, int* result_count) {
    visited[v] = true;
    result[(*result_count)++] = v;

    for (int i = 0; i < graph->adjListSizes[v]; i++) {
        int neighbor = graph->adjLists[v][i];
        if (!visited[neighbor]) {
            dfsUtil(graph, neighbor, visited, result, result_count);
        }
    }
}

int* dfs(Graph* graph, int startNode, int* result_size) {
    *result_size = 0;
    if (graph == NULL || startNode < 0 || startNode >= graph->numVertices) {
        fprintf(stderr, "Error: Invalid start node or graph.\n");
        return NULL;
    }

    bool* visited = (bool*)calloc(graph->numVertices, sizeof(bool));
    int* result = (int*)malloc(graph->numVertices * sizeof(int));
    
    if (visited == NULL || result == NULL) {
        perror("Failed to allocate memory for DFS");
        free(visited);
        free(result);
        return NULL;
    }

    dfsUtil(graph, startNode, visited, result, result_size);
    
    free(visited);
    
    int* final_result = (int*)realloc(result, *result_size * sizeof(int));
    if (final_result == NULL && *result_size > 0) {
        perror("Failed to reallocate result array, returning original buffer");
        return result;
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
    int result_size;
    int* result_arr;

    // Test Case 1: Standard DFS
    printf("Test Case 1:\n");
    Graph* g1 = createGraph(7);
    addEdge(g1, 0, 1); addEdge(g1, 0, 2); addEdge(g1, 1, 3);
    addEdge(g1, 1, 4); addEdge(g1, 2, 5); addEdge(g1, 2, 6);
    printf("DFS starting from vertex 0: ");
    result_arr = dfs(g1, 0, &result_size);
    if (result_arr) { print_array(result_arr, result_size); free(result_arr); }
    freeGraph(g1);

    // Test Case 2: Graph with a cycle
    printf("\nTest Case 2:\n");
    Graph* g2 = createGraph(4);
    addEdge(g2, 0, 1); addEdge(g2, 0, 2); addEdge(g2, 1, 2);
    addEdge(g2, 2, 0); addEdge(g2, 2, 3); addEdge(g2, 3, 3);
    printf("DFS starting from vertex 2: ");
    result_arr = dfs(g2, 2, &result_size);
    if (result_arr) { print_array(result_arr, result_size); free(result_arr); }
    freeGraph(g2);
    
    // Test Case 3: Disconnected graph component
    printf("\nTest Case 3:\n");
    Graph* g3 = createGraph(5);
    addEdge(g3, 0, 1); addEdge(g3, 1, 2); addEdge(g3, 3, 4);
    printf("DFS starting from vertex 0: ");
    result_arr = dfs(g3, 0, &result_size);
    if (result_arr) { print_array(result_arr, result_size); free(result_arr); }
    printf("DFS starting from vertex 3: ");
    result_arr = dfs(g3, 3, &result_size);
    if (result_arr) { print_array(result_arr, result_size); free(result_arr); }
    freeGraph(g3);

    // Test Case 4: Start node has no outgoing edges
    printf("\nTest Case 4:\n");
    Graph* g4 = createGraph(3);
    addEdge(g4, 0, 1); addEdge(g4, 0, 2);
    printf("DFS starting from vertex 1: ");
    result_arr = dfs(g4, 1, &result_size);
    if (result_arr) { print_array(result_arr, result_size); free(result_arr); }
    freeGraph(g4);

    // Test Case 5: Invalid start node
    printf("\nTest Case 5:\n");
    Graph* g5 = createGraph(3);
    addEdge(g5, 0, 1); addEdge(g5, 1, 2);
    printf("DFS starting from vertex 5: ");
    result_arr = dfs(g5, 5, &result_size);
    if (result_arr) { print_array(result_arr, result_size); free(result_arr); }
    printf("DFS starting from vertex -1: ");
    result_arr = dfs(g5, -1, &result_size);
    if (result_arr) { print_array(result_arr, result_size); free(result_arr); }
    freeGraph(g5);

    return 0;
}