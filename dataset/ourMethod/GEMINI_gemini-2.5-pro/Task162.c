#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

// Struct for an adjacency list node
typedef struct Edge {
    int dest;
    int weight;
    struct Edge* next;
} Edge;

// Struct for the graph
typedef struct Graph {
    int V;
    Edge** adj;
} Graph;

// Creates a graph with V vertices
Graph* createGraph(int V) {
    if (V <= 0) return NULL;
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (!graph) return NULL;

    graph->V = V;
    graph->adj = (Edge**)malloc(V * sizeof(Edge*));
    if (!graph->adj) {
        free(graph);
        return NULL;
    }

    for (int i = 0; i < V; ++i) {
        graph->adj[i] = NULL;
    }
    return graph;
}

// Adds an edge to the graph
void addEdge(Graph* graph, int src, int dest, int weight) {
    if (!graph || src < 0 || src >= graph->V || dest < 0 || dest >= graph->V) return;
    
    Edge* newEdge = (Edge*)malloc(sizeof(Edge));
    if (!newEdge) return;

    newEdge->dest = dest;
    newEdge->weight = weight;
    newEdge->next = graph->adj[src];
    graph->adj[src] = newEdge;
}

// Frees the memory allocated for the graph
void freeGraph(Graph* graph) {
    if (!graph) return;
    for (int i = 0; i < graph->V; ++i) {
        Edge* current = graph->adj[i];
        while (current) {
            Edge* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(graph->adj);
    free(graph);
}

// Utility to find the vertex with the minimum distance value,
// from the set of vertices not yet included in the shortest path tree.
int minDistance(int dist[], bool visited[], int V) {
    int min = INT_MAX, min_index = -1;

    for (int v = 0; v < V; v++) {
        if (!visited[v] && dist[v] < min) { // Use < instead of <= to handle INT_MAX case
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

/**
 * Implements Dijkstra's algorithm (O(V^2) version without a priority queue).
 * The caller is responsible for freeing the returned array.
 *
 * @param graph The graph.
 * @param startNode The starting node.
 * @return An array of distances, or NULL on failure.
 */
int* dijkstra(Graph* graph, int startNode) {
    if (!graph || startNode < 0 || startNode >= graph->V) {
        fprintf(stderr, "Error: Invalid input provided to dijkstra.\n");
        return NULL;
    }
    int V = graph->V;
    int* dist = (int*)malloc(V * sizeof(int));
    bool* visited = (bool*)malloc(V * sizeof(bool));

    if (!dist || !visited) {
        free(dist); // free is safe on NULL
        free(visited);
        return NULL;
    }

    for (int i = 0; i < V; i++) {
        dist[i] = INT_MAX;
        visited[i] = false;
    }
    dist[startNode] = 0;

    for (int count = 0; count < V - 1; count++) {
        int u = minDistance(dist, visited, V);
        if (u == -1) break; // All remaining vertices are unreachable

        visited[u] = true;

        Edge* pCrawl = graph->adj[u];
        while (pCrawl != NULL) {
            int v = pCrawl->dest;
            int weight = pCrawl->weight;
            if (!visited[v] && dist[u] != INT_MAX && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
            }
            pCrawl = pCrawl->next;
        }
    }

    free(visited);
    return dist;
}

void printDistances(int* distances, int V, int startNode) {
    if (!distances) return;
    printf("Shortest distances from source node %d:\n", startNode);
    for (int i = 0; i < V; ++i) {
        printf("Node %d: ", i);
        if (distances[i] == INT_MAX) {
            printf("Infinity\n");
        } else {
            printf("%d\n", distances[i]);
        }
    }
}

int main() {
    // Test Case 1: Standard case
    int V1 = 5;
    Graph* graph1 = createGraph(V1);
    addEdge(graph1, 0, 1, 9);
    addEdge(graph1, 0, 2, 6);
    addEdge(graph1, 0, 3, 5);
    addEdge(graph1, 0, 4, 3);
    addEdge(graph1, 2, 1, 2);
    addEdge(graph1, 2, 3, 4);
    int startNode1 = 0;
    printf("Test Case 1: Standard graph from source 0\n");
    int* distances1 = dijkstra(graph1, startNode1);
    printDistances(distances1, V1, startNode1);
    free(distances1);
    printf("\n");

    // Test Case 2: Graph with unreachable nodes
    int V2 = 6;
    Graph* graph2 = createGraph(V2);
    addEdge(graph2, 0, 1, 7);
    addEdge(graph2, 0, 2, 9);
    addEdge(graph2, 1, 2, 10);
    addEdge(graph2, 1, 3, 15);
    addEdge(graph2, 2, 3, 11);
    // Node 4 and 5 are unreachable from 0
    addEdge(graph2, 4, 5, 6);
    int startNode2 = 0;
    printf("Test Case 2: Graph with unreachable nodes from source 0\n");
    int* distances2 = dijkstra(graph2, startNode2);
    printDistances(distances2, V2, startNode2);
    free(distances2);
    printf("\n");
    
    // Test Case 3: Different starting node
    int startNode3 = 3;
    printf("Test Case 3: Same as graph 1, but from source 3\n");
    int* distances3 = dijkstra(graph1, startNode3);
    printDistances(distances3, V1, startNode3);
    free(distances3);
    printf("\n");

    // Test Case 4: Linear graph
    int V4 = 4;
    Graph* graph4 = createGraph(V4);
    addEdge(graph4, 0, 1, 10);
    addEdge(graph4, 1, 2, 20);
    addEdge(graph4, 2, 3, 30);
    int startNode4 = 0;
    printf("Test Case 4: Linear graph from source 0\n");
    int* distances4 = dijkstra(graph4, startNode4);
    printDistances(distances4, V4, startNode4);
    free(distances4);
    printf("\n");

    // Test Case 5: Invalid input
    printf("Test Case 5: Invalid start node\n");
    int* distances5 = dijkstra(graph1, -1);
    if (distances5 == NULL) {
        printf("Function correctly handled invalid input by returning NULL.\n");
    }
    free(distances5); // free(NULL) is safe

    // Clean up memory
    freeGraph(graph1);
    freeGraph(graph2);
    freeGraph(graph4);

    return 0;
}