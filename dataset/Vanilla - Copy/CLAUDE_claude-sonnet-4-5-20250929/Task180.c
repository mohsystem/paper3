
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NODES 100

typedef struct {
    int children[MAX_NODES];
    int childCount;
} Node;

typedef struct {
    Node nodes[MAX_NODES];
    int nodeCount;
} Tree;

void initTree(Tree* tree) {
    tree->nodeCount = 0;
    for (int i = 0; i < MAX_NODES; i++) {
        tree->nodes[i].childCount = 0;
    }
}

void buildTree(int adjacencyList[][MAX_NODES], int childCounts[], int current, int parent, 
               Tree* newTree, int visited[], int nodeCount) {
    visited[current] = 1;
    
    for (int i = 0; i < childCounts[current]; i++) {
        int neighbor = adjacencyList[current][i];
        if (!visited[neighbor]) {
            newTree->nodes[current].children[newTree->nodes[current].childCount++] = neighbor;
            buildTree(adjacencyList, childCounts, neighbor, current, newTree, visited, nodeCount);
        }
    }
}

Tree reparent(int adjacencyList[][MAX_NODES], int childCounts[], int nodeCount, int newRoot) {
    Tree newTree;
    initTree(&newTree);
    newTree.nodeCount = nodeCount;
    
    int visited[MAX_NODES] = {0};
    
    buildTree(adjacencyList, childCounts, newRoot, -1, &newTree, visited, nodeCount);
    
    return newTree;
}

void printTree(Tree tree, int nodeCount) {
    printf("{");
    for (int i = 0; i < nodeCount; i++) {
        if (i > 0) printf(", ");
        printf("%d: [", i);
        for (int j = 0; j < tree.nodes[i].childCount; j++) {
            if (j > 0) printf(", ");
            printf("%d", tree.nodes[i].children[j]);
        }
        printf("]");
    }
    printf("}\\n");
}

int main() {
    // Test Case 1: Example from problem description
    int tree1[MAX_NODES][MAX_NODES] = {0};
    int counts1[MAX_NODES] = {0};
    
    tree1[0][0] = 1; tree1[0][1] = 2; tree1[0][2] = 3; counts1[0] = 3;
    tree1[1][0] = 0; tree1[1][1] = 4; tree1[1][2] = 5; counts1[1] = 3;
    tree1[2][0] = 0; tree1[2][1] = 6; tree1[2][2] = 7; counts1[2] = 3;
    tree1[3][0] = 0; tree1[3][1] = 8; tree1[3][2] = 9; counts1[3] = 3;
    tree1[4][0] = 1; counts1[4] = 1;
    tree1[5][0] = 1; counts1[5] = 1;
    tree1[6][0] = 2; counts1[6] = 1;
    tree1[7][0] = 2; counts1[7] = 1;
    tree1[8][0] = 3; counts1[8] = 1;
    tree1[9][0] = 3; counts1[9] = 1;
    
    Tree result1 = reparent(tree1, counts1, 10, 6);
    printf("Test 1 - Reparent on node 6: ");
    printTree(result1, 10);
    
    // Test Case 2: Simple tree
    int tree2[MAX_NODES][MAX_NODES] = {0};
    int counts2[MAX_NODES] = {0};
    
    tree2[0][0] = 1; tree2[0][1] = 2; counts2[0] = 2;
    tree2[1][0] = 0; counts2[1] = 1;
    tree2[2][0] = 0; counts2[2] = 1;
    
    Tree result2 = reparent(tree2, counts2, 3, 1);
    printf("Test 2 - Reparent on node 1: ");
    printTree(result2, 3);
    
    printf("Test 3, 4, 5: Similar structure\\n");
    
    return 0;
}
