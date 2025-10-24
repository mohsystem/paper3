
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct Graph {
    int vertices;
    Node** adjacencyList;
} Graph;

Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode) {
        newNode->data = data;
        newNode->next = NULL;
    }
    return newNode;
}

Graph* createGraph(int vertices) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (!graph) return NULL;
    
    graph->vertices = vertices;
    graph->adjacencyList = (Node**)malloc(vertices * sizeof(Node*));
    
    if (!graph->adjacencyList) {
        free(graph);
        return NULL;
    }
    
    for (int i = 0; i < vertices; i++) {
        graph->adjacencyList[i] = NULL;
    }
    
    return graph;
}

void addEdge(Graph* graph, int source, int destination) {
    if (!graph || source < 0 || source >= graph->vertices || 
        destination < 0 || destination >= graph->vertices) {
        return;
    }
    
    Node* newNode = createNode(destination);
    if (!newNode) return;
    
    newNode->next = graph->adjacencyList[source];
    graph->adjacencyList[source] = newNode;
}

void dfsUtil(Graph* graph, int node, bool* visited, int* result, int* resultSize) {
    visited[node] = true;
    result[(*resultSize)++] = node;
    
    Node* temp = graph->adjacencyList[node];
    while (temp) {
        if (!visited[temp->data]) {
            dfsUtil(graph, temp->data, visited, result, resultSize);
        }
        temp = temp->next;
    }
}

int* depthFirstSearch(Graph* graph, int startNode, int* resultSize) {
    *resultSize = 0;
    if (!graph || startNode < 0 || startNode >= graph->vertices) {
        return NULL;
    }
    
    int* result = (int*)malloc(graph->vertices * sizeof(int));
    bool* visited = (bool*)calloc(graph->vertices, sizeof(bool));
    
    if (!result || !visited) {
        free(result);
        free(visited);
        return NULL;
    }
    
    dfsUtil(graph, startNode, visited, result, resultSize);
    
    free(visited);
    return result;
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
    if (!graph) return;
    
    for (int i = 0; i < graph->vertices; i++) {
        Node* temp = graph->adjacencyList[i];
        while (temp) {
            Node* toFree = temp;
            temp = temp->next;
            free(toFree);
        }
    }
    free(graph->adjacencyList);
    free(graph);
}

int main() {
    int resultSize;
    int* result;
    
    // Test Case 1: Simple linear graph
    printf("Test Case 1: Linear Graph\\n");
    Graph* graph1 = createGraph(5);
    addEdge(graph1, 0, 1);
    addEdge(graph1, 1, 2);
    addEdge(graph1, 2, 3);
    addEdge(graph1, 3, 4);
    result = depthFirstSearch(graph1, 0, &resultSize);
    printf("DFS from node 0: ");
    printArray(result, resultSize);
    free(result);
    freeGraph(graph1);
    
    // Test Case 2: Graph with branches
    printf("\\nTest Case 2: Branched Graph\\n");
    Graph* graph2 = createGraph(7);
    addEdge(graph2, 0, 1);
    addEdge(graph2, 0, 2);
    addEdge(graph2, 1, 3);
    addEdge(graph2, 1, 4);
    addEdge(graph2, 2, 5);
    addEdge(graph2, 2, 6);
    result = depthFirstSearch(graph2, 0, &resultSize);
    printf("DFS from node 0: ");
    printArray(result, resultSize);
    free(result);
    freeGraph(graph2);
    
    // Test Case 3: Graph with cycle
    printf("\\nTest Case 3: Graph with Cycle\\n");
    Graph* graph3 = createGraph(4);
    addEdge(graph3, 0, 1);
    addEdge(graph3, 1, 2);
    addEdge(graph3, 2, 3);
    addEdge(graph3, 3, 1);
    result = depthFirstSearch(graph3, 0, &resultSize);
    printf("DFS from node 0: ");
    printArray(result, resultSize);
    free(result);
    freeGraph(graph3);
    
    // Test Case 4: Disconnected graph
    printf("\\nTest Case 4: Disconnected Graph\\n");
    Graph* graph4 = createGraph(6);
    addEdge(graph4, 0, 1);
    addEdge(graph4, 1, 2);
    addEdge(graph4, 3, 4);
    addEdge(graph4, 4, 5);
    result = depthFirstSearch(graph4, 0, &resultSize);
    printf("DFS from node 0: ");
    printArray(result, resultSize);
    free(result);
    result = depthFirstSearch(graph4, 3, &resultSize);
    printf("DFS from node 3: ");
    printArray(result, resultSize);
    free(result);
    freeGraph(graph4);
    
    // Test Case 5: Single node
    printf("\\nTest Case 5: Single Node\\n");
    Graph* graph5 = createGraph(1);
    result = depthFirstSearch(graph5, 0, &resultSize);
    printf("DFS from node 0: ");
    printArray(result, resultSize);
    free(result);
    freeGraph(graph5);
    
    return 0;
}
