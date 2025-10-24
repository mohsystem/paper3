#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*
Chain-of-Through process:
1) Problem understanding:
   Re-root an undirected tree at newRoot and return a parent array with parent[newRoot] = -1.
2) Security requirements:
   Validate inputs: n > 0, edgesCount == n-1, node indices within [0,n), no self-loops, connectivity.
3) Secure coding generation:
   Use iterative DFS to avoid recursion depth issues; check all allocations; free allocated memory on error paths.
4) Code review:
   Ensure bounds checks, allocation checks, and no use-after-free or memory leaks in error paths.
5) Secure code output:
   Return NULL on invalid inputs; caller handles and frees memory where applicable.
*/

static int* reparentTreeC(int n, const int (*edges)[2], int edgesCount, int newRoot) {
    if (n <= 0) return NULL;
    if (edges == NULL) return NULL;
    if (edgesCount != n - 1) return NULL;
    if (newRoot < 0 || newRoot >= n) return NULL;

    int* degree = (int*)calloc(n, sizeof(int));
    if (!degree) return NULL;

    // Validate edges and compute degrees
    for (int i = 0; i < edgesCount; ++i) {
        int u = edges[i][0];
        int v = edges[i][1];
        if (u < 0 || u >= n || v < 0 || v >= n) {
            free(degree);
            return NULL;
        }
        if (u == v) {
            free(degree);
            return NULL;
        }
        degree[u]++;
        degree[v]++;
    }

    // Allocate adjacency
    int** adj = (int**)malloc(n * sizeof(int*));
    if (!adj) {
        free(degree);
        return NULL;
    }
    int* fill = (int*)calloc(n, sizeof(int));
    if (!fill) {
        free(adj);
        free(degree);
        return NULL;
    }
    for (int i = 0; i < n; ++i) {
        adj[i] = (int*)malloc((degree[i] > 0 ? degree[i] : 1) * sizeof(int));
        if (!adj[i]) {
            for (int j = 0; j < i; ++j) free(adj[j]);
            free(adj);
            free(degree);
            free(fill);
            return NULL;
        }
    }

    // Fill adjacency
    for (int i = 0; i < edgesCount; ++i) {
        int u = edges[i][0];
        int v = edges[i][1];
        adj[u][fill[u]++] = v;
        adj[v][fill[v]++] = u;
    }

    // Iterative DFS to orient from newRoot
    int* parent = (int*)malloc(n * sizeof(int));
    if (!parent) {
        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj);
        free(degree);
        free(fill);
        return NULL;
    }
    for (int i = 0; i < n; ++i) parent[i] = INT_MIN;

    int* stack = (int*)malloc(n * sizeof(int));
    if (!stack) {
        free(parent);
        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj);
        free(degree);
        free(fill);
        return NULL;
    }
    int top = 0;
    parent[newRoot] = -1;
    stack[top++] = newRoot;

    while (top > 0) {
        int u = stack[--top];
        for (int i = 0; i < degree[u]; ++i) {
            int v = adj[u][i];
            if (parent[v] == INT_MIN) {
                parent[v] = u;
                stack[top++] = v;
            }
        }
    }

    // Validate connectivity
    for (int i = 0; i < n; ++i) {
        if (parent[i] == INT_MIN) {
            // Not connected
            free(parent);
            parent = NULL;
            break;
        }
    }

    // Cleanup
    for (int i = 0; i < n; ++i) free(adj[i]);
    free(adj);
    free(degree);
    free(fill);
    free(stack);

    return parent;
}

static void printArrayC(const int* arr, int n) {
    if (!arr) {
        printf("Invalid input or not a tree\n");
        return;
    }
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    // 5 test cases
    {
        int n = 10;
        const int edges[][2] = {
            {0,1},{0,2},{0,3},{1,4},{1,5},{2,6},{2,7},{3,8},{3,9}
        };
        int* parent = reparentTreeC(n, edges, sizeof(edges)/sizeof(edges[0]), 6);
        printArrayC(parent, n);
        free(parent);
    }
    {
        int n = 10;
        const int edges[][2] = {
            {0,1},{0,2},{0,3},{1,4},{1,5},{2,6},{2,7},{3,8},{3,9}
        };
        int* parent = reparentTreeC(n, edges, sizeof(edges)/sizeof(edges[0]), 0);
        printArrayC(parent, n);
        free(parent);
    }
    {
        int n = 5;
        const int edges[][2] = {
            {0,1},{1,2},{2,3},{3,4}
        };
        int* parent = reparentTreeC(n, edges, sizeof(edges)/sizeof(edges[0]), 4);
        printArrayC(parent, n);
        free(parent);
    }
    {
        int n = 6;
        const int edges[][2] = {
            {0,1},{0,2},{0,3},{0,4},{0,5}
        };
        int* parent = reparentTreeC(n, edges, sizeof(edges)/sizeof(edges[0]), 3);
        printArrayC(parent, n);
        free(parent);
    }
    {
        int n = 5;
        const int edges[][2] = {
            {0,1},{1,2},{1,3},{3,4}
        };
        int* parent = reparentTreeC(n, edges, sizeof(edges)/sizeof(edges[0]), 4);
        printArrayC(parent, n);
        free(parent);
    }

    return 0;
}