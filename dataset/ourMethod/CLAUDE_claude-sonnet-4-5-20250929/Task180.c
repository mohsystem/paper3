
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Maximum nodes supported - prevents excessive memory allocation */
#define MAX_NODES 1000
#define MAX_CHILDREN 100

/* Tree node structure */
typedef struct TreeNode {
    int value;
    int* children;           /* Dynamic array of child values */
    size_t child_count;      /* Number of children */
    size_t child_capacity;   /* Allocated capacity */
} TreeNode;

/* Adjacency list for undirected graph */
typedef struct {
    int* neighbors;
    size_t count;
    size_t capacity;
} AdjList;

/* Graph structure */
typedef struct {
    AdjList lists[MAX_NODES];
    bool node_exists[MAX_NODES];
    int node_count;
} Graph;

/* Initialize graph - all memory zeroed for safety */
void initGraph(Graph* g) {
    if (g == NULL) return;
    
    /* Security: zero initialize all memory */
    memset(g, 0, sizeof(Graph));
}

/* Add edge to graph with bounds checking */
bool addEdge(Graph* g, int from, int to) {
    /* Input validation: bounds check */
    if (g == NULL || from < 0 || from >= MAX_NODES || 
        to < 0 || to >= MAX_NODES) {
        return false;
    }
    
    /* Security: prevent self-loops */
    if (from == to) {
        return false;
    }
    
    /* Initialize adjacency list if needed */
    if (g->lists[from].neighbors == NULL) {
        /* Security: check malloc return */
        g->lists[from].neighbors = (int*)calloc(10, sizeof(int));
        if (g->lists[from].neighbors == NULL) {
            return false;
        }
        g->lists[from].capacity = 10;
        g->lists[from].count = 0;
    }
    
    /* Grow array if needed with bounds check */
    if (g->lists[from].count >= g->lists[from].capacity) {
        /* Security: prevent excessive allocation */
        if (g->lists[from].capacity >= MAX_CHILDREN) {
            return false;
        }
        
        size_t new_cap = g->lists[from].capacity * 2;
        if (new_cap > MAX_CHILDREN) {
            new_cap = MAX_CHILDREN;
        }
        
        /* Security: check integer overflow before allocation */
        if (new_cap > SIZE_MAX / sizeof(int)) {
            return false;
        }
        
        int* new_arr = (int*)realloc(g->lists[from].neighbors, 
                                     new_cap * sizeof(int));
        if (new_arr == NULL) {
            return false;
        }
        g->lists[from].neighbors = new_arr;
        g->lists[from].capacity = new_cap;
    }
    
    /* Add neighbor */
    g->lists[from].neighbors[g->lists[from].count++] = to;
    g->node_exists[from] = true;
    g->node_exists[to] = true;
    
    return true;
}

/* Free graph memory */
void freeGraph(Graph* g) {
    if (g == NULL) return;
    
    /* Free all adjacency lists */
    for (int i = 0; i < MAX_NODES; i++) {
        if (g->lists[i].neighbors != NULL) {
            free(g->lists[i].neighbors);
            g->lists[i].neighbors = NULL;
        }
    }
}

/* Create tree node with input validation */
TreeNode* createNode(int value) {
    /* Input validation */
    if (value < 0 || value >= MAX_NODES) {
        return NULL;
    }
    
    /* Security: check malloc return */
    TreeNode* node = (TreeNode*)calloc(1, sizeof(TreeNode));
    if (node == NULL) {
        return NULL;
    }
    
    node->value = value;
    /* Security: check malloc return */
    node->children = (int*)calloc(10, sizeof(int));
    if (node->children == NULL) {
        free(node);
        return NULL;
    }
    node->child_capacity = 10;
    node->child_count = 0;
    
    return node;
}

/* Add child to node with bounds checking */
bool addChild(TreeNode* node, int child_value) {
    if (node == NULL || child_value < 0 || child_value >= MAX_NODES) {
        return false;
    }
    
    /* Grow array if needed */
    if (node->child_count >= node->child_capacity) {
        /* Security: prevent excessive allocation */
        if (node->child_capacity >= MAX_CHILDREN) {
            return false;
        }
        
        size_t new_cap = node->child_capacity * 2;
        if (new_cap > MAX_CHILDREN) {
            new_cap = MAX_CHILDREN;
        }
        
        /* Security: check integer overflow */
        if (new_cap > SIZE_MAX / sizeof(int)) {
            return false;
        }
        
        int* new_arr = (int*)realloc(node->children, new_cap * sizeof(int));
        if (new_arr == NULL) {
            return false;
        }
        node->children = new_arr;
        node->child_capacity = new_cap;
    }
    
    node->children[node->child_count++] = child_value;
    return true;
}

