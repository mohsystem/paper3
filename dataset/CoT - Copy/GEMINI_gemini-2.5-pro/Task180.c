#include <stdio.h>
#include <stdlib.h>

// --- Data Structures for Graph ---
typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

typedef struct Graph {
    int numVertices;
    Node** adjLists;
} Graph;

// --- Graph Utility Functions ---
Node* createNode(int v) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        perror("Failed to allocate node");
        exit(EXIT_FAILURE);
    }
    newNode->vertex = v;
    newNode->next = NULL;
    return newNode;
}

Graph* createGraph(int vertices) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
     if (!graph) {
        perror("Failed to allocate graph");
        exit(EXIT_FAILURE);
    }
    graph->numVertices = vertices;
    graph->adjLists = (Node**)malloc(vertices * sizeof(Node*));
    if (!graph->adjLists) {
        perror("Failed to allocate adjacency lists");
        free(graph);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < vertices; i++) {
        graph->adjLists[i] = NULL;
    }
    return graph;
}

void addDirectedEdge(Graph* graph, int src, int dest) {
    if (src >= graph->numVertices || dest >= graph->numVertices) return;
    Node* newNode = createNode(dest);
    newNode->next = graph->adjLists[src];
    graph->adjLists[src] = newNode;
}

void addUndirectedEdge(Graph* graph, int src, int dest) {
    addDirectedEdge(graph, src, dest);
    addDirectedEdge(graph, dest, src);
}

void destroyGraph(Graph* graph) {
    if (!graph) return;
    for (int i = 0; i < graph->numVertices; i++) {
        Node* current = graph->adjLists[i];
        while (current != NULL) {
            Node* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(graph->adjLists);
    free(graph);
}

void printGraph(const Graph* graph) {
    if (!graph) return;
    printf("{");
    for (int v = 0; v < graph->numVertices; v++) {
        Node* temp = graph->adjLists[v];
        if (v > 0) printf(", ");
        printf("%d=[", v);
        while (temp) {
            printf("%d", temp->vertex);
            if(temp->next) printf(", ");
            temp = temp->next;
        }
        printf("]");
    }
    printf("}\n");
}

// --- Reparenting Logic ---
void buildNewTree_C(int u, int p, const Graph* originalAdj, Graph* newAdj) {
    Node* neighborNode = originalAdj->adjLists[u];
    while (neighborNode != NULL) {
        int v = neighborNode->vertex;
        if (v != p) {
            addDirectedEdge(newAdj, u, v);
            buildNewTree_C(v, u, originalAdj, newAdj);
        }
        neighborNode = neighborNode->next;
    }
}

Graph* reparentTree(const Graph* originalAdj, int newRoot) {
    if (newRoot >= originalAdj->numVertices || newRoot < 0) {
        return createGraph(0); 
    }
    Graph* newAdj = createGraph(originalAdj->numVertices);
    buildNewTree_C(newRoot, -1, originalAdj, newAdj);
    return newAdj;
}

// --- Main Function with Test Cases ---
int main() {
    // Test Case 1
    int edges1[][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}};
    int numEdges1 = sizeof(edges1) / sizeof(edges1[0]);
    int numVertices1 = 10;
    int newRoot1 = 6;
    Graph* graph1 = createGraph(numVertices1);
    for (int i = 0; i < numEdges1; i++) addUndirectedEdge(graph1, edges1[i][0], edges1[i][1]);
    Graph* reparented1 = reparentTree(graph1, newRoot1);
    printf("Test Case 1: Reparent on %d\n", newRoot1);
    printf("Original Tree: "); printGraph(graph1);
    printf("Reparented Tree: "); printGraph(reparented1);
    printf("\n");
    
    // Test Case 2
    int edges2[][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
    int numEdges2 = sizeof(edges2) / sizeof(edges2[0]);
    int numVertices2 = 5;
    int newRoot2 = 2;
    Graph* graph2 = createGraph(numVertices2);
    for (int i = 0; i < numEdges2; i++) addUndirectedEdge(graph2, edges2[i][0], edges2[i][1]);
    Graph* reparented2 = reparentTree(graph2, newRoot2);
    printf("Test Case 2: Reparent on %d\n", newRoot2);
    printf("Original Tree: "); printGraph(graph2);
    printf("Reparented Tree: "); printGraph(reparented2);
    printf("\n");
    destroyGraph(graph2);
    destroyGraph(reparented2);
    
    // Test Case 3
    int newRoot3 = 0;
    Graph* reparented3 = reparentTree(graph1, newRoot3);
    printf("Test Case 3: Reparent on %d\n", newRoot3);
    printf("Original Tree: "); printGraph(graph1);
    printf("Reparented Tree: "); printGraph(reparented3);
    printf("\n");
    destroyGraph(reparented3);

    // Test Case 4
    int newRoot4 = 9;
    Graph* reparented4 = reparentTree(graph1, newRoot4);
    printf("Test Case 4: Reparent on %d\n", newRoot4);
    printf("Original Tree: "); printGraph(graph1);
    printf("Reparented Tree: "); printGraph(reparented4);
    printf("\n");
    destroyGraph(reparented4);

    // Test Case 5
    int edges5[][2] = {{1, 0}, {0, 2}};
    int numEdges5 = sizeof(edges5) / sizeof(edges5[0]);
    int numVertices5 = 3;
    int newRoot5 = 1;
    Graph* graph5 = createGraph(numVertices5);
    for (int i = 0; i < numEdges5; i++) addUndirectedEdge(graph5, edges5[i][0], edges5[i][1]);
    Graph* reparented5 = reparentTree(graph5, newRoot5);
    printf("Test Case 5: Reparent on %d\n", newRoot5);
    printf("Original Tree: "); printGraph(graph5);
    printf("Reparented Tree: "); printGraph(reparented5);
    printf("\n");
    destroyGraph(graph5);
    destroyGraph(reparented5);

    // Cleanup for graph from test 1
    destroyGraph(graph1);
    destroyGraph(reparented1);

    return 0;
}