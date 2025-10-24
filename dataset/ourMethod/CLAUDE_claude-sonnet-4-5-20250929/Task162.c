
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

/* Maximum nodes and edges to prevent excessive memory allocation */
#define MAX_NODES 10000
#define MAX_EDGES 100000

/* Structure to represent an edge */
typedef struct Edge {
    int destination;
    int weight;
    struct Edge* next;
} Edge;

/* Structure for adjacency list */
typedef struct Graph {
    int numNodes;
    Edge** adjacencyList;  /* Array of edge list heads */
    int edgeCount;         /* Track total edges */
} Graph;

/* Structure for priority queue node */
typedef struct PQNode {
    int node;
    int distance;
} PQNode;

/* Structure for min-heap priority queue */
typedef struct PriorityQueue {
    PQNode* heap;
    int size;
    int capacity;
} PriorityQueue;

/* Initialize graph with validation */
Graph* createGraph(int numNodes) {
    /* Validate node count */
    if (numNodes <= 0 || numNodes > MAX_NODES) {
        fprintf(stderr, "Error: Number of nodes must be between 1 and %d\\n", MAX_NODES);
        return NULL;
    }
    
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (graph == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for graph\\n");
        return NULL;
    }
    
    graph->numNodes = numNodes;
    graph->edgeCount = 0;
    
    /* Allocate adjacency list with NULL initialization */
    graph->adjacencyList = (Edge**)calloc(numNodes, sizeof(Edge*));
    if (graph->adjacencyList == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for adjacency list\\n");
        free(graph);
        return NULL;
    }
    
    return graph;
}

/* Add edge with validation */
bool addEdge(Graph* graph, int source, int destination, int weight) {
    /* Validate graph pointer */
    if (graph == NULL) {
        fprintf(stderr, "Error: Graph is NULL\\n");
        return false;
    }
    
    /* Validate source and destination bounds */
    if (source < 0 || source >= graph->numNodes) {
        fprintf(stderr, "Error: Source node out of range\\n");
        return false;
    }
    if (destination < 0 || destination >= graph->numNodes) {
        fprintf(stderr, "Error: Destination node out of range\\n");
        return false;
    }
    if (weight < 0) {
        fprintf(stderr, "Error: Weight must be non-negative\\n");
        return false;
    }
    
    /* Check edge count limit */
    if (graph->edgeCount >= MAX_EDGES) {
        fprintf(stderr, "Error: Maximum edge limit exceeded\\n");
        return false;
    }
    
    /* Allocate new edge */
    Edge* newEdge = (Edge*)malloc(sizeof(Edge));
    if (newEdge == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for edge\\n");
        return false;
    }
    
    newEdge->destination = destination;
    newEdge->weight = weight;
    newEdge->next = graph->adjacencyList[source];
    graph->adjacencyList[source] = newEdge;
    graph->edgeCount++;
    
    return true;
}

