#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// --- Graph Structure ---
typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

typedef struct Graph {
    int numVertices;
    Node** adjLists;
} Graph;

Node* createNode(int v) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Unable to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    newNode->vertex = v;
    newNode->next = NULL;
    return newNode;
}

Graph* createGraph(int vertices) {
    if (vertices <= 0) return NULL;
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (graph == NULL) {
        perror("Unable to allocate memory for graph");
        exit(EXIT_FAILURE);
    }
    graph->numVertices = vertices;
    graph->adjLists = (Node**)calloc(vertices, sizeof(Node*));
    if (graph->adjLists == NULL) {
        perror("Unable to allocate memory for adjacency lists");
        free(graph);
        exit(EXIT_FAILURE);
    }
    return graph;
}

void addEdge(Graph* graph, int src, int dest) {
    if (graph == NULL || src < 0 || dest < 0 || src >= graph->numVertices || dest >= graph->numVertices) {
        return;
    }
    // Add edge from src to dest
    Node* newNode = createNode(dest);
    newNode->next = graph->adjLists[src];
    graph->adjLists[src] = newNode;

    // Add edge from dest to src (for undirected graph)
    newNode = createNode(src);
    newNode->next = graph->adjLists[dest];
    graph->adjLists[dest] = newNode;
}

void freeGraph(Graph* graph) {
    if (graph == NULL) return;
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

// --- Queue Structure for BFS ---
typedef struct Queue {
    int* items;
    int front;
    int rear;
    int capacity;
} Queue;

Queue* createQueue(int capacity) {
    if (capacity <= 0) return NULL;
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (q == NULL) return NULL;
    q->items = (int*)malloc(capacity * sizeof(int));
    if (q->items == NULL) {
        free(q);
        return NULL;
    }
    q->capacity = capacity;
    q->front = -1;
    q->rear = -1;
    return q;
}

bool isEmpty(Queue* q) {
    return q->front == -1;
}

void enqueue(Queue* q, int value) {
    if (q == NULL || q->rear == q->capacity - 1) {
        // In a real-world scenario, you might reallocate, but for this example we assume capacity is sufficient.
        fprintf(stderr, "Queue is full\n");
        return;
    }
    if (q->front == -1) {
        q->front = 0;
    }
    q->rear++;
    q->items[q->rear] = value;
}

int dequeue(Queue* q) {
    if (q == NULL || isEmpty(q)) {
        fprintf(stderr, "Queue is empty\n");
        return -1; // Error code
    }
    int item = q->items[q->front];
    q->front++;
    if (q->front > q->rear) {
        q->front = q->rear = -1;
    }
    return item;
}

void freeQueue(Queue* q) {
    if (q != NULL) {
        free(q->items);
        free(q);
    }
}

/**
 * @brief Performs Breadth-First Search on a graph.
 * @param graph The graph to traverse.
 * @param startNode The starting vertex.
 * @param traversalSize Pointer to an int to store the size of the traversal array.
 * @return A dynamically allocated array with the traversal order. The caller MUST free this array.
 */
int* bfs(Graph* graph, int startNode, int* traversalSize) {
    *traversalSize = 0;
    // Input validation
    if (graph == NULL || startNode < 0 || startNode >= graph->numVertices) {
        fprintf(stderr, "Error: Invalid input. Graph is null or startNode is out of bounds.\n");
        return NULL;
    }

    Queue* q = createQueue(graph->numVertices);
    if (q == NULL) {
        perror("Failed to create queue");
        return NULL;
    }
    
    bool* visited = (bool*)calloc(graph->numVertices, sizeof(bool));
    if (visited == NULL) {
        perror("Failed to allocate visited array");
        freeQueue(q);
        return NULL;
    }

    int* traversalResult = (int*)malloc(graph->numVertices * sizeof(int));
    if (traversalResult == NULL) {
        perror("Failed to allocate traversal result array");
        free(visited);
        freeQueue(q);
        return NULL;
    }

    visited[startNode] = true;
    enqueue(q, startNode);

    while (!isEmpty(q)) {
        int currentVertex = dequeue(q);
        if (*traversalSize < graph->numVertices) {
            traversalResult[*traversalSize] = currentVertex;
            (*traversalSize)++;
        }

        Node* temp = graph->adjLists[currentVertex];
        while (temp) {
            int adjVertex = temp->vertex;
            if (!visited[adjVertex]) {
                visited[adjVertex] = true;
                enqueue(q, adjVertex);
            }
            temp = temp->next;
        }
    }

    free(visited);
    freeQueue(q);
    return traversalResult;
}

void printTraversal(int* arr, int size) {
    printf(" -> Traversal: [");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]\n");
}


int main() {
    int V = 7;
    Graph* graph = createGraph(V);

    // Creating a sample graph
    //      0 --- 1
    //      | \   |
    //      |  \  |
    //      2---3 4 --- 6
    //      |
    //      5
    addEdge(graph, 0, 1);
    addEdge(graph, 0, 2);
    addEdge(graph, 0, 3);
    addEdge(graph, 1, 4);
    addEdge(graph, 2, 3);
    addEdge(graph, 2, 5);
    addEdge(graph, 4, 6);

    // --- Test Cases ---
    int testStarts[] = {0, 3, 5, 6, 99}; // 99 is an invalid node

    for (int i = 0; i < 5; ++i) {
        int startNode = testStarts[i];
        printf("Test Case %d: BFS starting from node %d\n", i + 1, startNode);
        
        int traversalSize = 0;
        // The caller is responsible for freeing the 'result' array.
        int* result = bfs(graph, startNode, &traversalSize);

        if (result == NULL) {
            printf(" -> Handled invalid start node correctly.\n");
        } else {
            printTraversal(result, traversalSize);
            free(result); // Free the memory allocated by bfs()
        }
        printf("--------------------\n");
    }

    freeGraph(graph);
    return 0;
}