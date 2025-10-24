#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct AdjListNode {
    int dest;
    struct AdjListNode* next;
} AdjListNode;

typedef struct AdjList {
    AdjListNode *head;
} AdjList;

typedef struct Graph {
    int numVertices;
    AdjList* array;
} Graph;

typedef struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
} Queue;

AdjListNode* newAdjListNode(int dest) {
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (newNode == NULL) {
        perror("Failed to allocate AdjListNode");
        return NULL;
    }
    newNode->dest = dest;
    newNode->next = NULL;
    return newNode;
}

Graph* createGraph(int numVertices) {
    if (numVertices < 0) {
        fprintf(stderr, "Error: Number of vertices cannot be negative.\n");
        return NULL;
    }
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (graph == NULL) {
        perror("Failed to allocate Graph");
        return NULL;
    }
    graph->numVertices = numVertices;
    graph->array = (AdjList*)malloc(numVertices * sizeof(AdjList));
    if (graph->array == NULL) {
        perror("Failed to allocate adjacency list array");
        free(graph);
        return NULL;
    }
    for (int i = 0; i < numVertices; ++i) {
        graph->array[i].head = NULL;
    }
    return graph;
}

void addEdge(Graph* graph, int src, int dest) {
    if (graph == NULL || src < 0 || src >= graph->numVertices || dest < 0 || dest >= graph->numVertices) {
        fprintf(stderr, "Error: Invalid vertex for edge (%d, %d)\n", src, dest);
        return;
    }
    AdjListNode* newNode = newAdjListNode(dest);
    if (newNode == NULL) return;
    newNode->next = graph->array[src].head;
    graph->array[src].head = newNode;
}

void destroyGraph(Graph* graph) {
    if (graph == NULL) return;
    for (int i = 0; i < graph->numVertices; i++) {
        AdjListNode* current = graph->array[i].head;
        while (current != NULL) {
            AdjListNode* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(graph->array);
    free(graph);
}

Queue* createQueue(unsigned capacity) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if (queue == NULL) return NULL;
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity > 0 ? capacity - 1 : 0;
    queue->array = (int*)malloc(queue->capacity * sizeof(int));
    if (queue->array == NULL) {
        free(queue);
        return NULL;
    }
    return queue;
}

bool isQueueEmpty(Queue* queue) { return (queue == NULL || queue->size == 0); }
void enqueue(Queue* queue, int item) {
    if (queue == NULL || queue->size == queue->capacity) return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}
int dequeue(Queue* queue) {
    if (isQueueEmpty(queue)) return -1;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
void destroyQueue(Queue* queue) {
    if (queue == NULL) return;
    free(queue->array);
    free(queue);
}

int* bfs(Graph* graph, int startNode, int* resultSize) {
    *resultSize = 0;
    if (graph == NULL || startNode < 0 || startNode >= graph->numVertices) {
        fprintf(stderr, "Error: Invalid graph or start node %d is out of bounds.\n", startNode);
        return NULL;
    }

    bool* visited = (bool*)calloc(graph->numVertices, sizeof(bool));
    if (visited == NULL) return NULL;

    int* result = (int*)malloc(graph->numVertices * sizeof(int));
    if (result == NULL) { free(visited); return NULL; }

    Queue* queue = createQueue(graph->numVertices);
    if (queue == NULL) { free(visited); free(result); return NULL; }

    visited[startNode] = true;
    enqueue(queue, startNode);

    while (!isQueueEmpty(queue)) {
        int currentNode = dequeue(queue);
        result[*resultSize] = currentNode;
        (*resultSize)++;

        AdjListNode* temp = graph->array[currentNode].head;
        while (temp != NULL) {
            int adjVertex = temp->dest;
            if (adjVertex >= 0 && adjVertex < graph->numVertices && !visited[adjVertex]) {
                visited[adjVertex] = true;
                enqueue(queue, adjVertex);
            }
            temp = temp->next;
        }
    }
    free(visited);
    destroyQueue(queue);
    
    int* finalResult = (int*)realloc(result, *resultSize * sizeof(int));
    if (finalResult == NULL && *resultSize > 0) {
        free(result);
        *resultSize = 0;
        return NULL;
    }
    return finalResult;
}

void printArray(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]\n");
}

int main() {
    // Test Case 1: Simple connected graph
    printf("Test Case 1: Simple connected graph\n");
    Graph* g1 = createGraph(6);
    if(g1){
        addEdge(g1, 0, 1); addEdge(g1, 0, 2); addEdge(g1, 1, 3);
        addEdge(g1, 2, 4); addEdge(g1, 3, 5);
        int size1 = 0;
        int* res1 = bfs(g1, 0, &size1);
        printf("BFS starting from node 0: "); printArray(res1, size1);
        free(res1);
        destroyGraph(g1);
    }

    // Test Case 2: Start from a different node
    printf("\nTest Case 2: Start from a different node\n");
    Graph* g2 = createGraph(4);
    if(g2){
        addEdge(g2, 0, 1); addEdge(g2, 0, 2); addEdge(g2, 1, 2);
        addEdge(g2, 2, 0); addEdge(g2, 2, 3); addEdge(g2, 3, 3);
        int size2 = 0;
        int* res2 = bfs(g2, 2, &size2);
        printf("BFS starting from node 2: "); printArray(res2, size2);
        free(res2);
        destroyGraph(g2);
    }
    
    // Test Case 3: Graph with a cycle
    printf("\nTest Case 3: Graph with a cycle\n");
    Graph* g3 = createGraph(3);
    if(g3){
        addEdge(g3, 0, 1); addEdge(g3, 1, 2); addEdge(g3, 2, 0);
        int size3 = 0;
        int* res3 = bfs(g3, 0, &size3);
        printf("BFS starting from node 0: "); printArray(res3, size3);
        free(res3);
        destroyGraph(g3);
    }

    // Test Case 4: Disconnected graph
    printf("\nTest Case 4: Disconnected graph\n");
    Graph* g4 = createGraph(5);
    if(g4){
        addEdge(g4, 0, 1); addEdge(g4, 1, 2); addEdge(g4, 3, 4);
        int size4a = 0, size4b = 0;
        int* res4a = bfs(g4, 0, &size4a);
        printf("BFS starting from node 0: "); printArray(res4a, size4a);
        free(res4a);
        int* res4b = bfs(g4, 3, &size4b);
        printf("BFS starting from node 3: "); printArray(res4b, size4b);
        free(res4b);
        destroyGraph(g4);
    }

    // Test Case 5: Invalid start node
    printf("\nTest Case 5: Invalid start node\n");
    Graph* g5 = createGraph(3);
    if(g5){
        addEdge(g5, 0, 1);
        int size5 = 0;
        int* res5 = bfs(g5, 5, &size5);
        printf("BFS starting from node 5: "); printArray(res5, size5);
        free(res5);
        destroyGraph(g5);
    }

    return 0;
}