/* Priority queue functions */
PriorityQueue* createPriorityQueue(int capacity) {
    if (capacity <= 0 || capacity > MAX_NODES * 10) {
        return NULL;
    }
    
    PriorityQueue* pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    if (pq == NULL) {
        return NULL;
    }
    
    pq->heap = (PQNode*)malloc(sizeof(PQNode) * capacity);
    if (pq->heap == NULL) {
        free(pq);
        return NULL;
    }
    
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

void swap(PQNode* a, PQNode* b) {
    PQNode temp = *a;
    *a = *b;
    *b = temp;
}

void heapifyUp(PriorityQueue* pq, int index) {
    if (pq == NULL || index < 0 || index >= pq->size) {
        return;
    }
    
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (pq->heap[index].distance < pq->heap[parent].distance) {
            swap(&pq->heap[index], &pq->heap[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

void heapifyDown(PriorityQueue* pq, int index) {
    if (pq == NULL || index < 0 || index >= pq->size) {
        return;
    }
    
    while (true) {
        int smallest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        
        if (left < pq->size && pq->heap[left].distance < pq->heap[smallest].distance) {
            smallest = left;
        }
        if (right < pq->size && pq->heap[right].distance < pq->heap[smallest].distance) {
            smallest = right;
        }
        
        if (smallest != index) {
            swap(&pq->heap[index], &pq->heap[smallest]);
            index = smallest;
        } else {
            break;
        }
    }
}

bool pqPush(PriorityQueue* pq, int node, int distance) {
    if (pq == NULL || pq->size >= pq->capacity) {
        return false;
    }
    
    pq->heap[pq->size].node = node;
    pq->heap[pq->size].distance = distance;
    heapifyUp(pq, pq->size);
    pq->size++;
    return true;
}

bool pqPop(PriorityQueue* pq, int* node, int* distance) {
    if (pq == NULL || pq->size == 0) {
        return false;
    }
    
    *node = pq->heap[0].node;
    *distance = pq->heap[0].distance;
    
    pq->size--;
    if (pq->size > 0) {
        pq->heap[0] = pq->heap[pq->size];
        heapifyDown(pq, 0);
    }
    
    return true;
}

void freePriorityQueue(PriorityQueue* pq) {
    if (pq != NULL) {
        if (pq->heap != NULL) {
            free(pq->heap);
            pq->heap = NULL;
        }
        free(pq);
    }
}

/* Dijkstra's algorithm implementation */
int* dijkstra(Graph* graph, int startNode) {
    /* Validate inputs */
    if (graph == NULL) {
        fprintf(stderr, "Error: Graph is NULL\\n");
        return NULL;
    }
    if (startNode < 0 || startNode >= graph->numNodes) {
        fprintf(stderr, "Error: Start node out of range\\n");
        return NULL;
    }
    
    /* Allocate distance array */
    int* distances = (int*)malloc(sizeof(int) * graph->numNodes);
    if (distances == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for distances\\n");
        return NULL;
    }
    
    /* Allocate visited array */
    bool* visited = (bool*)calloc(graph->numNodes, sizeof(bool));
    if (visited == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for visited array\\n");
        free(distances);
        return NULL;
    }
    
    /* Initialize distances to maximum value */
    for (int i = 0; i < graph->numNodes; i++) {
        distances[i] = INT_MAX;
    }
    distances[startNode] = 0;
    
    /* Create priority queue */
    PriorityQueue* pq = createPriorityQueue(graph->numNodes * 10);
    if (pq == NULL) {
        fprintf(stderr, "Error: Failed to create priority queue\\n");
        free(distances);
        free(visited);
        return NULL;
    }
    
    if (!pqPush(pq, startNode, 0)) {
        fprintf(stderr, "Error: Failed to push to priority queue\\n");
        freePriorityQueue(pq);
        free(distances);
        free(visited);
        return NULL;
    }
    
    /* Main Dijkstra loop */
    while (pq->size > 0) {
        int currentNode, currentDistance;
        if (!pqPop(pq, &currentNode, &currentDistance)) {
            break;
        }
        
        /* Bounds check for current node */
        if (currentNode < 0 || currentNode >= graph->numNodes) {
            continue;
        }
        
        if (visited[currentNode]) {
            continue;
        }
        visited[currentNode] = true;
        
        /* Explore neighbors */
        Edge* edge = graph->adjacencyList[currentNode];
        while (edge != NULL) {
            int neighbor = edge->destination;
            int weight = edge->weight;
            
            /* Bounds check for neighbor */
            if (neighbor >= 0 && neighbor < graph->numNodes) {
                /* Check for integer overflow before addition */
                if (distances[currentNode] != INT_MAX &&
                    weight <= INT_MAX - distances[currentNode]) {
                    
                    int newDistance = distances[currentNode] + weight;
                    
                    /* Relaxation step */
                    if (newDistance < distances[neighbor]) {
                        distances[neighbor] = newDistance;
                        pqPush(pq, neighbor, newDistance);
                    }
                }
            }
            
            edge = edge->next;
        }
    }
    
    /* Cleanup */
    freePriorityQueue(pq);
    free(visited);
    
    return distances;
}

/* Free graph memory */
void freeGraph(Graph* graph) {
    if (graph == NULL) {
        return;
    }
    
    if (graph->adjacencyList != NULL) {
        for (int i = 0; i < graph->numNodes; i++) {
            Edge* current = graph->adjacencyList[i];
            while (current != NULL) {
                Edge* next = current->next;
                free(current);
                current = next;
            }
        }
        free(graph->adjacencyList);
        graph->adjacencyList = NULL;
    }
    
    free(graph);
}

int main(void) {
    /* Test Case 1: Simple graph */
    printf("Test Case 1: Simple graph\\n");
    Graph* graph1 = createGraph(5);
    if (graph1 != NULL) {
        addEdge(graph1, 0, 1, 4);
        addEdge(graph1, 0, 2, 1);
        addEdge(graph1, 2, 1, 2);
        addEdge(graph1, 1, 3, 1);
        addEdge(graph1, 2, 3, 5);
        addEdge(graph1, 3, 4, 3);
        
        int* distances1 = dijkstra(graph1, 0);
        if (distances1 != NULL) {
            for (int i = 0; i < graph1->numNodes; i++) {
                printf("Distance from 0 to %d: ", i);
                if (distances1[i] == INT_MAX) {
                    printf("INF\\n");
                } else {
                    printf("%d\\n", distances1[i]);
                }
            }
            free(distances1);
        }
        freeGraph(graph1);
    }
    
    /* Test Case 2: Disconnected graph */
    printf("\\nTest Case 2: Disconnected graph\\n");
    Graph* graph2 = createGraph(4);
    if (graph2 != NULL) {
        addEdge(graph2, 0, 1, 5);
        addEdge(graph2, 1, 2, 3);
        
        int* distances2 = dijkstra(graph2, 0);
        if (distances2 != NULL) {
            for (int i = 0; i < graph2->numNodes; i++) {
                printf("Distance from 0 to %d: ", i);
                if (distances2[i] == INT_MAX) {
                    printf("INF\\n");
                } else {
                    printf("%d\\n", distances2[i]);
                }
            }
            free(distances2);
        }
        freeGraph(graph2);
    }
    
    /* Test Case 3: Single node */
    printf("\\nTest Case 3: Single node\\n");
    Graph* graph3 = createGraph(1);
    if (graph3 != NULL) {
        int* distances3 = dijkstra(graph3, 0);
        if (distances3 != NULL) {
            printf("Distance from 0 to 0: %d\\n", distances3[0]);
            free(distances3);
        }
        freeGraph(graph3);
    }
    
    /* Test Case 4: Multiple paths */
    printf("\\nTest Case 4: Multiple paths\\n");
    Graph* graph4 = createGraph(6);
    if (graph4 != NULL) {
        addEdge(graph4, 0, 1, 7);
        addEdge(graph4, 0, 2, 9);
        addEdge(graph4, 0, 5, 14);
        addEdge(graph4, 1, 2, 10);
        addEdge(graph4, 1, 3, 15);
        addEdge(graph4, 2, 3, 11);
        addEdge(graph4, 2, 5, 2);
        addEdge(graph4, 3, 4, 6);
        addEdge(graph4, 4, 5, 9);
        
        int* distances4 = dijkstra(graph4, 0);
        if (distances4 != NULL) {
            for (int i = 0; i < graph4->numNodes; i++) {
                printf("Distance from 0 to %d: %d\\n", i, distances4[i]);
            }
            free(distances4);
        }
        freeGraph(graph4);
    }
    
    /* Test Case 5: Linear chain */
    printf("\\nTest Case 5: Linear chain\\n");
    Graph* graph5 = createGraph(4);
    if (graph5 != NULL) {
        addEdge(graph5, 0, 1, 1);
        addEdge(graph5, 1, 2, 2);
        addEdge(graph5, 2, 3, 3);
        
        int* distances5 = dijkstra(graph5, 0);
        if (distances5 != NULL) {
            for (int i = 0; i < graph5->numNodes; i++) {
                printf("Distance from 0 to %d: %d\\n", i, distances5[i]);
            }
            free(distances5);
        }
        freeGraph(graph5);
    }
    
    return 0;
}
