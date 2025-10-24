#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int ok;          /* 1 for success, 0 for failure */
    int* order;      /* DFS order array (size = order_len) */
    int order_len;   /* length of order */
    char error[128]; /* error message if not ok */
} DFSResult;

static void safe_set_error(DFSResult* res, const char* msg) {
    if (!res || !msg) return;
    res->ok = 0;
    res->order = NULL;
    res->order_len = 0;
    /* Use bounded copy to avoid overflow */
    snprintf(res->error, sizeof(res->error), "%s", msg);
}

static DFSResult dfs_traversal(int n, int** adj, int* adj_sizes, int start) {
    DFSResult res;
    res.ok = 0;
    res.order = NULL;
    res.order_len = 0;
    res.error[0] = '\0';

    /* Validate basic inputs */
    if (n <= 0) {
        safe_set_error(&res, "Invalid number of nodes (must be > 0).");
        return res;
    }
    if (adj == NULL || adj_sizes == NULL) {
        safe_set_error(&res, "Adjacency pointers must not be NULL.");
        return res;
    }
    if (start < 0 || start >= n) {
        safe_set_error(&res, "Start node is out of range.");
        return res;
    }
    /* Validate adjacency list contents */
    for (int i = 0; i < n; i++) {
        if (adj_sizes[i] < 0) {
            safe_set_error(&res, "Adjacency size is negative.");
            return res;
        }
        if (adj_sizes[i] > 0 && adj[i] == NULL) {
            safe_set_error(&res, "Adjacency has NULL row with positive size.");
            return res;
        }
        for (int j = 0; j < adj_sizes[i]; j++) {
            int v = adj[i][j];
            if (v < 0 || v >= n) {
                safe_set_error(&res, "Edge points to out-of-range node.");
                return res;
            }
        }
    }

    int* order = (int*)malloc((size_t)n * sizeof(int));
    if (!order) {
        safe_set_error(&res, "Memory allocation failed for order.");
        return res;
    }
    int* stack = (int*)malloc((size_t)n * sizeof(int));
    if (!stack) {
        free(order);
        safe_set_error(&res, "Memory allocation failed for stack.");
        return res;
    }
    unsigned char* visited = (unsigned char*)calloc((size_t)n, sizeof(unsigned char));
    if (!visited) {
        free(order);
        free(stack);
        safe_set_error(&res, "Memory allocation failed for visited.");
        return res;
    }

    int top = -1;
    stack[++top] = start;
    int out_len = 0;

    while (top >= 0) {
        int u = stack[top--];
        if (visited[u]) {
            continue;
        }
        visited[u] = 1;
        order[out_len++] = u;
        /* Push neighbors in reverse order for predictability */
        for (int i = adj_sizes[u] - 1; i >= 0; i--) {
            int v = adj[u][i];
            if (!visited[v]) {
                stack[++top] = v;
            }
        }
    }

    free(stack);
    free(visited);

    res.ok = 1;
    res.order = order;
    res.order_len = out_len;
    res.error[0] = '\0';
    return res;
}

/* Helper to build adjacency for tests */
static void build_adj(int n, const int edges[][2], int edges_len, int undirected, int*** out_adj, int** out_sizes) {
    int** adj = (int**)calloc((size_t)n, sizeof(int*));
    int* sizes = (int*)calloc((size_t)n, sizeof(int));
    int* caps = (int*)calloc((size_t)n, sizeof(int));
    if (!adj || !sizes || !caps) {
        fprintf(stderr, "Memory allocation failed in build_adj.\n");
        free(adj); free(sizes); free(caps);
        *out_adj = NULL;
        *out_sizes = NULL;
        return;
    }

    for (int i = 0; i < edges_len; i++) {
        int u = edges[i][0];
        int v = edges[i][1];
        if (u >= 0 && u < n && v >= 0 && v < n) {
            if (sizes[u] == caps[u]) {
                int new_cap = caps[u] == 0 ? 2 : caps[u] * 2;
                int* tmp = (int*)realloc(adj[u], (size_t)new_cap * sizeof(int));
                if (!tmp) {
                    fprintf(stderr, "Memory allocation failed when growing adjacency.\n");
                    // Free allocated memory
                    for (int k = 0; k < n; k++) free(adj[k]);
                    free(adj); free(sizes); free(caps);
                    *out_adj = NULL; *out_sizes = NULL;
                    return;
                }
                adj[u] = tmp;
                caps[u] = new_cap;
            }
            adj[u][sizes[u]++] = v;

            if (undirected) {
                if (sizes[v] == caps[v]) {
                    int new_cap2 = caps[v] == 0 ? 2 : caps[v] * 2;
                    int* tmp2 = (int*)realloc(adj[v], (size_t)new_cap2 * sizeof(int));
                    if (!tmp2) {
                        fprintf(stderr, "Memory allocation failed when growing adjacency.\n");
                        for (int k = 0; k < n; k++) free(adj[k]);
                        free(adj); free(sizes); free(caps);
                        *out_adj = NULL; *out_sizes = NULL;
                        return;
                    }
                    adj[v] = tmp2;
                    caps[v] = new_cap2;
                }
                adj[v][sizes[v]++] = u;
            }
        }
    }

    free(caps);
    *out_adj = adj;
    *out_sizes = sizes;
}

