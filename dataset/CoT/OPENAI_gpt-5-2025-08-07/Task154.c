#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Comparator for qsort
static int cmp_int(const void* a, const void* b) {
    int x = *(const int*)a;
    int y = *(const int*)b;
    return (x > y) - (x < y);
}

// Depth-First Search (DFS) iterative for C
// n: number of nodes
// edges: m x 2 array of edges (u, v)
// m: number of edges
// start: starting node
// directed: non-zero for directed, zero for undirected
// out_len: output length of traversal
// Returns dynamically allocated array of traversal order; caller must free. On error, returns NULL and sets *out_len = 0.
int* dfs(int n, const int edges[][2], int m, int start, int directed, int* out_len) {
    if (out_len == NULL) return NULL;
    *out_len = 0;

    if (n <= 0) return NULL;
    if (start < 0 || start >= n) return NULL;
    if (m < 0) return NULL;

    int* deg = (int*)calloc((size_t)n, sizeof(int));
    if (!deg) return NULL;

    // Validate edges and compute degrees
    for (int i = 0; i < m; ++i) {
        int u = edges[i][0];
        int v = edges[i][1];
        if (u < 0 || u >= n || v < 0 || v >= n) {
            free(deg);
            return NULL;
        }
        deg[u]++;
        if (!directed) deg[v]++;
    }

    int** adj = (int**)malloc((size_t)n * sizeof(int*));
    if (!adj) {
        free(deg);
        return NULL;
    }
    int* idx = (int*)calloc((size_t)n, sizeof(int));
    if (!idx) {
        free(adj);
        free(deg);
        return NULL;
    }

    for (int i = 0; i < n; ++i) {
        if (deg[i] > 0) {
            adj[i] = (int*)malloc((size_t)deg[i] * sizeof(int));
            if (!adj[i]) {
                for (int j = 0; j < i; ++j) free(adj[j]);
                free(adj);
                free(deg);
                free(idx);
                return NULL;
            }
        } else {
            adj[i] = NULL;
        }
    }

    // Fill adjacency
    for (int i = 0; i < m; ++i) {
        int u = edges[i][0];
        int v = edges[i][1];
        adj[u][idx[u]++] = v;
        if (!directed) {
            adj[v][idx[v]++] = u;
        }
    }

    // Sort adjacency lists
    for (int i = 0; i < n; ++i) {
        if (deg[i] > 0) {
            qsort(adj[i], (size_t)deg[i], sizeof(int), cmp_int);
        }
    }

    bool* visited = (bool*)calloc((size_t)n, sizeof(bool));
    if (!visited) {
        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj);
        free(deg);
        free(idx);
        return NULL;
    }

    int* stack = (int*)malloc((size_t)n * sizeof(int));
    if (!stack) {
        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj);
        free(deg);
        free(idx);
        free(visited);
        return NULL;
    }

    int* result = (int*)malloc((size_t)n * sizeof(int));
    if (!result) {
        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj);
        free(deg);
        free(idx);
        free(visited);
        free(stack);
        return NULL;
    }

    int top = -1;
    stack[++top] = start;
    int rlen = 0;

    while (top >= 0) {
        int u = stack[top--];
        if (visited[u]) continue;
        visited[u] = true;
        result[rlen++] = u;

        for (int i = deg[u] - 1; i >= 0; --i) {
            int v = adj[u][i];
            if (!visited[v]) {
                stack[++top] = v;
            }
        }
    }

    *out_len = rlen;

    // Cleanup
    for (int i = 0; i < n; ++i) free(adj[i]);
    free(adj);
    free(deg);
    free(idx);
    free(visited);
    free(stack);

    return result;
}

static void print_array(const int* arr, int len) {
    printf("[");
    for (int i = 0; i < len; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    // Test case 1: Simple chain undirected
    {
        int n = 4;
        int edges[][2] = {{0,1},{1,2},{2,3}};
        int m = (int)(sizeof(edges)/sizeof(edges[0]));
        int out_len = 0;
        int* res = dfs(n, edges, m, 0, 0, &out_len);
        printf("Test 1: ");
        if (res) {
            print_array(res, out_len);
            free(res);
        } else {
            printf("Error\n");
        }
    }

    // Test case 2: Branching undirected
    {
        int n = 5;
        int edges[][2] = {{0,1},{0,2},{1,3},{1,4}};
        int m = (int)(sizeof(edges)/sizeof(edges[0]));
        int out_len = 0;
        int* res = dfs(n, edges, m, 0, 0, &out_len);
        printf("Test 2: ");
        if (res) {
            print_array(res, out_len);
            free(res);
        } else {
            printf("Error\n");
        }
    }

    // Test case 3: Directed with cycle
    {
        int n = 4;
        int edges[][2] = {{0,1},{1,2},{2,0},{2,3}};
        int m = (int)(sizeof(edges)/sizeof(edges[0]));
        int out_len = 0;
        int* res = dfs(n, edges, m, 0, 1, &out_len);
        printf("Test 3: ");
        if (res) {
            print_array(res, out_len);
            free(res);
        } else {
            printf("Error\n");
        }
    }

    // Test case 4: Isolated node
    {
        int n = 5;
        int edges[][2] = {{0,1},{1,2}};
        int m = (int)(sizeof(edges)/sizeof(edges[0]));
        int out_len = 0;
        int* res = dfs(n, edges, m, 4, 0, &out_len);
        printf("Test 4: ");
        if (res) {
            print_array(res, out_len);
            free(res);
        } else {
            printf("Error\n");
        }
    }

    // Test case 5: Invalid start node
    {
        int n = 3;
        int edges[][2] = {{0,1}};
        int m = (int)(sizeof(edges)/sizeof(edges[0]));
        int out_len = 0;
        int* res = dfs(n, edges, m, 7, 0, &out_len);
        printf("Test 5: ");
        if (res) {
            print_array(res, out_len);
            free(res);
        } else {
            printf("Error\n");
        }
    }

    return 0;
}