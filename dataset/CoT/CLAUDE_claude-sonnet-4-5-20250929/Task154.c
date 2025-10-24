
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_NODES 1000
#define MAX_EDGES 1000

typedef struct {
    int nodes[MAX_EDGES];
    int count;
} AdjacencyList;

typedef struct {
    AdjacencyList lists[MAX_NODES];
    bool nodeExists[MAX_NODES];
} Graph;

typedef struct {
    int data[MAX_NODES];
    int size;
} Result;

void initGraph(Graph* graph) {
    if (graph == NULL) return;
    
    memset(graph->nodeExists, false, sizeof(graph->nodeExists));
    for (int i = 0; i < MAX_NODES; i++) {
        graph->lists[i].count = 0;
    }
}

bool addEdge(Graph* graph, int source, int destination) {
    if (graph == NULL || source < 0 || destination < 0 || 
        source >= MAX_NODES || destination >= MAX_NODES) {
        return false;
    }
    
    graph->nodeExists[source] = true;
    graph->nodeExists[destination] = true;
    
    if (graph->lists[source].count < MAX_EDGES) {
        graph->lists[source].nodes[graph->lists[source].count++] = destination;
        return true;
    }
    
    return false;
}

void dfsHelper(Graph* graph, int node, bool* visited, Result* result) {
    if (graph == NULL || visited == NULL || result == NULL) return;
    if (node < 0 || node >= MAX_NODES) return;
    if (visited[node]) return;
    
    visited[node] = true;
    
    if (result->size < MAX_NODES) {
        result->data[result->size++] = node;
    }
    
    for (int i = 0; i < graph->lists[node].count; i++) {
        int neighbor = graph->lists[node].nodes[i];
        if (!visited[neighbor]) {
            dfsHelper(graph, neighbor, visited, result);
        }
    }
}

Result depthFirstSearch(Graph* graph, int startNode) {
    Result result;
    result.size = 0;
    
    if (graph == NULL || startNode < 0 || startNode >= MAX_NODES) {
        return result;
    }
    
    if (!graph->nodeExists[startNode]) {
        return result;
    }
    
    bool visited[MAX_NODES];
    memset(visited, false, sizeof(visited));
    
    dfsHelper(graph, startNode, visited, &result);
    return result;
}

void printResult(Result result) {
    printf("[");
    for (int i = 0; i < result.size; i++) {
        printf("%d", result.data[i]);
        if (i < result.size - 1) printf(", ");
    }
    printf("]\\n");
}

int main() {
    // Test Case 1: Simple linear graph
    printf("Test Case 1: Linear Graph\\n");
    Graph graph1;
    initGraph(&graph1);
    addEdge(&graph1, 1, 2);
    addEdge(&graph1, 2, 3);
    addEdge(&graph1, 3, 4);
    printf("DFS from node 1: ");
    printResult(depthFirstSearch(&graph1, 1));
    
    // Test Case 2: Graph with branches
    printf("\\nTest Case 2: Branched Graph\\n");
    Graph graph2;
    initGraph(&graph2);
    addEdge(&graph2, 1, 2);
    addEdge(&graph2, 1, 3);
    addEdge(&graph2, 2, 4);
    addEdge(&graph2, 2, 5);
    addEdge(&graph2, 3, 6);
    printf("DFS from node 1: ");
    printResult(depthFirstSearch(&graph2, 1));
    
    // Test Case 3: Graph with cycle
    printf("\\nTest Case 3: Graph with Cycle\\n");
    Graph graph3;
    initGraph(&graph3);
    addEdge(&graph3, 1, 2);
    addEdge(&graph3, 2, 3);
    addEdge(&graph3, 3, 1);
    addEdge(&graph3, 3, 4);
    printf("DFS from node 1: ");
    printResult(depthFirstSearch(&graph3, 1));
    
    // Test Case 4: Disconnected graph
    printf("\\nTest Case 4: Disconnected Graph\\n");
    Graph graph4;
    initGraph(&graph4);
    addEdge(&graph4, 1, 2);
    addEdge(&graph4, 3, 4);
    addEdge(&graph4, 5, 6);
    printf("DFS from node 1: ");
    printResult(depthFirstSearch(&graph4, 1));
    printf("DFS from node 3: ");
    printResult(depthFirstSearch(&graph4, 3));
    
    // Test Case 5: Single node
    printf("\\nTest Case 5: Single Node\\n");
    Graph graph5;
    initGraph(&graph5);
    addEdge(&graph5, 1, 1);
    printf("DFS from node 1: ");
    printResult(depthFirstSearch(&graph5, 1));
    
    return 0;
}
