#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

// Structure to represent a node in the adjacency list
typedef struct AdjListNode {
    int dest;
    int weight;
    struct AdjListNode* next;
} AdjListNode;

// Structure to represent the graph
typedef struct Graph {
    int V;
    AdjListNode** array;
} Graph;

// Function to create a new adjacency list node
AdjListNode* newAdjListNode(int dest, int weight) {
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    newNode->dest = dest;
    newNode->weight = weight;
    newNode->next = NULL;
    return newNode;
}

// Function to create a graph with V vertices
Graph* createGraph(int V) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->V = V;
    graph->array = (AdjListNode**)malloc(V * sizeof(AdjListNode*));
    for (int i = 0; i < V; ++i) {
        graph->array[i] = NULL;
    }
    return graph;
}

// Function to add an edge to the graph
void addEdge(Graph* graph, int src, int dest, int weight) {
    AdjListNode* newNode = newAdjListNode(dest, weight);
    newNode->next = graph->array[src];
    graph->array[src] = newNode;
}

// Helper to find the vertex with the minimum distance, from vertices not yet processed.
int minDistance(int dist[], bool sptSet[], int V) {
    int min = INT_MAX, min_index = -1;
    for (int v = 0; v < V; v++) {
        if (sptSet[v] == false && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

/**
 * Finds the shortest paths from a starting node to all other nodes in a weighted graph.
 *
 * @param graph      The graph structure.
 * @param startNode  The starting node.
 * @param dist       An allocated array to store the resulting distances.
 */
void dijkstra(Graph* graph, int startNode, int* dist) {
    int V = graph->V;
    bool sptSet[V];

    for (int i = 0; i < V; i++) {
        dist[i] = INT_MAX;
        sptSet[i] = false;
    }

    dist[startNode] = 0;

    for (int count = 0; count < V - 1; count++) {
        int u = minDistance(dist, sptSet, V);
        if (u == -1) break;

        sptSet[u] = true;

        AdjListNode* pCrawl = graph->array[u];
        while (pCrawl != NULL) {
            int v = pCrawl->dest;
            if (!sptSet[v] && dist[u] != INT_MAX && dist[u] + pCrawl->weight < dist[v]) {
                dist[v] = dist[u] + pCrawl->weight;
            }
            pCrawl = pCrawl->next;
        }
    }
}

// Function to free the memory allocated for the graph
void freeGraph(Graph* graph) {
    for (int i = 0; i < graph->V; i++) {
        AdjListNode* current = graph->array[i];
        while (current != NULL) {
            AdjListNode* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(graph->array);
    free(graph);
}

void printDistances(int startNode, int dist[], int V) {
    printf("Shortest distances from node %d: [", startNode);
    for (int i = 0; i < V; ++i) {
        if (dist[i] == INT_MAX) {
            printf("INF");
        } else {
            printf("%d", dist[i]);
        }
        if (i < V - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

int main() {
    // Test Case 1: Simple connected graph
    printf("--- Test Case 1 ---\n");
    int V1 = 5;
    Graph* graph1 = createGraph(V1);
    addEdge(graph1, 0, 1, 9); addEdge(graph1, 0, 2, 6); addEdge(graph1, 0, 3, 5); addEdge(graph1, 0, 4, 3);
    addEdge(graph1, 2, 1, 2); addEdge(graph1, 2, 3, 4);
    int* dist1 = (int*)malloc(V1 * sizeof(int));
    dijkstra(graph1, 0, dist1);
    printDistances(0, dist1, V1);
    free(dist1); freeGraph(graph1);

    // Test Case 2: Graph with unreachable node
    printf("\n--- Test Case 2 ---\n");
    int V2 = 4;
    Graph* graph2 = createGraph(V2);
    addEdge(graph2, 0, 1, 10); addEdge(graph2, 1, 2, 20);
    int* dist2 = (int*)malloc(V2 * sizeof(int));
    dijkstra(graph2, 0, dist2);
    printDistances(0, dist2, V2);
    free(dist2); freeGraph(graph2);
    
    // Test Case 3: Linear graph
    printf("\n--- Test Case 3 ---\n");
    int V3 = 4;
    Graph* graph3 = createGraph(V3);
    addEdge(graph3, 0, 1, 5); addEdge(graph3, 1, 2, 5); addEdge(graph3, 2, 3, 5);
    int* dist3 = (int*)malloc(V3 * sizeof(int));
    dijkstra(graph3, 0, dist3);
    printDistances(0, dist3, V3);
    free(dist3); freeGraph(graph3);

    // Test Case 4: Graph with a cycle
    printf("\n--- Test Case 4 ---\n");
    int V4 = 4;
    Graph* graph4 = createGraph(V4);
    addEdge(graph4, 0, 1, 1); addEdge(graph4, 0, 3, 10);
    addEdge(graph4, 1, 2, 2); addEdge(graph4, 2, 0, 3);
    int* dist4 = (int*)malloc(V4 * sizeof(int));
    dijkstra(graph4, 0, dist4);
    printDistances(0, dist4, V4);
    free(dist4); freeGraph(graph4);

    // Test Case 5: More complex graph with multiple paths
    printf("\n--- Test Case 5 ---\n");
    int V5 = 6;
    Graph* graph5 = createGraph(V5);
    addEdge(graph5, 0, 1, 7); addEdge(graph5, 0, 2, 9); addEdge(graph5, 0, 5, 14);
    addEdge(graph5, 1, 2, 10); addEdge(graph5, 1, 3, 15);
    addEdge(graph5, 2, 3, 11); addEdge(graph5, 2, 5, 2);
    addEdge(graph5, 3, 4, 6); addEdge(graph5, 4, 5, 9);
    int* dist5 = (int*)malloc(V5 * sizeof(int));
    dijkstra(graph5, 0, dist5);
    printDistances(0, dist5, V5);
    free(dist5); freeGraph(graph5);
    
    return 0;
}