
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>

/* Maximum limits to prevent excessive memory usage */
#define MAX_NODES 100000
#define MAX_EDGES 1000000
#define INITIAL_CAPACITY 16

/* Node structure for adjacency list */
typedef struct {
    int* neighbors;
    size_t neighborCount;
    size_t capacity;
} AdjNode;

/* Graph structure using adjacency list */
typedef struct {
    AdjNode* nodes;
    int* nodeIds;
    size_t nodeCount;
    size_t nodeCapacity;
    size_t edgeCount;
} Graph;

/* Queue structure for BFS */
typedef struct {
    int* data;
    size_t front;
    size_t rear;
    size_t capacity;
    size_t size;
} Queue;

/* Initialize graph with bounds checking */
Graph* createGraph(void) {
    Graph* graph = (Graph*)calloc(1, sizeof(Graph));
    if (graph == NULL) {
        return NULL;
    }
    
    graph->nodeCapacity = INITIAL_CAPACITY;
    graph->nodes = (AdjNode*)calloc(graph->nodeCapacity, sizeof(AdjNode));
    graph->nodeIds = (int*)calloc(graph->nodeCapacity, sizeof(int));
    
    if (graph->nodes == NULL || graph->nodeIds == NULL) {
        free(graph->nodes);
        free(graph->nodeIds);
        free(graph);
        return NULL;
    }
    
    graph->nodeCount = 0;
    graph->edgeCount = 0;
    
    return graph;
}

/* Find node index, return -1 if not found */
int findNodeIndex(const Graph* graph, int nodeId) {
    if (graph == NULL) {
        return -1;
    }
    
    /* Linear search with bounds checking */
    for (size_t i = 0; i < graph->nodeCount; ++i) {
        if (graph->nodeIds[i] == nodeId) {
            return (int)i;
        }
    }
    return -1;
}

/* Add or get node index with memory safety */
int getOrCreateNodeIndex(Graph* graph, int nodeId) {
    if (graph == NULL || nodeId < 0) {
        return -1;
    }
    
    /* Check if node already exists */
    int index = findNodeIndex(graph, nodeId);
    if (index != -1) {
        return index;
    }
    
    /* Check maximum nodes limit */
    if (graph->nodeCount >= MAX_NODES) {
        return -1;
    }
    
    /* Resize arrays if needed */
    if (graph->nodeCount >= graph->nodeCapacity) {
        size_t newCapacity = graph->nodeCapacity * 2;
        if (newCapacity > MAX_NODES) {
            newCapacity = MAX_NODES;
        }
        
        AdjNode* newNodes = (AdjNode*)realloc(graph->nodes, newCapacity * sizeof(AdjNode));
        int* newIds = (int*)realloc(graph->nodeIds, newCapacity * sizeof(int));
        
        if (newNodes == NULL || newIds == NULL) {
            free(newNodes);
            return -1;
        }
        
        /* Clear new memory */
        memset(newNodes + graph->nodeCapacity, 0, 
               (newCapacity - graph->nodeCapacity) * sizeof(AdjNode));
        
        graph->nodes = newNodes;
        graph->nodeIds = newIds;
        graph->nodeCapacity = newCapacity;
    }
    
    /* Initialize new node */
    index = (int)graph->nodeCount;
    graph->nodeIds[index] = nodeId;
    graph->nodes[index].capacity = INITIAL_CAPACITY;
    graph->nodes[index].neighbors = (int*)calloc(INITIAL_CAPACITY, sizeof(int));
    
    if (graph->nodes[index].neighbors == NULL) {
        return -1;
    }
    
    graph->nodes[index].neighborCount = 0;
    graph->nodeCount++;
    
    return index;
}

