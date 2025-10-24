#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NODE_ID 50 // Assuming node IDs are within 0-49

// For adjacency list of the input (undirected) graph
typedef struct AdjNode {
    int dest;
    struct AdjNode* next;
} AdjNode;

// For storing children in the output (directed) tree
typedef struct {
    int* children;
    int count;
} ChildrenList;


// --- Helper Functions ---
void addEdge(AdjNode** adj, int src, int dest) {
    AdjNode* newNode1 = (AdjNode*)malloc(sizeof(AdjNode));
    if (!newNode1) return;
    newNode1->dest = dest;
    newNode1->next = adj[src];
    adj[src] = newNode1;

    AdjNode* newNode2 = (AdjNode*)malloc(sizeof(AdjNode));
    if (!newNode2) { free(newNode1); return; }
    newNode2->dest = src;
    newNode2->next = adj[dest];
    adj[dest] = newNode2;
}

void freeAdjList(AdjNode** adj) {
    if (!adj) return;
    for (int i = 0; i < MAX_NODE_ID; ++i) {
        AdjNode* current = adj[i];
        while (current != NULL) {
            AdjNode* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(adj);
}

void freeTree(ChildrenList* tree) {
    if (!tree) return;
    for (int i = 0; i < MAX_NODE_ID; ++i) {
        free(tree[i].children);
    }
    free(tree);
}

int compare_ints(const void* a, const void* b) {
    return (*(const int*)a - *(const int*)b);
}

// --- Core Logic ---
void buildNewTreeDfs(int u, int p, AdjNode** old_adj, ChildrenList* new_tree) {
    int neighbors[MAX_NODE_ID];
    int neighbor_count = 0;
    AdjNode* crawl = old_adj[u];
    while (crawl != NULL) {
        if (neighbor_count < MAX_NODE_ID) {
            neighbors[neighbor_count++] = crawl->dest;
        }
        crawl = crawl->next;
    }

    qsort(neighbors, neighbor_count, sizeof(int), compare_ints);

    int children_count = 0;
    for (int i = 0; i < neighbor_count; ++i) {
        if (neighbors[i] != p) {
            children_count++;
        }
    }

    if (children_count > 0) {
        new_tree[u].children = (int*)malloc(children_count * sizeof(int));
        if (new_tree[u].children == NULL) return; // Allocation failed
    }
    new_tree[u].count = children_count;

    int current_child_idx = 0;
    for (int i = 0; i < neighbor_count; ++i) {
        int v = neighbors[i];
        if (v != p) {
            new_tree[u].children[current_child_idx++] = v;
            buildNewTreeDfs(v, u, old_adj, new_tree);
        }
    }
}

ChildrenList* reparentTree(AdjNode** old_adj, int new_root) {
    ChildrenList* new_tree = (ChildrenList*)calloc(MAX_NODE_ID, sizeof(ChildrenList));
    if (!new_tree) return NULL;
    
    buildNewTreeDfs(new_root, -1, old_adj, new_tree);
    return new_tree;
}


// --- Test Main ---
void printTreeFromRoot(int u, ChildrenList* tree) {
    printf("%d -> [", u);
    for (int j = 0; j < tree[u].count; ++j) {
        printf("%d%s", tree[u].children[j], (j == tree[u].count - 1) ? "" : ", ");
    }
    printf("]\n");

    for (int j = 0; j < tree[u].count; ++j) {
        printTreeFromRoot(tree[u].children[j], tree);
    }
}

void runTestCase(int testNum, const char* description, const int edges[][2], int num_edges, int new_root) {
    printf("--- Test Case %d: %s ---\n", testNum, description);

    AdjNode** adj = (AdjNode**)calloc(MAX_NODE_ID, sizeof(AdjNode*));
    if (!adj) {
        printf("Failed to allocate adjacency list.\n");
        return;
    }
    for(int i = 0; i < num_edges; ++i) {
        addEdge(adj, edges[i][0], edges[i][1]);
    }
    
    printf("Reparenting on node: %d\n", new_root);
    ChildrenList* reparentedTree = reparentTree(adj, new_root);

    if (reparentedTree) {
        printf("Reparented Tree (Parent -> Children):\n");
        printTreeFromRoot(new_root, reparentedTree);
    } else {
        printf("Failed to reparent tree.\n");
    }
    printf("\n");

    freeAdjList(adj);
    freeTree(reparentedTree);
}

int main() {
    int edges1[][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}};
    runTestCase(1, "Example from prompt", edges1, sizeof(edges1)/sizeof(edges1[0]), 6);
    
    int edges2[][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
    runTestCase(2, "Simple line graph", edges2, sizeof(edges2)/sizeof(edges2[0]), 2);

    int edges3[][2] = {{0, 1}, {0, 2}, {0, 3}, {0, 4}};
    runTestCase(3, "Star graph, reparent on root", edges3, sizeof(edges3)/sizeof(edges3[0]), 0);
    
    int edges4[][2] = {{0, 1}, {0, 2}, {0, 3}, {0, 4}};
    runTestCase(4, "Star graph, reparent on a leaf", edges4, sizeof(edges4)/sizeof(edges4[0]), 3);

    int edges5[][2] = {{1,2},{1,3},{1,4},{2,5},{2,6},{4,7},{4,8}};
    runTestCase(5, "A more complex tree", edges5, sizeof(edges5)/sizeof(edges5[0]), 4);
    
    return 0;
}