static void free_adj(int n, int** adj, int* sizes) {
    if (adj) {
        for (int i = 0; i < n; i++) {
            free(adj[i]);
        }
        free(adj);
    }
    free(sizes);
}

static void print_result(const char* label, DFSResult res) {
    printf("%s: ", label);
    if (!res.ok) {
        printf("Error: %s\n", res.error);
        return;
    }
    printf("[");
    for (int i = 0; i < res.order_len; i++) {
        if (i) printf(", ");
        printf("%d", res.order[i]);
    }
    printf("]\n");
    free(res.order);
}

int main(void) {
    /* Test 1: Undirected chain 0-1-2-3, start=0 */
    {
        int n = 4;
        const int edges[][2] = { {0,1}, {1,2}, {2,3} };
        int** adj = NULL; int* sizes = NULL;
        build_adj(n, edges, (int)(sizeof(edges)/sizeof(edges[0])), 1, &adj, &sizes);
        if (!adj || !sizes) return 1;
        DFSResult res = dfs_traversal(n, adj, sizes, 0);
        print_result("Test 1", res);
        free_adj(n, adj, sizes);
    }

    /* Test 2: Directed cycle 0->1->2->0 and 2->3, start=0 */
    {
        int n = 4;
        const int edges[][2] = { {0,1}, {1,2}, {2,0}, {2,3} };
        int** adj = NULL; int* sizes = NULL;
        build_adj(n, edges, (int)(sizeof(edges)/sizeof(edges[0])), 0, &adj, &sizes);
        if (!adj || !sizes) return 1;
        DFSResult res = dfs_traversal(n, adj, sizes, 0);
        print_result("Test 2", res);
        free_adj(n, adj, sizes);
    }

    /* Test 3: Disconnected graph, start in second component */
    {
        int n = 5;
        const int edges[][2] = { {0,1}, {1,0}, {2,3}, {3,2} };
        int** adj = NULL; int* sizes = NULL;
        build_adj(n, edges, (int)(sizeof(edges)/sizeof(edges[0])), 0, &adj, &sizes);
        if (!adj || !sizes) return 1;
        DFSResult res = dfs_traversal(n, adj, sizes, 2);
        print_result("Test 3", res);
        free_adj(n, adj, sizes);
    }

    /* Test 4: Single node graph */
    {
        int n = 1;
        const int edges[][2] = { };
        int** adj = NULL; int* sizes = NULL;
        build_adj(n, edges, 0, 0, &adj, &sizes);
        if (!adj || !sizes) return 1;
        DFSResult res = dfs_traversal(n, adj, sizes, 0);
        print_result("Test 4", res);
        free_adj(n, adj, sizes);
    }

    /* Test 5: Invalid start node */
    {
        int n = 3;
        const int edges[][2] = { {0,1}, {1,2} };
        int** adj = NULL; int* sizes = NULL;
        build_adj(n, edges, (int)(sizeof(edges)/sizeof(edges[0])), 0, &adj, &sizes);
        if (!adj || !sizes) return 1;
        DFSResult res = dfs_traversal(n, adj, sizes, 5);
        print_result("Test 5", res);
        free_adj(n, adj, sizes);
    }

    return 0;
}