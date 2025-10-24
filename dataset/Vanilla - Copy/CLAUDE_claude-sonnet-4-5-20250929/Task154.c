
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_VERTICES 100

typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

typedef struct Graph {
    int vertices;
    Node* adjList[MAX_VERTICES];
} Graph;

Graph* createGraph(int vertices) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->vertices = vertices;
    
    for (int i = 0; i < vertices; i++) {
        graph->adjList[i] = NULL;
    }
    
    return graph;
}

void addEdge(Graph* graph, int src, int dest) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->vertex = dest;
    newNode->next = graph->adjList[src];
    graph->adjList[src] = newNode;
}

void dfsUtil(Graph* graph, int node, bool* visited, int* result, int* resultSize) {
    visited[node] = true;
    result[(*resultSize)++] = node;
    
    Node* temp = graph->adjList[node];
    while (temp != NULL) {
        if (!visited[temp->vertex]) {
            dfsUtil(graph, temp->vertex, visited, result, resultSize);
        }
        temp = temp->next;
    }
}

void dfs(Graph* graph, int startNode, int* result, int* resultSize) {
    bool visited[MAX_VERTICES] = {false};
    *resultSize = 0;
    dfsUtil(graph, startNode, visited, result, resultSize);
}

void printArray(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]\\n");
}

void freeGraph(Graph* graph) {
    for (int i = 0; i < graph->vertices; i++) {
        Node* temp = graph->adjList[i];
        while (temp != NULL) {
            Node* toFree = temp;
            temp = temp->next;
            free(toFree);
        }
    }
    free(graph);
}

int main() {
    int result[MAX_VERTICES];
    int resultSize;
    
    // Test Case 1: Simple linear graph
    printf("Test Case 1: Linear Graph (0->1->2->3)\\n");
    Graph* g1 = createGraph(4);
    addEdge(g1, 0, 1);
    addEdge(g1, 1, 2);
    addEdge(g1, 2, 3);
    dfs(g1, 0, result, &resultSize);
    printf("DFS from node 0: ");
    printArray(result, resultSize);
    freeGraph(g1);
    
    // Test Case 2: Graph with branching
    printf("\\nTest Case 2: Branching Graph\\n");
    Graph* g2 = createGraph(7);
    addEdge(g2, 0, 1);
    addEdge(g2, 0, 2);
    addEdge(g2, 1, 3);
    addEdge(g2, 1, 4);
    addEdge(g2, 2, 5);
    addEdge(g2, 2, 6);
    dfs(g2, 0, result, &resultSize);
    printf("DFS from node 0: ");
    printArray(result, resultSize);
    freeGraph(g2);
    
    // Test Case 3: Graph with cycle
    printf("\\nTest Case 3: Graph with Cycle\\n");
    Graph* g3 = createGraph(4);
    addEdge(g3, 0, 1);
    addEdge(g3, 1, 2);
    addEdge(g3, 2, 0);
    addEdge(g3, 2, 3);
    dfs(g3, 0, result, &resultSize);
    printf("DFS from node 0: ");
    printArray(result, resultSize);
    freeGraph(g3);
    
    // Test Case 4: Disconnected graph
    printf("\\nTest Case 4: Disconnected Graph\\n");
    Graph* g4 = createGraph(5);
    addEdge(g4, 0, 1);
    addEdge(g4, 1, 2);
    addEdge(g4, 3, 4);
    dfs(g4, 0, result, &resultSize);
    printf("DFS from node 0: ");
    printArray(result, resultSize);
    dfs(g4, 3, result, &resultSize);
    printf("DFS from node 3: ");
    printArray(result, resultSize);
    freeGraph(g4);
    
    // Test Case 5: Single node
    printf("\\nTest Case 5: Single Node\\n");
    Graph* g5 = createGraph(1);
    dfs(g5, 0, result, &resultSize);
    printf("DFS from node 0: ");
    printArray(result, resultSize);
    freeGraph(g5);
    
    return 0;
}
