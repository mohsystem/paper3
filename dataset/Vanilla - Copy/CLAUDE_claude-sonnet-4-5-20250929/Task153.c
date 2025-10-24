
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

typedef struct Graph {
    int vertices;
    Node** adjacencyList;
} Graph;

typedef struct Queue {
    int* items;
    int front;
    int rear;
    int capacity;
} Queue;

Queue* createQueue(int capacity) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = -1;
    queue->items = (int*)malloc(capacity * sizeof(int));
    return queue;
}

bool isEmpty(Queue* queue) {
    return queue->rear < queue->front;
}

void enqueue(Queue* queue, int value) {
    queue->items[++queue->rear] = value;
}

int dequeue(Queue* queue) {
    return queue->items[queue->front++];
}

Graph* createGraph(int vertices) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->vertices = vertices;
    graph->adjacencyList = (Node**)malloc(vertices * sizeof(Node*));
    for (int i = 0; i < vertices; i++) {
        graph->adjacencyList[i] = NULL;
    }
    return graph;
}

void addEdge(Graph* graph, int source, int destination) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->vertex = destination;
    newNode->next = graph->adjacencyList[source];
    graph->adjacencyList[source] = newNode;
}

int* breadthFirstSearch(int vertices, int edges[][2], int edgeCount, int startNode, int* resultSize) {
    Graph* graph = createGraph(vertices);
    for (int i = 0; i < edgeCount; i++) {
        addEdge(graph, edges[i][0], edges[i][1]);
    }
    
    int* result = (int*)malloc(vertices * sizeof(int));
    bool* visited = (bool*)calloc(vertices, sizeof(bool));
    Queue* queue = createQueue(vertices);
    
    *resultSize = 0;
    visited[startNode] = true;
    enqueue(queue, startNode);
    
    while (!isEmpty(queue)) {
        int current = dequeue(queue);
        result[(*resultSize)++] = current;
        
        Node* temp = graph->adjacencyList[current];
        while (temp != NULL) {
            int neighbor = temp->vertex;
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                enqueue(queue, neighbor);
            }
            temp = temp->next;
        }
    }
    
    free(visited);
    free(queue->items);
    free(queue);
    for (int i = 0; i < vertices; i++) {
        Node* temp = graph->adjacencyList[i];
        while (temp != NULL) {
            Node* toFree = temp;
            temp = temp->next;
            free(toFree);
        }
    }
    free(graph->adjacencyList);
    free(graph);
    
    return result;
}

void printArray(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]\\n");
}

int main() {
    int resultSize;
    
    // Test Case 1: Simple linear graph
    printf("Test Case 1:\\n");
    int edges1[][2] = {{0, 1}, {1, 2}, {2, 3}};
    int* result1 = breadthFirstSearch(4, edges1, 3, 0, &resultSize);
    printArray(result1, resultSize);
    free(result1);
    
    // Test Case 2: Graph with multiple branches
    printf("\\nTest Case 2:\\n");
    int edges2[][2] = {{0, 1}, {0, 2}, {1, 3}, {1, 4}, {2, 5}};
    int* result2 = breadthFirstSearch(6, edges2, 5, 0, &resultSize);
    printArray(result2, resultSize);
    free(result2);
    
    // Test Case 3: Disconnected graph
    printf("\\nTest Case 3:\\n");
    int edges3[][2] = {{0, 1}, {2, 3}};
    int* result3 = breadthFirstSearch(4, edges3, 2, 0, &resultSize);
    printArray(result3, resultSize);
    free(result3);
    
    // Test Case 4: Graph with cycles
    printf("\\nTest Case 4:\\n");
    int edges4[][2] = {{0, 1}, {0, 2}, {1, 2}, {2, 0}, {2, 3}};
    int* result4 = breadthFirstSearch(4, edges4, 5, 0, &resultSize);
    printArray(result4, resultSize);
    free(result4);
    
    // Test Case 5: Single node
    printf("\\nTest Case 5:\\n");
    int edges5[][2] = {};
    int* result5 = breadthFirstSearch(1, edges5, 0, 0, &resultSize);
    printArray(result5, resultSize);
    free(result5);
    
    return 0;
}
