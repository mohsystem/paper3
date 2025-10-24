
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_NODES 1000
#define MAX_NEIGHBORS 100

typedef struct Node {
    int value;
    int neighbors[MAX_NEIGHBORS];
    int neighborCount;
} Node;

typedef struct Graph {
    Node nodes[MAX_NODES];
    int nodeCount;
} Graph;

typedef struct Queue {
    int items[MAX_NODES];
    int front;
    int rear;
} Queue;

void initGraph(Graph* graph) {
    if (graph == NULL) return;
    graph->nodeCount = 0;
    memset(graph->nodes, 0, sizeof(graph->nodes));
}

void initQueue(Queue* queue) {
    if (queue == NULL) return;
    queue->front = -1;
    queue->rear = -1;
}

bool isQueueEmpty(Queue* queue) {
    if (queue == NULL) return true;
    return queue->front == -1;
}

void enqueue(Queue* queue, int value) {
    if (queue == NULL) return;
    if (queue->front == -1) queue->front = 0;
    queue->rear++;
    queue->items[queue->rear] = value;
}

int dequeue(Queue* queue) {
    if (queue == NULL || isQueueEmpty(queue)) return -1;
    int item = queue->items[queue->front];
    queue->front++;
    if (queue->front > queue->rear) {
        queue->front = queue->rear = -1;
    }
    return item;
}

int findNodeIndex(Graph* graph, int value) {
    if (graph == NULL) return -1;
    for (int i = 0; i < graph->nodeCount; i++) {
        if (graph->nodes[i].value == value) {
            return i;
        }
    }
    return -1;
}

void addEdge(Graph* graph, int source, int destination) {
    if (graph == NULL || source < 0 || destination < 0) return;
    
    int sourceIdx = findNodeIndex(graph, source);
    if (sourceIdx == -1) {
        sourceIdx = graph->nodeCount;
        graph->nodes[sourceIdx].value = source;
        graph->nodes[sourceIdx].neighborCount = 0;
        graph->nodeCount++;
    }
    
    int destIdx = findNodeIndex(graph, destination);
    if (destIdx == -1) {
        destIdx = graph->nodeCount;
        graph->nodes[destIdx].value = destination;
        graph->nodes[destIdx].neighborCount = 0;
        graph->nodeCount++;
    }
    
    if (graph->nodes[sourceIdx].neighborCount < MAX_NEIGHBORS) {
        graph->nodes[sourceIdx].neighbors[graph->nodes[sourceIdx].neighborCount++] = destination;
    }
}

int* breadthFirstSearch(Graph* graph, int startNode, int* resultSize) {
    if (graph == NULL || resultSize == NULL || startNode < 0) {
        *resultSize = 0;
        return NULL;
    }
    
    int* result = (int*)malloc(MAX_NODES * sizeof(int));
    if (result == NULL) {
        *resultSize = 0;
        return NULL;
    }
    
    *resultSize = 0;
    
    int startIdx = findNodeIndex(graph, startNode);
    if (startIdx == -1) {
        return result;
    }
    
    bool visited[MAX_NODES] = {false};
    Queue queue;
    initQueue(&queue);
    
    enqueue(&queue, startNode);
    visited[startIdx] = true;
    
    while (!isQueueEmpty(&queue)) {
        int currentNode = dequeue(&queue);
        result[(*resultSize)++] = currentNode;
        
        int currentIdx = findNodeIndex(graph, currentNode);
        if (currentIdx != -1) {
            for (int i = 0; i < graph->nodes[currentIdx].neighborCount; i++) {
                int neighbor = graph->nodes[currentIdx].neighbors[i];
                int neighborIdx = findNodeIndex(graph, neighbor);
                
                if (neighborIdx != -1 && !visited[neighborIdx]) {
                    visited[neighborIdx] = true;
                    enqueue(&queue, neighbor);
                }
            }
        }
    }
    
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
    // Test case 1: Simple linear graph
    printf("Test Case 1: Linear Graph\\n");
    Graph graph1;
    initGraph(&graph1);
    addEdge(&graph1, 1, 2);
    addEdge(&graph1, 2, 3);
    addEdge(&graph1, 3, 4);
    int size1;
    int* result1 = breadthFirstSearch(&graph1, 1, &size1);
    printf("BFS from node 1: ");
    printArray(result1, size1);
    free(result1);
    
    // Test case 2: Graph with multiple branches
    printf("\\nTest Case 2: Graph with Multiple Branches\\n");
    Graph graph2;
    initGraph(&graph2);
    addEdge(&graph2, 0, 1);
    addEdge(&graph2, 0, 2);
    addEdge(&graph2, 1, 3);
    addEdge(&graph2, 1, 4);
    addEdge(&graph2, 2, 5);
    int size2;
    int* result2 = breadthFirstSearch(&graph2, 0, &size2);
    printf("BFS from node 0: ");
    printArray(result2, size2);
    free(result2);
    
    // Test case 3: Disconnected graph
    printf("\\nTest Case 3: Disconnected Graph\\n");
    Graph graph3;
    initGraph(&graph3);
    addEdge(&graph3, 0, 1);
    addEdge(&graph3, 2, 3);
    int size3;
    int* result3 = breadthFirstSearch(&graph3, 0, &size3);
    printf("BFS from node 0: ");
    printArray(result3, size3);
    free(result3);
    
    // Test case 4: Graph with cycle
    printf("\\nTest Case 4: Graph with Cycle\\n");
    Graph graph4;
    initGraph(&graph4);
    addEdge(&graph4, 0, 1);
    addEdge(&graph4, 1, 2);
    addEdge(&graph4, 2, 0);
    addEdge(&graph4, 2, 3);
    int size4;
    int* result4 = breadthFirstSearch(&graph4, 0, &size4);
    printf("BFS from node 0: ");
    printArray(result4, size4);
    free(result4);
    
    // Test case 5: Single node
    printf("\\nTest Case 5: Single Node\\n");
    Graph graph5;
    initGraph(&graph5);
    addEdge(&graph5, 5, 5);
    int size5;
    int* result5 = breadthFirstSearch(&graph5, 5, &size5);
    printf("BFS from node 5: ");
    printArray(result5, size5);
    free(result5);
    
    return 0;
}
