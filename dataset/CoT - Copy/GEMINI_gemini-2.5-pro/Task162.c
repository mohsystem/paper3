#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

// Struct to represent an edge in the adjacency list
typedef struct {
    int to;
    int weight;
} Edge;

// Struct to represent an adjacency list for a vertex
typedef struct {
    Edge* edges;
    int count;
    int capacity;
} AdjList;

// Helper function to create a graph with V vertices
AdjList* create_graph(int V) {
    AdjList* graph = (AdjList*)malloc(V * sizeof(AdjList));
    if (graph == NULL) return NULL;
    for (int i = 0; i < V; i++) {
        graph[i].edges = NULL;
        graph[i].count = 0;
        graph[i].capacity = 0;
    }
    return graph;
}

// Helper function to add an edge to the graph
void add_edge(AdjList* graph, int u, int v, int weight) {
    // Reallocate memory for the new edge if needed
    if (graph[u].count >= graph[u].capacity) {
        graph[u].capacity = (graph[u].capacity == 0) ? 2 : graph[u].capacity * 2;
        Edge* new_edges = (Edge*)realloc(graph[u].edges, graph[u].capacity * sizeof(Edge));
        if (new_edges == NULL) {
            // Handle realloc failure
            return;
        }
        graph[u].edges = new_edges;
    }
    graph[u].edges[graph[u].count].to = v;
    graph[u].edges[graph[u].count].weight = weight;
    graph[u].count++;
}

// Helper function to free the graph's memory
void free_graph(AdjList* graph, int V) {
    if (graph == NULL) return;
    for (int i = 0; i < V; i++) {
        free(graph[i].edges);
    }
    free(graph);
}


// Function to find the vertex with the minimum distance value,
// from the set of vertices not yet included in the shortest path tree.
int min_distance(int V, const int dist[], const bool visited[]) {
    int min = INT_MAX, min_index = -1;

    for (int v = 0; v < V; v++) {
        if (!visited[v] && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

/**
 * Implements Dijkstra's algorithm (O(V^2) version) to find the shortest
 * path from a source node to all other nodes.
 *
 * @param V The number of vertices in the graph.
 * @param adj The adjacency list representation of the graph.
 * @param src The source vertex.
 * @return A dynamically allocated array of distances. The caller must free it.
 */
int* dijkstra(int V, AdjList* adj, int src) {
    int* dist = (int*)malloc(V * sizeof(int));
    bool* visited = (bool*)malloc(V * sizeof(bool));

    if (dist == NULL || visited == NULL) {
        free(dist);
        free(visited);
        return NULL;
    }

    for (int i = 0; i < V; i++) {
        dist[i] = INT_MAX;
        visited[i] = false;
    }
    dist[src] = 0;

    for (int count = 0; count < V - 1; count++) {
        int u = min_distance(V, dist, visited);

        if (u == -1) {
            break; // All remaining vertices are unreachable
        }

        visited[u] = true;

        for (int i = 0; i < adj[u].count; i++) {
            Edge edge = adj[u].edges[i];
            int v = edge.to;
            int weight = edge.weight;

            if (!visited[v] && dist[u] != INT_MAX && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
            }
        }
    }

    free(visited);
    return dist;
}

void print_distances(int src, int V, const int* dist) {
    printf("Distances from source %d: [", src);
    for (int i = 0; i < V; i++) {
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
    // --- Test Case 1: Simple Graph ---
    printf("--- Test Case 1 ---\n");
    int V1 = 5, src1 = 0;
    AdjList* graph1 = create_graph(V1);
    add_edge(graph1, 0, 1, 10); add_edge(graph1, 0, 3, 5);
    add_edge(graph1, 1, 2, 1); add_edge(graph1, 1, 3, 2);
    add_edge(graph1, 2, 4, 4);
    add_edge(graph1, 3, 1, 3); add_edge(graph1, 3, 2, 9); add_edge(graph1, 3, 4, 2);
    add_edge(graph1, 4, 0, 7); add_edge(graph1, 4, 2, 6);
    int* dist1 = dijkstra(V1, graph1, src1);
    if(dist1) { print_distances(src1, V1, dist1); free(dist1); }
    free_graph(graph1, V1);

    // --- Test Case 2: Disconnected Graph ---
    printf("\n--- Test Case 2 ---\n");
    int V2 = 6, src2 = 0;
    AdjList* graph2 = create_graph(V2);
    add_edge(graph2, 0, 1, 2);
    add_edge(graph2, 1, 2, 3);
    add_edge(graph2, 3, 4, 5);
    add_edge(graph2, 4, 5, 6);
    int* dist2 = dijkstra(V2, graph2, src2);
    if(dist2) { print_distances(src2, V2, dist2); free(dist2); }
    free_graph(graph2, V2);

    // --- Test Case 3: Line Graph ---
    printf("\n--- Test Case 3 ---\n");
    int V3 = 4, src3 = 0;
    AdjList* graph3 = create_graph(V3);
    add_edge(graph3, 0, 1, 1);
    add_edge(graph3, 1, 2, 2);
    add_edge(graph3, 2, 3, 3);
    int* dist3 = dijkstra(V3, graph3, src3);
    if(dist3) { print_distances(src3, V3, dist3); free(dist3); }
    free_graph(graph3, V3);

    // --- Test Case 4: Graph with alternative paths ---
    printf("\n--- Test Case 4 ---\n");
    int V4 = 6, src4 = 0;
    AdjList* graph4 = create_graph(V4);
    add_edge(graph4, 0, 1, 4); add_edge(graph4, 0, 2, 1);
    add_edge(graph4, 1, 3, 1);
    add_edge(graph4, 2, 1, 2); add_edge(graph4, 2, 3, 5);
    add_edge(graph4, 3, 4, 3);
    add_edge(graph4, 4, 5, 2);
    add_edge(graph4, 2, 5, 10);
    int* dist4 = dijkstra(V4, graph4, src4);
    if(dist4) { print_distances(src4, V4, dist4); free(dist4); }
    free_graph(graph4, V4);

    // --- Test Case 5: Single Node Graph ---
    printf("\n--- Test Case 5 ---\n");
    int V5 = 1, src5 = 0;
    AdjList* graph5 = create_graph(V5);
    int* dist5 = dijkstra(V5, graph5, src5);
    if(dist5) { print_distances(src5, V5, dist5); free(dist5); }
    free_graph(graph5, V5);

    return 0;
}