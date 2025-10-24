
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

typedef struct Edge {
    int target;
    int weight;
    struct Edge* next;
} Edge;

typedef struct {
    int vertex;
    int distance;
} Node;

typedef struct {
    Node* data;
    int size;
    int capacity;
} PriorityQueue;

PriorityQueue* createPQ(int capacity) {
    PriorityQueue* pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    pq->data = (Node*)malloc(capacity * sizeof(Node));
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

void swap(Node* a, Node* b) {
    Node temp = *a;
    *a = *b;
    *b = temp;
}

void heapifyUp(PriorityQueue* pq, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (pq->data[idx].distance < pq->data[parent].distance) {
            swap(&pq->data[idx], &pq->data[parent]);
            idx = parent;
        } else {
            break;
        }
    }
}

void heapifyDown(PriorityQueue* pq, int idx) {
    while (true) {
        int smallest = idx;
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        
        if (left < pq->size && pq->data[left].distance < pq->data[smallest].distance)
            smallest = left;
        if (right < pq->size && pq->data[right].distance < pq->data[smallest].distance)
            smallest = right;
        
        if (smallest != idx) {
            swap(&pq->data[idx], &pq->data[smallest]);
            idx = smallest;
        } else {
            break;
        }
    }
}

void push(PriorityQueue* pq, int vertex, int distance) {
    if (pq->size >= pq->capacity) return;
    pq->data[pq->size].vertex = vertex;
    pq->data[pq->size].distance = distance;
    heapifyUp(pq, pq->size);
    pq->size++;
}

Node pop(PriorityQueue* pq) {
    Node result = pq->data[0];
    pq->data[0] = pq->data[pq->size - 1];
    pq->size--;
    heapifyDown(pq, 0);
    return result;
}

bool isEmpty(PriorityQueue* pq) {
    return pq->size == 0;
}

int* dijkstra(Edge** graph, int start, int n) {
    int* distances = (int*)malloc(n * sizeof(int));
    bool* visited = (bool*)malloc(n * sizeof(bool));
    
    for (int i = 0; i < n; i++) {
        distances[i] = INT_MAX;
        visited[i] = false;
    }
    distances[start] = 0;
    
    PriorityQueue* pq = createPQ(n * 10);
    push(pq, start, 0);
    
    while (!isEmpty(pq)) {
        Node current = pop(pq);
        int u = current.vertex;
        
        if (visited[u]) continue;
        visited[u] = true;
        
        Edge* edge = graph[u];
        while (edge != NULL) {
            int v = edge->target;
            int weight = edge->weight;
            
            if (!visited[v] && distances[u] != INT_MAX && distances[u] + weight < distances[v]) {
                distances[v] = distances[u] + weight;
                push(pq, v, distances[v]);
            }
            edge = edge->next;
        }
    }
    
    free(visited);
    free(pq->data);
    free(pq);
    return distances;
}

void addEdge(Edge** graph, int src, int dest, int weight) {
    Edge* newEdge = (Edge*)malloc(sizeof(Edge));
    newEdge->target = dest;
    newEdge->weight = weight;
    newEdge->next = graph[src];
    graph[src] = newEdge;
}

void printResult(int* result, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        if (result[i] == INT_MAX)
            printf("INF");
        else
            printf("%d", result[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]\\n");
}

void freeGraph(Edge** graph, int n) {
    for (int i = 0; i < n; i++) {
        Edge* edge = graph[i];
        while (edge != NULL) {
            Edge* temp = edge;
            edge = edge->next;
            free(temp);
        }
    }
}

int main() {
    // Test Case 1: Simple graph
    printf("Test Case 1:\\n");
    int n1 = 5;
    Edge** graph1 = (Edge**)calloc(n1, sizeof(Edge*));
    addEdge(graph1, 0, 1, 4);
    addEdge(graph1, 0, 2, 1);
    addEdge(graph1, 1, 3, 1);
    addEdge(graph1, 2, 1, 2);
    addEdge(graph1, 2, 3, 5);
    addEdge(graph1, 3, 4, 3);
    int* result1 = dijkstra(graph1, 0, n1);
    printResult(result1, n1);
    free(result1);
    freeGraph(graph1, n1);
    free(graph1);
    
    // Test Case 2: Single node
    printf("\\nTest Case 2:\\n");
    int n2 = 1;
    Edge** graph2 = (Edge**)calloc(n2, sizeof(Edge*));
    int* result2 = dijkstra(graph2, 0, n2);
    printResult(result2, n2);
    free(result2);
    free(graph2);
    
    // Test Case 3: Disconnected graph
    printf("\\nTest Case 3:\\n");
    int n3 = 4;
    Edge** graph3 = (Edge**)calloc(n3, sizeof(Edge*));
    addEdge(graph3, 0, 1, 1);
    addEdge(graph3, 1, 0, 1);
    int* result3 = dijkstra(graph3, 0, n3);
    printResult(result3, n3);
    free(result3);
    freeGraph(graph3, n3);
    free(graph3);
    
    // Test Case 4: Dense graph
    printf("\\nTest Case 4:\\n");
    int n4 = 4;
    Edge** graph4 = (Edge**)calloc(n4, sizeof(Edge*));
    addEdge(graph4, 0, 1, 5);
    addEdge(graph4, 0, 2, 10);
    addEdge(graph4, 1, 2, 3);
    addEdge(graph4, 1, 3, 8);
    addEdge(graph4, 2, 3, 2);
    int* result4 = dijkstra(graph4, 0, n4);
    printResult(result4, n4);
    free(result4);
    freeGraph(graph4, n4);
    free(graph4);
    
    // Test Case 5: Linear graph
    printf("\\nTest Case 5:\\n");
    int n5 = 6;
    Edge** graph5 = (Edge**)calloc(n5, sizeof(Edge*));
    addEdge(graph5, 0, 1, 2);
    addEdge(graph5, 1, 2, 3);
    addEdge(graph5, 2, 3, 1);
    addEdge(graph5, 3, 4, 4);
    addEdge(graph5, 4, 5, 2);
    int* result5 = dijkstra(graph5, 0, n5);
    printResult(result5, n5);
    free(result5);
    freeGraph(graph5, n5);
    free(graph5);
    
    return 0;
}
