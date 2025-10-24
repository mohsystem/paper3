
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

// Security: Define maximum limits to prevent resource exhaustion
#define MAX_NODES 100000
#define MAX_EDGES_PER_NODE 10000
#define INITIAL_CAPACITY 16

// Security: All structures use bounded arrays and validation

typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

typedef struct {
    Node** adjacencyList;  // Array of linked lists
    size_t capacity;
    size_t nodeCount;
} Graph;

typedef struct {
    int* data;
    size_t size;
    size_t capacity;
} IntArray;

typedef struct {
    int* data;
    size_t top;
    size_t capacity;
} Stack;

typedef struct {
    bool* visited;
    size_t capacity;
} VisitedSet;

// Security: Initialize all pointers to NULL
Graph* createGraph(void) {
    Graph* graph = (Graph*)calloc(1, sizeof(Graph));
    if (!graph) {
        fprintf(stderr, "Error: Memory allocation failed for graph\\n");
        return NULL;
    }
    
    graph->capacity = INITIAL_CAPACITY;
    graph->nodeCount = 0;
    graph->adjacencyList = (Node**)calloc(graph->capacity, sizeof(Node*));
    
    if (!graph->adjacencyList) {
        fprintf(stderr, "Error: Memory allocation failed for adjacency list\\n");
        free(graph);
        return NULL;
    }
    
    return graph;
}

// Security: Bounds-checked array initialization
IntArray* createIntArray(void) {
    IntArray* arr = (IntArray*)calloc(1, sizeof(IntArray));
    if (!arr) return NULL;
    
    arr->capacity = INITIAL_CAPACITY;
    arr->size = 0;
    arr->data = (int*)calloc(arr->capacity, sizeof(int));
    
    if (!arr->data) {
        free(arr);
        return NULL;
    }
    
    return arr;
}

// Security: Check bounds before adding
bool intArrayAdd(IntArray* arr, int value) {
    if (!arr || !arr->data) return false;
    
    // Security: Limit maximum size
    if (arr->size >= MAX_NODES) {
        fprintf(stderr, "Error: Array size limit exceeded\\n");
        return false;
    }
    
    if (arr->size >= arr->capacity) {
        size_t newCapacity = arr->capacity * 2;
        if (newCapacity > MAX_NODES) newCapacity = MAX_NODES;
        
        int* newData = (int*)realloc(arr->data, newCapacity * sizeof(int));
        if (!newData) {
            fprintf(stderr, "Error: Memory reallocation failed\\n");
            return false;
        }
        
        arr->data = newData;
        arr->capacity = newCapacity;
    }
    
    arr->data[arr->size++] = value;
    return true;
}

Stack* createStack(void) {
    Stack* stack = (Stack*)calloc(1, sizeof(Stack));
    if (!stack) return NULL;
    
    stack->capacity = INITIAL_CAPACITY;
    stack->top = 0;
    stack->data = (int*)calloc(stack->capacity, sizeof(int));
    
    if (!stack->data) {
        free(stack);
        return NULL;
    }
    
    return stack;
}

bool stackPush(Stack* stack, int value) {
    if (!stack || !stack->data) return false;
    
    if (stack->top >= stack->capacity) {
        size_t newCapacity = stack->capacity * 2;
        if (newCapacity > MAX_NODES) newCapacity = MAX_NODES;
        if (stack->top >= newCapacity) return false;
        
        int* newData = (int*)realloc(stack->data, newCapacity * sizeof(int));
        if (!newData) return false;
        
        stack->data = newData;
        stack->capacity = newCapacity;
    }
    
    stack->data[stack->top++] = value;
    return true;
}

bool stackPop(Stack* stack, int* value) {
    if (!stack || !value || stack->top == 0) return false;
    *value = stack->data[--stack->top];
    return true;
}

bool stackIsEmpty(Stack* stack) {
    return !stack || stack->top == 0;
}

