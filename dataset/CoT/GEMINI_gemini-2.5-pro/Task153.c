#include <stdio.h>
#include <stdlib.h>

// --- Graph Structure (Adjacency List) ---
typedef struct AdjListNode {
    int dest;
    struct AdjListNode* next;
} AdjListNode;

typedef struct AdjList {
    AdjListNode* head;
} AdjList;

typedef struct Graph {
    int V;
    AdjList* array;
} Graph;

AdjListNode* newAdjListNode(int dest) {
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (!newNode) {
        perror("Failed to allocate memory for AdjListNode");
        exit(EXIT_FAILURE);
    }
    newNode->dest = dest;
    newNode->next = NULL;
    return newNode;
}

Graph* createGraph(int V) {
    if (V < 0) return NULL;
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (!graph) {
        perror("Failed to allocate memory for Graph");
        exit(EXIT_FAILURE);
    }
    graph->V = V;
    graph->array = (AdjList*)malloc(V * sizeof(AdjList));
     if (!graph->array) {
        perror("Failed to allocate memory for adjacency list array");
        free(graph);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < V; ++i) {
        graph->array[i].head = NULL;
    }
    return graph;
}

void addEdge(Graph* graph, int src, int dest) {
    if (!graph || src < 0 || src >= graph->V || dest < 0 || dest >= graph->V) return;
    // Add an edge from src to dest
    AdjListNode* newNode = newAdjListNode(dest);
    newNode->next = graph->array[src].head;
    graph->array[src].head = newNode;

    // Add an edge from dest to src (for undirected graph)
    newNode = newAdjListNode(src);
    newNode->next = graph->array[dest].head;
    graph->array[dest].head = newNode;
}

void freeGraph(Graph* graph) {
    if (!graph) return;
    for (int v = 0; v < graph->V; ++v) {
        AdjListNode* pCrawl = graph->array[v].head;
        while (pCrawl) {
            AdjListNode* temp = pCrawl;
            pCrawl = pCrawl->next;
            free(temp);
        }
    }
    free(graph->array);
    free(graph);
}

// --- Queue Structure for BFS ---
typedef struct QueueNode {
    int data;
    struct QueueNode* next;
} QueueNode;

typedef struct Queue {
    QueueNode *front, *rear;
} Queue;

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        perror("Failed to allocate memory for Queue");
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    return q;
}

void enqueue(Queue* q, int data) {
    QueueNode* temp = (QueueNode*)malloc(sizeof(QueueNode));
    if (!temp) {
        perror("Failed to allocate memory for QueueNode");
        exit(EXIT_FAILURE);
    }
    temp->data = data;
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
    QueueNode* temp = q->front;
    int data = temp->data;
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    return data;
}

int isQueueEmpty(Queue* q) {
    return q->front == NULL;
}

// --- BFS Function ---
// This function prints the BFS traversal. It does not return a list to simplify memory management in C.
void bfs(Graph* graph, int startNode) {
    if (!graph || startNode < 0 || startNode >= graph->V) {
        fprintf(stderr, "Error: Invalid graph or start node.\n");
        return;
    }
    
    // Using calloc to initialize all elements to 0 (false)
    int* visited = (int*)calloc(graph->V, sizeof(int));
    if (!visited) {
        perror("Failed to allocate memory for visited array");
        return;
    }
    
    Queue* q = createQueue();

    visited[startNode] = 1;
    enqueue(q, startNode);

    while (!isQueueEmpty(q)) {
        int currentNode = dequeue(q);
        printf("%d ", currentNode);

        AdjListNode* pCrawl = graph->array[currentNode].head;
        while (pCrawl) {
            int adjNode = pCrawl->dest;
            if (!visited[adjNode]) {
                visited[adjNode] = 1;
                enqueue(q, adjNode);
            }
            pCrawl = pCrawl->next;
        }
    }
    
    free(visited);
    free(q); // Free the queue structure itself
}

int main() {
    printf("--- Breadth-First Search (BFS) Traversal ---\n");

    // Graph structure for all test cases
    //        0
    //       / \
    //      1---2
    //     / \ |
    //    3---4 5
    //         |
    //         6
    int numVertices = 7;
    Graph* g = createGraph(numVertices);
    addEdge(g, 0, 1);
    addEdge(g, 0, 2);
    addEdge(g, 1, 3);
    addEdge(g, 1, 4);
    addEdge(g, 2, 5);
    addEdge(g, 3, 4);
    addEdge(g, 5, 6);

    int startNodes[] = {0, 3, 6, 4, 1};
    int numTestCases = sizeof(startNodes) / sizeof(startNodes[0]);

    for (int i = 0; i < numTestCases; ++i) {
        printf("\nTest Case %d: Starting from node %d\n", i + 1, startNodes[i]);
        printf("BFS Traversal: ");
        bfs(g, startNodes[i]);
        printf("\n");
    }

    freeGraph(g);
    return 0;
}