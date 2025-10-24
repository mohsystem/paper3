
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int data;
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

void enqueue(Queue* queue, int item) {
    queue->items[++queue->rear] = item;
}

int dequeue(Queue* queue) {
    return queue->items[queue->front++];
}

void freeQueue(Queue* queue) {
    free(queue->items);
    free(queue);
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
    if (source >= 0 && source < graph->vertices && 
        destination >= 0 && destination < graph->vertices) {
        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->data = destination;
        newNode->next = graph->adjacencyList[source];
        graph->adjacencyList[source] = newNode;
    }
}

int* breadthFirstSearch(Graph* graph, int startNode, int* resultSize) {
    *resultSize = 0;
    if (startNode < 0 || startNode >= graph->vertices) {
        return NULL;
    }
    
    int* result = (int*)malloc(graph->vertices * sizeof(int));
    bool* visited = (bool*)calloc(graph->vertices, sizeof(bool));
    Queue* queue = createQueue(graph->vertices);
    
    visited[startNode] = true;
    enqueue(queue, startNode);
    
    while (!isEmpty(queue)) {
        int node = dequeue(queue);
        result[(*resultSize)++] = node;
        
        Node* temp = graph->adjacencyList[node];
        while (temp != NULL) {
            int neighbor = temp->data;
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                enqueue(queue, neighbor);
            }
            temp = temp->next;
        }
    }
    
    free(visited);
    freeQueue(queue);
    return result;
}

void freeGraph(Graph* graph) {
    for (int i = 0; i < graph->vertices; i++) {
        Node* temp = graph->adjacencyList[i];
        while (temp != NULL) {
            Node* toFree = temp;
            temp = temp->next;
            free(toFree);
        }
    }
    free(graph->adjacencyList);
    free(graph);
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
    // Test Case 1: Simple linear graph
    printf("Test Case 1: Linear Graph\\n");
    Graph* graph1 = createGraph(4);
    addEdge(graph1, 0, 1);
    addEdge(graph1, 1, 2);
    addEdge(graph1, 2, 3);
    int size1;
    int* result1 = breadthFirstSearch(graph1, 0, &size1);
    printf("BFS from node 0: ");
    printArray(result1, size1);
    free(result1);
    freeGraph(graph1);
    
    // Test Case 2: Graph with multiple branches
    printf("\\nTest Case 2: Branching Graph\\n");
    Graph* graph2 = createGraph(6);
    addEdge(graph2, 0, 1);
    addEdge(graph2, 0, 2);
    addEdge(graph2, 1, 3);
    addEdge(graph2, 1, 4);
    addEdge(graph2, 2, 5);
    int size2;
    int* result2 = breadthFirstSearch(graph2, 0, &size2);
    printf("BFS from node 0: ");
    printArray(result2, size2);
    free(result2);
    freeGraph(graph2);
    
    // Test Case 3: Graph with cycle
    printf("\\nTest Case 3: Graph with Cycle\\n");
    Graph* graph3 = createGraph(5);
    addEdge(graph3, 0, 1);
    addEdge(graph3, 1, 2);
    addEdge(graph3, 2, 3);
    addEdge(graph3, 3, 1);
    addEdge(graph3, 0, 4);
    int size3;
    int* result3 = breadthFirstSearch(graph3, 0, &size3);
    printf("BFS from node 0: ");
    printArray(result3, size3);
    free(result3);
    freeGraph(graph3);
    
    // Test Case 4: Disconnected graph
    printf("\\nTest Case 4: Disconnected Graph\\n");
    Graph* graph4 = createGraph(5);
    addEdge(graph4, 0, 1);
    addEdge(graph4, 2, 3);
    addEdge(graph4, 3, 4);
    int size4;
    int* result4 = breadthFirstSearch(graph4, 0, &size4);
    printf("BFS from node 0: ");
    printArray(result4, size4);
    free(result4);
    freeGraph(graph4);
    
    // Test Case 5: Single node
    printf("\\nTest Case 5: Single Node\\n");
    Graph* graph5 = createGraph(1);
    int size5;
    int* result5 = breadthFirstSearch(graph5, 0, &size5);
    printf("BFS from node 0: ");
    printArray(result5, size5);
    free(result5);
    freeGraph(graph5);
    
    return 0;
}