VisitedSet* createVisitedSet(size_t maxNode) {
    VisitedSet* set = (VisitedSet*)calloc(1, sizeof(VisitedSet));
    if (!set) return NULL;
    
    // Security: Limit capacity to prevent excessive allocation
    set->capacity = (maxNode < MAX_NODES) ? maxNode + 1 : MAX_NODES;
    set->visited = (bool*)calloc(set->capacity, sizeof(bool));
    
    if (!set->visited) {
        free(set);
        return NULL;
    }
    
    return set;
}

bool isVisited(VisitedSet* set, int node) {
    if (!set || !set->visited || node < 0 || (size_t)node >= set->capacity) {
        return false;
    }
    return set->visited[node];
}

void markVisited(VisitedSet* set, int node) {
    if (set && set->visited && node >= 0 && (size_t)node < set->capacity) {
        set->visited[node] = true;
    }
}

// Security: Validate inputs before adding edge
bool addEdge(Graph* graph, int from, int to) {
    if (!graph || !graph->adjacencyList) return false;
    
    // Security: Validate node IDs
    if (from < 0 || to < 0) {
        fprintf(stderr, "Error: Node IDs must be non-negative\\n");
        return false;
    }
    
    // Security: Check maximum nodes limit
    size_t maxNode = (from > to) ? (size_t)from : (size_t)to;
    if (maxNode >= MAX_NODES) {
        fprintf(stderr, "Error: Node ID exceeds maximum\\n");
        return false;
    }
    
    // Expand graph if needed
    if (maxNode >= graph->capacity) {
        size_t newCapacity = graph->capacity * 2;
        while (newCapacity <= maxNode && newCapacity < MAX_NODES) {
            newCapacity *= 2;
        }
        if (newCapacity > MAX_NODES) newCapacity = MAX_NODES;
        
        Node** newList = (Node**)realloc(graph->adjacencyList, 
                                         newCapacity * sizeof(Node*));
        if (!newList) return false;
        
        // Security: Initialize new memory to NULL
        for (size_t i = graph->capacity; i < newCapacity; i++) {
            newList[i] = NULL;
        }
        
        graph->adjacencyList = newList;
        graph->capacity = newCapacity;
    }
    
    // Security: Limit edges per node
    size_t edgeCount = 0;
    Node* current = graph->adjacencyList[from];
    while (current) {
        edgeCount++;
        if (edgeCount >= MAX_EDGES_PER_NODE) {
            fprintf(stderr, "Error: Maximum edges per node exceeded\\n");
            return false;
        }
        current = current->next;
    }
    
    // Add edge
    Node* newNode = (Node*)calloc(1, sizeof(Node));
    if (!newNode) return false;
    
    newNode->vertex = to;
    newNode->next = graph->adjacencyList[from];
    graph->adjacencyList[from] = newNode;
    
    if ((size_t)from >= graph->nodeCount) graph->nodeCount = from + 1;
    if ((size_t)to >= graph->nodeCount) graph->nodeCount = to + 1;
    
    return true;
}

// Security: Iterative DFS to prevent stack overflow
IntArray* dfs(Graph* graph, int startNode) {
    IntArray* result = createIntArray();
    if (!result || !graph) return result;
    
    // Security: Validate start node
    if (startNode < 0) {
        fprintf(stderr, "Error: Invalid start node\\n");
        return result;
    }
    
    Stack* stack = createStack();
    VisitedSet* visited = createVisitedSet(graph->nodeCount);
    
    if (!stack || !visited) {
        if (stack) {
            free(stack->data);
            free(stack);
        }
        if (visited) {
            free(visited->visited);
            free(visited);
        }
        return result;
    }
    
    stackPush(stack, startNode);
    
    // Security: Limit iterations to prevent infinite loops
    size_t iterations = 0;
    const size_t MAX_ITERATIONS = MAX_NODES * MAX_EDGES_PER_NODE;
    
    while (!stackIsEmpty(stack) && iterations++ < MAX_ITERATIONS) {
        int node;
        if (!stackPop(stack, &node)) break;
        
        if (isVisited(visited, node)) continue;
        
        markVisited(visited, node);
        intArrayAdd(result, node);
        
        // Add unvisited neighbors to stack in reverse order
        if ((size_t)node < graph->capacity && graph->adjacencyList[node]) {
            Node* neighbor = graph->adjacencyList[node];
            while (neighbor) {
                if (!isVisited(visited, neighbor->vertex)) {
                    stackPush(stack, neighbor->vertex);
                }
                neighbor = neighbor->next;
            }
        }
    }
    
    // Security: Clean up - clear sensitive data and free memory
    free(stack->data);
    free(stack);
    free(visited->visited);
    free(visited);
    
    return result;
}

