
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NODES 100

typedef struct Node {
    int value;
    int* children;
    int childCount;
    int capacity;
} Node;

typedef struct Tree {
    Node nodes[MAX_NODES];
    int nodeCount;
} Tree;

typedef struct {
    int visited[MAX_NODES];
} VisitedSet;

void initTree(Tree* tree) {
    tree->nodeCount = 0;
    for (int i = 0; i < MAX_NODES; i++) {
        tree->nodes[i].value = -1;
        tree->nodes[i].children = NULL;
        tree->nodes[i].childCount = 0;
        tree->nodes[i].capacity = 0;
    }
}

void addNode(Tree* tree, int value) {
    if (tree->nodeCount < MAX_NODES) {
        tree->nodes[tree->nodeCount].value = value;
        tree->nodes[tree->nodeCount].capacity = 10;
        tree->nodes[tree->nodeCount].children = (int*)malloc(10 * sizeof(int));
        tree->nodes[tree->nodeCount].childCount = 0;
        tree->nodeCount++;
    }
}

void addChild(Node* node, int child) {
    if (node->childCount >= node->capacity) {
        node->capacity *= 2;
        node->children = (int*)realloc(node->children, node->capacity * sizeof(int));
    }
    node->children[node->childCount++] = child;
}

Node* findNode(Tree* tree, int value) {
    for (int i = 0; i < tree->nodeCount; i++) {
        if (tree->nodes[i].value == value) {
            return &tree->nodes[i];
        }
    }
    return NULL;
}

void buildReparentedTree(Tree* inputTree, int current, int parent, VisitedSet* visited, Tree* result) {
    visited->visited[current] = 1;
    
    Node* resultNode = findNode(result, current);
    if (resultNode == NULL) {
        addNode(result, current);
        resultNode = findNode(result, current);
    }
    
    Node* currentNode = findNode(inputTree, current);
    if (currentNode != NULL) {
        for (int i = 0; i < currentNode->childCount; i++) {
            int neighbor = currentNode->children[i];
            if (!visited->visited[neighbor]) {
                addChild(resultNode, neighbor);
                buildReparentedTree(inputTree, neighbor, current, visited, result);
            }
        }
    }
    
    if (parent != -1) {
        addChild(resultNode, parent);
    }
}

Tree reparentTree(Tree* inputTree, int newRoot) {
    Tree result;
    initTree(&result);
    
    VisitedSet visited;
    memset(&visited, 0, sizeof(VisitedSet));
    
    if (findNode(inputTree, newRoot) == NULL) {
        return result;
    }
    
    buildReparentedTree(inputTree, newRoot, -1, &visited, &result);
    
    return result;
}

void printTree(Tree* tree) {
    printf("{ ");
    for (int i = 0; i < tree->nodeCount; i++) {
        printf("%d: [", tree->nodes[i].value);
        for (int j = 0; j < tree->nodes[i].childCount; j++) {
            printf("%d", tree->nodes[i].children[j]);
            if (j < tree->nodes[i].childCount - 1) printf(", ");
        }
        printf("] ");
    }
    printf("}\\n");
}

void freeTree(Tree* tree) {
    for (int i = 0; i < tree->nodeCount; i++) {
        if (tree->nodes[i].children != NULL) {
            free(tree->nodes[i].children);
        }
    }
}

int main() {
    // Test case 1: Simple tree
    Tree tree1;
    initTree(&tree1);
    addNode(&tree1, 0);
    addNode(&tree1, 1);
    addNode(&tree1, 2);
    addNode(&tree1, 3);
    addChild(&tree1.nodes[0], 1);
    addChild(&tree1.nodes[0], 2);
    addChild(&tree1.nodes[0], 3);
    
    printf("Test 1 - Reparent from 0:\\n");
    Tree result1 = reparentTree(&tree1, 0);
    printTree(&result1);
    freeTree(&tree1);
    freeTree(&result1);
    
    return 0;
}