/* Add edge with validation and bounds checking */
bool addEdge(Graph* graph, int from, int to) {
    if (graph == NULL || from < 0 || to < 0) {
        return false;
    }
    
    /* Check edge count limit */
    if (graph->edgeCount >= MAX_EDGES) {
        return false;
    }
    
    /* Get or create node indices */
    int fromIndex = getOrCreateNodeIndex(graph, from);
    int toIndex = getOrCreateNodeIndex(graph, to);
    
    if (fromIndex == -1 || toIndex == -1) {
        return false;
    }
    
    AdjNode* node = &graph->nodes[fromIndex];
    
    /* Resize neighbors array if needed */
    if (node->neighborCount >= node->capacity) {
        size_t newCapacity = node->capacity * 2;
        int* newNeighbors = (int*)realloc(node->neighbors, newCapacity * sizeof(int));
        
        if (newNeighbors == NULL) {
            return false;
        }
        
        node->neighbors = newNeighbors;
        node->capacity = newCapacity;
    }
    
    /* Add neighbor with bounds check */
    node->neighbors[node->neighborCount] = to;
    node->neighborCount++;
    graph->edgeCount++;
    
    return true;
}

/* Create queue with size validation */
Queue* createQueue(size_t capacity) {
    if (capacity == 0 || capacity > MAX_NODES) {
        return NULL;
    }
    
    Queue* queue = (Queue*)calloc(1, sizeof(Queue));
    if (queue == NULL) {
        return NULL;
    }
    
    queue->data = (int*)calloc(capacity, sizeof(int));
    if (queue->data == NULL) {
        free(queue);
        return NULL;
    }
    
    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = 0;
    queue->size = 0;
    
    return queue;
}

/* Enqueue with bounds checking */
bool enqueue(Queue* queue, int value) {
    if (queue == NULL || queue->size >= queue->capacity) {
        return false;
    }
    
    queue->data[queue->rear] = value;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->size++;
    
    return true;
}

/* Dequeue with validation */
bool dequeue(Queue* queue, int* value) {
    if (queue == NULL || value == NULL || queue->size == 0) {
        return false;
    }
    
    *value = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    
    return true;
}

/* BFS implementation with memory safety */
int* breadthFirstSearch(Graph* graph, int startNode, size_t* resultSize) {
    /* Initialize output parameters */
    if (resultSize != NULL) {
        *resultSize = 0;
    }
    
    /* Validate inputs */
    if (graph == NULL || resultSize == NULL || startNode < 0) {
        return NULL;
    }
    
    /* Find start node index */
    int startIndex = findNodeIndex(graph, startNode);
    if (startIndex == -1) {
        return NULL;
    }
    
    /* Allocate result array with size validation */
    int* result = (int*)calloc(graph->nodeCount, sizeof(int));
    if (result == NULL) {
        return NULL;
    }
    
    /* Allocate visited array */
    bool* visited = (bool*)calloc(graph->nodeCount, sizeof(bool));
    if (visited == NULL) {
        free(result);
        return NULL;
    }
    
    /* Create queue */
    Queue* queue = createQueue(graph->nodeCount);
    if (queue == NULL) {
        free(result);
        free(visited);
        return NULL;
    }
    
    /* Initialize BFS */
    enqueue(queue, startNode);
    visited[startIndex] = true;
    size_t resultCount = 0;
    
    /* Perform BFS traversal */
    while (queue->size > 0) {
        int currentNode;
        if (!dequeue(queue, &currentNode)) {
            break;
        }
        
        /* Add to result with bounds checking */
        if (resultCount < graph->nodeCount) {
            result[resultCount++] = currentNode;
        }
        
        /* Find current node index */
        int currentIndex = findNodeIndex(graph, currentNode);
        if (currentIndex == -1) {
            continue;
        }
        
        /* Process neighbors with bounds checking */
        AdjNode* node = &graph->nodes[currentIndex];
        for (size_t i = 0; i < node->neighborCount; ++i) {
            int neighbor = node->neighbors[i];
            int neighborIndex = findNodeIndex(graph, neighbor);
            
            if (neighborIndex != -1 && !visited[neighborIndex]) {
                visited[neighborIndex] = true;
                enqueue(queue, neighbor);
            }
        }
    }
    
    *resultSize = resultCount;
    
    /* Clean up */
    free(visited);
    free(queue->data);
    free(queue);
    
    return result;
}