void freeGraph(Graph* graph) {
    if (!graph) return;
    
    if (graph->adjacencyList) {
        for (size_t i = 0; i < graph->capacity; i++) {
            Node* current = graph->adjacencyList[i];
            while (current) {
                Node* temp = current;
                current = current->next;
                free(temp);
            }
        }
        free(graph->adjacencyList);
    }
    
    free(graph);
}

void freeIntArray(IntArray* arr) {
    if (!arr) return;
    if (arr->data) free(arr->data);
    free(arr);
}

int main(void) {
    // Test case 1: Simple linear graph
    {
        Graph* g = createGraph();
        if (g) {
            addEdge(g, 0, 1);
            addEdge(g, 1, 2);
            addEdge(g, 2, 3);
            IntArray* result = dfs(g, 0);
            printf("Test 1 - Linear graph: ");
            if (result) {
                for (size_t i = 0; i < result->size; i++) {
                    printf("%d ", result->data[i]);
                }
                freeIntArray(result);
            }
            printf("\\n");
            freeGraph(g);
        }
    }
    
    // Test case 2: Graph with branches
    {
        Graph* g = createGraph();
        if (g) {
            addEdge(g, 0, 1);
            addEdge(g, 0, 2);
            addEdge(g, 1, 3);
            addEdge(g, 2, 4);
            IntArray* result = dfs(g, 0);
            printf("Test 2 - Branching graph: ");
            if (result) {
                for (size_t i = 0; i < result->size; i++) {
                    printf("%d ", result->data[i]);
                }
                freeIntArray(result);
            }
            printf("\\n");
            freeGraph(g);
        }
    }
    
    // Test case 3: Graph with cycle
    {
        Graph* g = createGraph();
        if (g) {
            addEdge(g, 0, 1);
            addEdge(g, 1, 2);
            addEdge(g, 2, 0);
            addEdge(g, 2, 3);
            IntArray* result = dfs(g, 0);
            printf("Test 3 - Graph with cycle: ");
            if (result) {
                for (size_t i = 0; i < result->size; i++) {
                    printf("%d ", result->data[i]);
                }
                freeIntArray(result);
            }
            printf("\\n");
            freeGraph(g);
        }
    }
    
    // Test case 4: Disconnected node
    {
        Graph* g = createGraph();
        if (g) {
            addEdge(g, 0, 1);
            addEdge(g, 2, 3);
            IntArray* result = dfs(g, 0);
            printf("Test 4 - Disconnected graph: ");
            if (result) {
                for (size_t i = 0; i < result->size; i++) {
                    printf("%d ", result->data[i]);
                }
                freeIntArray(result);
            }
            printf("\\n");
            freeGraph(g);
        }
    }
    
    // Test case 5: Single node
    {
        Graph* g = createGraph();
        if (g) {
            IntArray* result = dfs(g, 0);
            printf("Test 5 - Single node: ");
            if (result) {
                for (size_t i = 0; i < result->size; i++) {
                    printf("%d ", result->data[i]);
                }
                freeIntArray(result);
            }
            printf("\\n");
            freeGraph(g);
        }
    }
    
    return 0;
}
