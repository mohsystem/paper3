
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHILDREN 10
#define MAX_NODES 100

typedef struct TreeNode {
    int value;
    struct TreeNode* children[MAX_CHILDREN];
    int childCount;
    struct TreeNode* parent;
} TreeNode;

TreeNode* createNode(int value) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if (node == NULL) return NULL;
    
    node->value = value;
    node->childCount = 0;
    node->parent = NULL;
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = NULL;
    }
    return node;
}

TreeNode* findNode(TreeNode* root, int value) {
    if (root == NULL) return NULL;
    if (root->value == value) return root;
    
    for (int i = 0; i < root->childCount; i++) {
        TreeNode* found = findNode(root->children[i], value);
        if (found != NULL) return found;
    }
    return NULL;
}

void removeChild(TreeNode* parent, TreeNode* child) {
    if (parent == NULL || child == NULL) return;
    
    for (int i = 0; i < parent->childCount; i++) {
        if (parent->children[i] == child) {
            for (int j = i; j < parent->childCount - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->children[parent->childCount - 1] = NULL;
            parent->childCount--;
            break;
        }
    }
}

void addChild(TreeNode* parent, TreeNode* child) {
    if (parent == NULL || child == NULL) return;
    if (parent->childCount >= MAX_CHILDREN) return;
    
    parent->children[parent->childCount] = child;
    parent->childCount++;
}

TreeNode* reparentTree(TreeNode* root, int targetValue) {
    if (root == NULL) return NULL;
    
    TreeNode* targetNode = findNode(root, targetValue);
    if (targetNode == NULL) return root;
    if (targetNode == root) return root;
    
    TreeNode* pathToRoot[MAX_NODES];
    int pathLength = 0;
    TreeNode* current = targetNode;
    
    while (current != NULL && pathLength < MAX_NODES) {
        pathToRoot[pathLength++] = current;
        current = current->parent;
    }
    
    for (int i = 0; i < pathLength - 1; i++) {
        TreeNode* child = pathToRoot[i];
        TreeNode* parent = pathToRoot[i + 1];
        
        removeChild(parent, child);
        addChild(child, parent);
        parent->parent = child;
    }
    
    targetNode->parent = NULL;
    return targetNode;
}

TreeNode* buildTree(int edges[][2], int edgeCount) {
    if (edgeCount == 0) return NULL;
    
    TreeNode* nodes[MAX_NODES] = {NULL};
    int childrenSet[MAX_NODES] = {0};
    int maxNode = -1;
    
    for (int i = 0; i < edgeCount; i++) {
        int parentVal = edges[i][0];
        int childVal = edges[i][1];
        
        if (parentVal > maxNode) maxNode = parentVal;
        if (childVal > maxNode) maxNode = childVal;
        
        if (parentVal >= MAX_NODES || childVal >= MAX_NODES) continue;
        
        if (nodes[parentVal] == NULL) {
            nodes[parentVal] = createNode(parentVal);
        }
        if (nodes[childVal] == NULL) {
            nodes[childVal] = createNode(childVal);
        }
        
        addChild(nodes[parentVal], nodes[childVal]);
        nodes[childVal]->parent = nodes[parentVal];
        childrenSet[childVal] = 1;
    }
    
    for (int i = 0; i <= maxNode && i < MAX_NODES; i++) {
        if (nodes[i] != NULL && childrenSet[i] == 0) {
            return nodes[i];
        }
    }
    
    return NULL;
}

void printTree(TreeNode* root, char* prefix, int isTail) {
    if (root == NULL) return;
    
    printf("%s%s%d\\n", prefix, isTail ? "└── " : "├── ", root->value);
    
    char newPrefix[256];
    snprintf(newPrefix, sizeof(newPrefix), "%s%s", prefix, isTail ? "    " : "│   ");
    
    for (int i = 0; i < root->childCount; i++) {
        printTree(root->children[i], newPrefix, i == root->childCount - 1);
    }
}

void freeTree(TreeNode* root) {
    if (root == NULL) return;
    
    for (int i = 0; i < root->childCount; i++) {
        freeTree(root->children[i]);
    }
    free(root);
}

int main() {
    // Test case 1
    int edges1[][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}};
    TreeNode* root1 = buildTree(edges1, 9);
    printf("Test 1 - Original tree:\\n");
    printTree(root1, "", 1);
    TreeNode* newRoot1 = reparentTree(root1, 6);
    printf("\\nTest 1 - Reparented on node 6:\\n");
    printTree(newRoot1, "", 1);
    freeTree(newRoot1);
    
    // Test case 2
    int edges2[][2] = {{0, 1}, {0, 2}};
    TreeNode* root2 = buildTree(edges2, 2);
    printf("\\n\\nTest 2 - Reparent on root:\\n");
    TreeNode* newRoot2 = reparentTree(root2, 0);
    printTree(newRoot2, "", 1);
    freeTree(newRoot2);
    
    // Test case 3
    TreeNode* root3 = createNode(0);
    printf("\\n\\nTest 3 - Single node:\\n");
    TreeNode* newRoot3 = reparentTree(root3, 0);
    printTree(newRoot3, "", 1);
    freeTree(newRoot3);
    
    // Test case 4
    int edges4[][2] = {{0, 1}, {1, 2}, {2, 3}};
    TreeNode* root4 = buildTree(edges4, 3);
    printf("\\n\\nTest 4 - Linear tree reparented on leaf:\\n");
    TreeNode* newRoot4 = reparentTree(root4, 3);
    printTree(newRoot4, "", 1);
    freeTree(newRoot4);
    
    // Test case 5
    int edges5[][2] = {{0, 1}, {0, 2}};
    TreeNode* root5 = buildTree(edges5, 2);
    printf("\\n\\nTest 5 - Invalid target (returns original):\\n");
    TreeNode* newRoot5 = reparentTree(root5, 99);
    printTree(newRoot5, "", 1);
    freeTree(newRoot5);
    
    return 0;
}