/* Build tree recursively with cycle prevention */
bool buildTree(int current, int parent, Graph* g, TreeNode** nodes, 
               bool* visited, int depth) {
    /* Security: prevent stack overflow from deep recursion */
    if (depth > MAX_NODES) {
        return false;
    }
    
    /* Security: prevent cycles */
    if (visited[current]) {
        return true;
    }
    visited[current] = true;
    
    /* Input validation: bounds check */
    if (current < 0 || current >= MAX_NODES) {
        return false;
    }
    
    /* Create node */
    nodes[current] = createNode(current);
    if (nodes[current] == NULL) {
        return false;
    }
    
    /* Add children */
    if (g->lists[current].neighbors != NULL) {
        for (size_t i = 0; i < g->lists[current].count; i++) {
            int neighbor = g->lists[current].neighbors[i];
            
            /* Skip parent and already visited nodes */
            if (neighbor != parent && !visited[neighbor]) {
                if (!addChild(nodes[current], neighbor)) {
                    return false;
                }
                if (!buildTree(neighbor, current, g, nodes, visited, depth + 1)) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

/* Main reparent function */
TreeNode* reparentTree(int* edges_from, int* edges_to, int edge_count, 
                       int new_root) {
    /* Input validation */
    if (edges_from == NULL || edges_to == NULL) {
        return NULL;
    }
    if (new_root < 0 || new_root >= MAX_NODES) {
        return NULL;
    }
    if (edge_count < 0 || edge_count > MAX_NODES) {
        return NULL;
    }
    
    /* Initialize graph */
    Graph g;
    initGraph(&g);
    
    /* Build undirected graph */
    for (int i = 0; i < edge_count; i++) {
        /* Bounds check on array access */
        int from = edges_from[i];
        int to = edges_to[i];
        
        if (!addEdge(&g, from, to) || !addEdge(&g, to, from)) {
            freeGraph(&g);
            return NULL;
        }
    }
    
    /* Validate new root exists in tree */
    if (edge_count > 0 && !g.node_exists[new_root]) {
        freeGraph(&g);
        return NULL;
    }
    
    /* Security: allocate and zero-initialize tracking arrays */
    TreeNode** nodes = (TreeNode**)calloc(MAX_NODES, sizeof(TreeNode*));
    bool* visited = (bool*)calloc(MAX_NODES, sizeof(bool));
    
    if (nodes == NULL || visited == NULL) {
        free(nodes);
        free(visited);
        freeGraph(&g);
        return NULL;
    }
    
    /* Build tree from new root */
    bool success = buildTree(new_root, -1, &g, nodes, visited, 0);
    
    TreeNode* root = NULL;
    if (success && nodes[new_root] != NULL) {
        root = nodes[new_root];
    }
    
    /* Cleanup */
    free(visited);
    free(nodes);
    freeGraph(&g);
    
    return root;
}

/* Free tree node and children */
void freeTreeNode(TreeNode* node) {
    if (node == NULL) return;
    
    if (node->children != NULL) {
        free(node->children);
        node->children = NULL;
    }
    free(node);
}

int main(void) {
    /* Test case 1: Simple linear tree */
    int from1[] = {0, 1};
    int to1[] = {1, 2};
    TreeNode* root1 = reparentTree(from1, to1, 2, 1);
    if (root1 != NULL) {
        printf("Test 1 - Root: %d\\n", root1->value);
        freeTreeNode(root1);
    }
    
    /* Test case 2: Tree from problem */
    int from2[] = {0, 0, 0, 1, 1, 2, 2, 3, 3};
    int to2[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    TreeNode* root2 = reparentTree(from2, to2, 9, 6);
    if (root2 != NULL) {
        printf("Test 2 - Root: %d\\n", root2->value);
        freeTreeNode(root2);
    }
    
    /* Test case 3: Single node */
    TreeNode* root3 = reparentTree(NULL, NULL, 0, 0);
    if (root3 != NULL) {
        printf("Test 3 - Root: %d\\n", root3->value);
        freeTreeNode(root3);
    }
    
    /* Test case 4: Star topology */
    int from4[] = {0, 0, 0};
    int to4[] = {1, 2, 3};
    TreeNode* root4 = reparentTree(from4, to4, 3, 2);
    if (root4 != NULL) {
        printf("Test 4 - Root: %d\\n", root4->value);
        freeTreeNode(root4);
    }
    
    /* Test case 5: Chain */
    int from5[] = {0, 1, 2};
    int to5[] = {1, 2, 3};
    TreeNode* root5 = reparentTree(from5, to5, 3, 3);
    if (root5 != NULL) {
        printf("Test 5 - Root: %d\\n", root5->value);
        freeTreeNode(root5);
    }
    
    return 0;
}
