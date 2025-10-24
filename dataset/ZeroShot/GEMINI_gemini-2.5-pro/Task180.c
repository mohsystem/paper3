#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// --- Data Structures ---
typedef struct Node {
    int id;
    struct Node** children;
    int children_count;
    int children_capacity;
} Node;

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
    bool* nodes_exist;
} Graph;

// --- Tree Helper Functions ---
Node* createNode(int id) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) return NULL;
    newNode->id = id;
    newNode->children_count = 0;
    newNode->children_capacity = 2;
    newNode->children = (Node**)malloc(newNode->children_capacity * sizeof(Node*));
    if (!newNode->children) {
        free(newNode);
        return NULL;
    }
    return newNode;
}

void addChild(Node* parent, Node* child) {
    if (!parent || !child) return;
    if (parent->children_count >= parent->children_capacity) {
        parent->children_capacity *= 2;
        Node** new_children = (Node**)realloc(parent->children, parent->children_capacity * sizeof(Node*));
        if (!new_children) return;
        parent->children = new_children;
    }
    parent->children[parent->children_count++] = child;
}

void freeTree(Node* node) {
    if (node == NULL) return;
    for (int i = 0; i < node->children_count; i++) {
        freeTree(node->children[i]);
    }
    free(node->children);
    free(node);
}

int compareNodes(const void* a, const void* b) {
    Node* nodeA = *(Node**)a;
    Node* nodeB = *(Node**)b;
    return (nodeA->id < nodeB->id) ? -1 : (nodeA->id > nodeB->id);
}

// --- Graph Helper Functions ---
Graph* createGraph(int V) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (!graph) return NULL;
    graph->V = V;
    graph->array = (AdjList*)calloc(V, sizeof(AdjList));
    graph->nodes_exist = (bool*)calloc(V, sizeof(bool));
    if (!graph->array || !graph->nodes_exist) {
        free(graph->array); free(graph->nodes_exist); free(graph);
        return NULL;
    }
    return graph;
}

void freeGraph(Graph* graph) {
    if (graph == NULL) return;
    for (int i = 0; i < graph->V; i++) {
        AdjListNode* pCrawl = graph->array[i].head;
        while (pCrawl) {
            AdjListNode* temp = pCrawl;
            pCrawl = pCrawl->next;
            free(temp);
        }
    }
    free(graph->array);
    free(graph->nodes_exist);
    free(graph);
}

void addEdge(Graph* graph, int src, int dest) {
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (!newNode) return;
    newNode->dest = dest;
    newNode->next = graph->array[src].head;
    graph->array[src].head = newNode;
    graph->nodes_exist[src] = true;
}

// --- Core Logic ---
Node* buildTreeRecursive(int currentNodeId, int parentId, Graph* graph) {
    Node* node = createNode(currentNodeId);
    if (!node) return NULL;
    
    AdjListNode* pCrawl = graph->array[currentNodeId].head;
    while (pCrawl != NULL) {
        int neighborId = pCrawl->dest;
        if (neighborId != parentId) {
            Node* childNode = buildTreeRecursive(neighborId, currentNodeId, graph);
            if (childNode) {
                addChild(node, childNode);
            }
        }
        pCrawl = pCrawl->next;
    }

    if (node->children_count > 1) {
        qsort(node->children, node->children_count, sizeof(Node*), compareNodes);
    }
    return node;
}

Node* reparentTree(const int edges[][2], int num_edges, int newRoot) {
    if (num_edges == 0) {
        return createNode(newRoot);
    }
    
    int max_node_id = newRoot;
    for (int i = 0; i < num_edges; i++) {
        if (edges[i][0] > max_node_id) max_node_id = edges[i][0];
        if (edges[i][1] > max_node_id) max_node_id = edges[i][1];
    }

    Graph* graph = createGraph(max_node_id + 1);
    if (!graph) return NULL;

    for (int i = 0; i < num_edges; i++) {
        addEdge(graph, edges[i][0], edges[i][1]);
        addEdge(graph, edges[i][1], edges[i][0]);
    }
    
    if (newRoot <= max_node_id) {
        graph->nodes_exist[newRoot] = true;
    }

    if (newRoot > max_node_id || !graph->nodes_exist[newRoot]) {
        freeGraph(graph);
        return NULL;
    }
    
    Node* newTreeRoot = buildTreeRecursive(newRoot, -1, graph);
    freeGraph(graph);
    return newTreeRoot;
}

// --- Main and Test Cases ---
void printTree(const Node* node, const char* prefix) {
    if (node == NULL) {
        printf("Tree is null.\n");
        return;
    }
    printf("%s%d\n", prefix, node->id);
    
    size_t prefix_len = strlen(prefix);
    char* new_prefix = (char*)malloc(prefix_len + 3);
    if (!new_prefix) return;
    strcpy(new_prefix, prefix);
    strcat(new_prefix, "  ");

    for (int i = 0; i < node->children_count; i++) {
        printTree(node->children[i], new_prefix);
    }
    free(new_prefix);
}

void runTestCase(int testNum, const int edges[][2], int num_edges, int newRoot) {
    printf("--- Test Case %d ---\n", testNum);
    printf("Reparenting on node %d\n", newRoot);
    Node* newTree = reparentTree(edges, num_edges, newRoot);
    printTree(newTree, "");
    freeTree(newTree);
    printf("\n");
}

int main() {
    const int edges1[][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}};
    runTestCase(1, edges1, sizeof(edges1)/sizeof(edges1[0]), 6);

    runTestCase(2, edges1, sizeof(edges1)/sizeof(edges1[0]), 0);

    runTestCase(3, edges1, sizeof(edges1)/sizeof(edges1[0]), 9);
    
    const int edges4[][2] = {{1, 0}, {2, 0}, {3, 1}, {4, 1}};
    runTestCase(4, edges4, sizeof(edges4)/sizeof(edges4[0]), 1);

    const int edges5[][2] = {{0, 1}, {1, 2}, {2, 3}};
    runTestCase(5, edges5, sizeof(edges5)/sizeof(edges5[0]), 2);

    return 0;
}