/* Free graph memory */
void freeGraph(Graph* graph) {
    if (graph == NULL) {
        return;
    }
    
    /* Free each node's neighbor list */
    for (size_t i = 0; i < graph->nodeCount; ++i) {
        free(graph->nodes[i].neighbors);
        graph->nodes[i].neighbors = NULL;
    }
    
    free(graph->nodes);
    free(graph->nodeIds);
    free(graph);
}

int main(void) {
    /* Test case 1: Simple linear graph */
    {
        Graph* g = createGraph();
        if (g != NULL) {
            addEdge(g, 1, 2);
            addEdge(g, 2, 3);
            addEdge(g, 3, 4);
            
            size_t resultSize = 0;
            int* result = breadthFirstSearch(g, 1, &resultSize);
            
            printf("Test 1 - Linear graph BFS from node 1: ");
            if (result != NULL) {
                for (size_t i = 0; i < resultSize; ++i) {
                    printf("%d", result[i]);
                    if (i < resultSize - 1) printf(" ");
                }
                free(result);
            }
            printf("\\n");
            
            freeGraph(g);
        }
    }
    
    /* Test case 2: Binary tree structure */
    {
        Graph* g = createGraph();
        if (g != NULL) {
            addEdge(g, 1, 2);
            addEdge(g, 1, 3);
            addEdge(g, 2, 4);
            addEdge(g, 2, 5);
            addEdge(g, 3, 6);
            addEdge(g, 3, 7);
            
            size_t resultSize = 0;
            int* result = breadthFirstSearch(g, 1, &resultSize);
            
            printf("Test 2 - Tree BFS from node 1: ");
            if (result != NULL) {
                for (size_t i = 0; i < resultSize; ++i) {
                    printf("%d", result[i]);
                    if (i < resultSize - 1) printf(" ");
                }
                free(result);
            }
            printf("\\n");
            
            freeGraph(g);
        }
    }
    
    /* Test case 3: Graph with cycle */
    {
        Graph* g = createGraph();
        if (g != NULL) {
            addEdge(g, 0, 1);
            addEdge(g, 1, 2);
            addEdge(g, 2, 0);
            addEdge(g, 2, 3);
            
            size_t resultSize = 0;
            int* result = breadthFirstSearch(g, 0, &resultSize);
            
            printf("Test 3 - Cyclic graph BFS from node 0: ");
            if (result != NULL) {
                for (size_t i = 0; i < resultSize; ++i) {
                    printf("%d", result[i]);
                    if (i < resultSize - 1) printf(" ");
                }
                free(result);
            }
            printf("\\n");
            
            freeGraph(g);
        }
    }
    
    /* Test case 4: Disconnected components */
    {
        Graph* g = createGraph();
        if (g != NULL) {
            addEdge(g, 0, 1);
            addEdge(g, 1, 2);
            addEdge(g, 3, 4);
            addEdge(g, 4, 5);
            
            size_t resultSize = 0;
            int* result = breadthFirstSearch(g, 0, &resultSize);
            
            printf("Test 4 - Disconnected graph BFS from node 0: ");
            if (result != NULL) {
                for (size_t i = 0; i < resultSize; ++i) {
                    printf("%d", result[i]);
                    if (i < resultSize - 1) printf(" ");
                }
                free(result);
            }
            printf("\\n");
            
            freeGraph(g);
        }
    }
    
    /* Test case 5: Invalid start node */
    {
        Graph* g = createGraph();
        if (g != NULL) {
            addEdge(g, 1, 2);
            addEdge(g, 2, 3);
            
            size_t resultSize = 0;
            int* result = breadthFirstSearch(g, 99, &resultSize);
            
            printf("Test 5 - Invalid start node (99): ");
            if (result == NULL || resultSize == 0) {
                printf("Empty result (expected)");
            }
            free(result);
            printf("\\n");
            
            freeGraph(g);
        }
    }
    
    return 0;
}
