#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_NODES 100 // Assuming max node value is less than 100

typedef struct Node {
    int value;
    struct Node** children;
    int children_count;
} Node;

Node* createNode(int value) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->value = value;
    newNode->children = NULL;
    newNode->children_count = 0;
    return newNode;
}

void addChild(Node* parent, Node* child) {
    parent->children_count++;
    parent->children = (Node**)realloc(parent->children, parent->children_count * sizeof(Node*));
    parent->children[parent->children_count - 1] = child;
}

void deleteTree(Node* node) {
    if (!node) return;
    for (int i = 0; i < node->children_count; i++) {
        deleteTree(node->children[i]);
    }
    free(node->children);
    free(node);
}

typedef struct AdjListNode {
    int dest;
    struct AdjListNode* next;
} AdjListNode;

typedef struct AdjList {
    AdjListNode* head;
} AdjList;

void addEdge(AdjList* adj, int src, int dest) {
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    newNode->dest = dest;
    newNode->next = adj[src].head;
    adj[src].head = newNode;

    newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    newNode->dest = src;
    newNode->next = adj[dest].head;
    adj[dest].head = newNode;
}

void freeAdjList(AdjList* adj) {
    for(int i = 0; i < MAX_NODES; i++) {
        AdjListNode* current = adj[i].head;
        while(current) {
            AdjListNode* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(adj);
}

void buildAdjacencyListRecursive(Node* node, AdjList* adj, bool* nodes) {
    if (!node) return;
    nodes[node->value] = true;
    for (int i = 0; i < node->children_count; i++) {
        Node* child = node->children[i];
        addEdge(adj, node->value, child->value);
        buildAdjacencyListRecursive(child, adj, nodes);
    }
}

typedef struct QueueNode {
    Node* treeNode;
    struct QueueNode* next;
} QueueNode;

typedef struct Queue {
    QueueNode *front, *rear;
} Queue;

void enqueue(Queue* q, Node* treeNode) {
    QueueNode* temp = (QueueNode*)malloc(sizeof(QueueNode));
    temp->treeNode = treeNode;
    temp->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}

Node* dequeue(Queue* q) {
    if (q->front == NULL) return NULL;
    QueueNode* temp = q->front;
    Node* treeNode = temp->treeNode;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    free(temp);
    return treeNode;
}


Node* buildNewTreeFromAdjacencyList(int rootValue, AdjList* adj) {
    Node* newRoot = createNode(rootValue);
    Queue q = {NULL, NULL};
    enqueue(&q, newRoot);
    
    bool visited[MAX_NODES] = {false};
    visited[rootValue] = true;

    while (q.front != NULL) {
        Node* currentNode = dequeue(&q);
        AdjListNode* neighbor = adj[currentNode->value].head;
        while (neighbor != NULL) {
            if (!visited[neighbor->dest]) {
                visited[neighbor->dest] = true;
                Node* childNode = createNode(neighbor->dest);
                addChild(currentNode, childNode);
                enqueue(&q, childNode);
            }
            neighbor = neighbor->next;
        }
    }
    return newRoot;
}

Node* reparentTree(Node* oldRoot, int newRootValue) {
    if (!oldRoot) return NULL;

    AdjList* adj = (AdjList*)calloc(MAX_NODES, sizeof(AdjList));
    bool nodes[MAX_NODES] = {false};
    buildAdjacencyListRecursive(oldRoot, adj, nodes);

    if (!nodes[newRootValue]) {
        freeAdjList(adj);
        return NULL;
    }

    Node* newRoot = buildNewTreeFromAdjacencyList(newRootValue, adj);
    
    freeAdjList(adj);
    
    return newRoot;
}

void printTree(Node* node, const char* prefix, bool isTail) {
    if (!node) return;
    printf("%s%s%d\n", prefix, isTail ? "└── " : "├── ", node->value);
    
    char newPrefix[256];
    snprintf(newPrefix, sizeof(newPrefix), "%s%s", prefix, isTail ? "    " : "│   ");

    for (int i = 0; i < node->children_count; i++) {
        printTree(node->children[i], newPrefix, i == node->children_count - 1);
    }
}

void runTestCase(const char* name, Node* root, int newRootValue) {
    printf("--- %s ---\n", name);
    printf("Original tree (rooted at %d):\n", root->value);
    printTree(root, "", true);

    Node* newRoot = reparentTree(root, newRootValue);
    if (newRoot) {
        printf("\nNew tree (re-rooted at %d):\n", newRootValue);
        printTree(newRoot, "", true);
        deleteTree(newRoot);
    } else {
        printf("\nCould not re-root at %d (node not in tree).\n", newRootValue);
    }
    printf("\n%s\n", "========================================");
}

int main() {
    // Test Case 1: Example from prompt
    Node* root1 = createNode(0);
    Node* n1 = createNode(1);
    Node* n2 = createNode(2);
    Node* n3 = createNode(3);
    addChild(n1, createNode(4)); addChild(n1, createNode(5));
    addChild(n2, createNode(6)); addChild(n2, createNode(7));
    addChild(n3, createNode(8)); addChild(n3, createNode(9));
    addChild(root1, n1); addChild(root1, n2); addChild(root1, n3);
    runTestCase("Test Case 1: Re-parenting on node 6", root1, 6);

    // Test Case 2: Re-rooting at the current root (0)
    runTestCase("Test Case 2: Re-parenting on current root 0", root1, 0);

    // Test Case 3: Re-rooting at a leaf node (9)
    runTestCase("Test Case 3: Re-parenting on leaf node 9", root1, 9);
    
    deleteTree(root1);

    // Test Case 4: Linear tree
    Node* root4 = createNode(0);
    Node* r4n1 = createNode(1);
    Node* r4n2 = createNode(2);
    Node* r4n3 = createNode(3);
    addChild(r4n2, r4n3);
    addChild(r4n1, r4n2);
    addChild(root4, r4n1);
    runTestCase("Test Case 4: Linear tree", root4, 2);
    deleteTree(root4);

    // Test Case 5: Tree with a single node
    Node* root5 = createNode(42);
    runTestCase("Test Case 5: Single node tree", root5, 42);
    deleteTree(root5);

    return 0;
}