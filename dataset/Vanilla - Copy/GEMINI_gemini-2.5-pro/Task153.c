#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Adjacency list node
typedef struct AdjListNode {
    int dest;
    struct AdjListNode* next;
} AdjListNode;

// Graph structure
typedef struct Graph {
    int V;
    AdjListNode** array;
} Graph;

// Queue node
typedef struct QNode {
    int key;
    struct QNode* next;
} QNode;

// Queue structure
typedef struct Queue {
    QNode *front, *rear;
} Queue;

// --- Graph utility functions ---
AdjListNode* newAdjListNode(int dest) {
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    newNode->dest = dest;
    newNode->next = NULL;
    return newNode;
}

Graph* createGraph(int V) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->V = V;
    graph->array = (AdjListNode**)malloc(V * sizeof(AdjListNode*));
    for (int i = 0; i < V; ++i) graph->array[i] = NULL;
    return graph;
}

void addEdge(Graph* graph, int src, int dest) {
    AdjListNode* newNode = newAdjListNode(dest);
    newNode->next = graph->array[src];
    graph->array[src] = newNode;
    newNode = newAdjListNode(src);
    newNode->next = graph->array[dest];
    graph->array[dest] = newNode;
}

void freeGraph(Graph* graph) {
    for (int i = 0; i < graph->V; ++i) {
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

// --- Queue utility functions ---
Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enqueue(Queue* q, int k) {
    QNode* temp = (QNode*)malloc(sizeof(QNode));
    temp->key = k;
    temp->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}

int dequeue(Queue* q) {
    if (q->front == NULL) return -1;
    QNode* temp = q->front;
    int key = temp->key;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    free(temp);
    return key;
}

bool isQueueEmpty(Queue* q) {
    return (q->front == NULL);
}

/**
 * Performs BFS on a graph.
 *
 * @param graph The graph.
 * @param startNode The starting vertex.
 * @param resultSize Pointer to an integer to store the size of the result array.
 * @return A dynamically allocated array with the BFS traversal. The caller must free this array.
 */
int* bfs(Graph* graph, int startNode, int* resultSize) {
    *resultSize = 0;
    if (startNode >= graph->V) return NULL;
    
    int* bfsTraversal = (int*)malloc(graph->V * sizeof(int));
    bool* visited = (bool*)calloc(graph->V, sizeof(bool));
    Queue* q = createQueue();

    visited[startNode] = true;
    enqueue(q, startNode);

    while (!isQueueEmpty(q)) {
        int currentNode = dequeue(q);
        if (*resultSize < graph->V) {
            bfsTraversal[(*resultSize)++] = currentNode;
        }

        AdjListNode* adjList = graph->array[currentNode];
        while (adjList != NULL) {
            int neighbor = adjList->dest;
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                enqueue(q, neighbor);
            }
            adjList = adjList->next;
        }
    }

    free(visited);
    free(q);
    
    int* finalTraversal = (int*)realloc(bfsTraversal, (*resultSize) * sizeof(int));
    return (finalTraversal != NULL || *resultSize == 0) ? finalTraversal : bfsTraversal;
}

// Helper to print result array
void printArray(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i < size - 1) ? ", " : "");
    }
    printf("]\n");
}

int main() {
    // Test Case 1: Connected graph
    printf("--- Test Case 1 ---\n");
    Graph* g1 = createGraph(5);
    addEdge(g1, 0, 1); addEdge(g1, 0, 2); addEdge(g1, 1, 3); addEdge(g1, 2, 4);
    printf("Graph: Connected graph with 5 vertices.\n");
    int size1;
    int* res1 = bfs(g1, 0, &size1);
    printf("BFS starting from node 0: ");
    printArray(res1, size1);
    free(res1);
    freeGraph(g1);

    // Test Case 2: Different start node on a more complex graph
    printf("\n--- Test Case 2 ---\n");
    Graph* g2 = createGraph(5);
    addEdge(g2, 0, 1); addEdge(g2, 0, 2); addEdge(g2, 1, 2); addEdge(g2, 1, 3); addEdge(g2, 2, 4);
    printf("Graph: More complex connected graph.\n");
    int size2;
    int* res2 = bfs(g2, 3, &size2);
    printf("BFS starting from node 3: ");
    printArray(res2, size2);
    free(res2);
    freeGraph(g2);

    // Test Case 3: Disconnected graph
    printf("\n--- Test Case 3 ---\n");
    Graph* g3 = createGraph(7);
    addEdge(g3, 0, 1); addEdge(g3, 0, 2); // C1
    addEdge(g3, 3, 4);                   // C2
    addEdge(g3, 5, 6);                   // C3
    printf("Graph: Disconnected graph with 7 vertices.\n");
    int size3;
    int* res3 = bfs(g3, 3, &size3);
    printf("BFS starting from node 3: ");
    printArray(res3, size3);
    free(res3);
    freeGraph(g3);
    
    // Test Case 4: Single node graph
    printf("\n--- Test Case 4 ---\n");
    Graph* g4 = createGraph(1);
    printf("Graph: Single node graph.\n");
    int size4;
    int* res4 = bfs(g4, 0, &size4);
    printf("BFS starting from node 0: ");
    printArray(res4, size4);
    free(res4);
    freeGraph(g4);
    
    // Test Case 5: Linear graph
    printf("\n--- Test Case 5 ---\n");
    Graph* g5 = createGraph(5);
    addEdge(g5, 0, 1); addEdge(g5, 1, 2); addEdge(g5, 2, 3); addEdge(g5, 3, 4);
    printf("Graph: Linear graph with 5 vertices.\n");
    int size5;
    int* res5 = bfs(g5, 2, &size5);
    printf("BFS starting from node 2: ");
    printArray(res5, size5);
    free(res5);
    freeGraph(g5);
    
    return 0;
}