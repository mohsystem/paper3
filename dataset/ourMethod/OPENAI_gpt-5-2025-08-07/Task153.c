#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
bfs:
  n: number of nodes
  adj: array of int* of size n, each int* points to neighbors array
  sizes: array of int of size n, sizes[i] is number of neighbors for node i
  start: start node index
  out_order: output pointer to int* array of size <= n (allocated inside on success)
  out_size: output pointer to int (number of nodes visited)
  errbuf: buffer for error message (may be NULL)
  errbuf_len: size of errbuf
Returns: 0 on success, non-zero on error (with errbuf filled if provided)
*/
int bfs(int n, int **adj, const int *sizes, int start, int **out_order, int *out_size, char *errbuf, size_t errbuf_len) {
    if (out_order == NULL || out_size == NULL) {
        if (errbuf && errbuf_len) snprintf(errbuf, errbuf_len, "out parameters must not be NULL");
        return 1;
    }
    *out_order = NULL;
    *out_size = 0;

    if (n <= 0) {
        if (errbuf && errbuf_len) snprintf(errbuf, errbuf_len, "graph must have at least 1 node");
        return 2;
    }
    if (adj == NULL || sizes == NULL) {
        if (errbuf && errbuf_len) snprintf(errbuf, errbuf_len, "adj and sizes must not be NULL");
        return 3;
    }
    if (start < 0 || start >= n) {
        if (errbuf && errbuf_len) snprintf(errbuf, errbuf_len, "start node out of range");
        return 4;
    }
    for (int i = 0; i < n; i++) {
        if (sizes[i] < 0) {
            if (errbuf && errbuf_len) snprintf(errbuf, errbuf_len, "sizes[%d] is negative", i);
            return 5;
        }
        if (sizes[i] > 0 && adj[i] == NULL) {
            if (errbuf && errbuf_len) snprintf(errbuf, errbuf_len, "adj[%d] is NULL with positive size", i);
            return 6;
        }
        for (int k = 0; k < sizes[i]; k++) {
            int v = adj[i][k];
            if (v < 0 || v >= n) {
                if (errbuf && errbuf_len) snprintf(errbuf, errbuf_len, "edge from %d to invalid node %d", i, v);
                return 7;
            }
        }
    }

    char *visited = (char *)calloc((size_t)n, sizeof(char));
    if (!visited) {
        if (errbuf && errbuf_len) snprintf(errbuf, errbuf_len, "memory allocation failed (visited)");
        return 8;
    }
    int *queue = (int *)malloc((size_t)n * sizeof(int));
    if (!queue) {
        free(visited);
        if (errbuf && errbuf_len) snprintf(errbuf, errbuf_len, "memory allocation failed (queue)");
        return 9;
    }
    int *order = (int *)malloc((size_t)n * sizeof(int));
    if (!order) {
        free(visited);
        free(queue);
        if (errbuf && errbuf_len) snprintf(errbuf, errbuf_len, "memory allocation failed (order)");
        return 10;
    }

    int head = 0, tail = 0, out_idx = 0;
    visited[start] = 1;
    queue[tail++] = start;

    while (head < tail) {
        int u = queue[head++];
        order[out_idx++] = u;
        for (int i = 0; i < sizes[u]; i++) {
            int v = adj[u][i];
            if (!visited[v]) {
                visited[v] = 1;
                if (tail >= n) {
                    // Should never happen, but check bounds
                    free(visited);
                    free(queue);
                    free(order);
                    if (errbuf && errbuf_len) snprintf(errbuf, errbuf_len, "queue overflow");
                    return 11;
                }
                queue[tail++] = v;
            }
        }
    }

    free(visited);
    free(queue);
    *out_order = order;
    *out_size = out_idx;
    return 0;
}

/* Helper to print BFS order */
static void print_order(const int *order, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        if (i > 0) printf(" ");
        printf("%d", order[i]);
    }
    printf("]\n");
}

/* Helpers to build and free adjacency lists for tests */
static int **alloc_adj(int n, const int *sizes) {
    int **adj = (int **)calloc((size_t)n, sizeof(int *));
    if (!adj) return NULL;
    for (int i = 0; i < n; i++) {
        if (sizes[i] > 0) {
            adj[i] = (int *)malloc((size_t)sizes[i] * sizeof(int));
            if (!adj[i]) {
                for (int j = 0; j < i; j++) free(adj[j]);
                free(adj);
                return NULL;
            }
        } else {
            adj[i] = NULL;
        }
    }
    return adj;
}

static void free_adj(int n, int **adj) {
    if (!adj) return;
    for (int i = 0; i < n; i++) {
        free(adj[i]);
    }
    free(adj);
}

int main(void) {
    char err[128];

    // Test 1: Simple line graph 0-1-2-3 from 0
    {
        int n = 4;
        int sizes[4] = {1, 1, 1, 0};
        int **adj = alloc_adj(n, sizes);
        if (!adj) {
            printf("Test 1 Error: allocation failed\n");
        } else {
            adj[0][0] = 1;
            adj[1][0] = 2;
            adj[2][0] = 3;
            int *order = NULL, out_size = 0;
            int rc = bfs(n, adj, sizes, 0, &order, &out_size, err, sizeof(err));
            if (rc == 0) {
                printf("Test 1: ");
                print_order(order, out_size);
                free(order);
            } else {
                printf("Test 1 Error: %s\n", err);
            }
            free_adj(n, adj);
        }
    }

    // Test 2: Graph with cycles
    {
        int n = 4;
        int sizes[4] = {2, 1, 2, 0};
        int **adj = alloc_adj(n, sizes);
        if (!adj) {
            printf("Test 2 Error: allocation failed\n");
        } else {
            adj[0][0] = 1; adj[0][1] = 2;
            adj[1][0] = 2;
            adj[2][0] = 0; adj[2][1] = 3;
            int *order = NULL, out_size = 0;
            int rc = bfs(n, adj, sizes, 1, &order, &out_size, err, sizeof(err));
            if (rc == 0) {
                printf("Test 2: ");
                print_order(order, out_size);
                free(order);
            } else {
                printf("Test 2 Error: %s\n", err);
            }
            free_adj(n, adj);
        }
    }

    // Test 3: Disconnected graph start at 3
    {
        int n = 5;
        int sizes[5] = {1, 1, 0, 1, 0};
        int **adj = alloc_adj(n, sizes);
        if (!adj) {
            printf("Test 3 Error: allocation failed\n");
        } else {
            adj[0][0] = 1;
            adj[1][0] = 2;
            adj[3][0] = 4;
            int *order = NULL, out_size = 0;
            int rc = bfs(n, adj, sizes, 3, &order, &out_size, err, sizeof(err));
            if (rc == 0) {
                printf("Test 3: ");
                print_order(order, out_size);
                free(order);
            } else {
                printf("Test 3 Error: %s\n", err);
            }
            free_adj(n, adj);
        }
    }

    // Test 4: Invalid start node (-1)
    {
        int n = 3;
        int sizes[3] = {1, 1, 0};
        int **adj = alloc_adj(n, sizes);
        if (!adj) {
            printf("Test 4 Error: allocation failed\n");
        } else {
            adj[0][0] = 1;
            adj[1][0] = 2;
            int *order = NULL, out_size = 0;
            int rc = bfs(n, adj, sizes, -1, &order, &out_size, err, sizeof(err));
            if (rc == 0) {
                printf("Test 4: ");
                print_order(order, out_size);
                free(order);
            } else {
                printf("Test 4 Error: %s\n", err);
            }
            free_adj(n, adj);
        }
    }

    // Test 5: Invalid edge reference (neighbor 99)
    {
        int n = 3;
        int sizes[3] = {2, 1, 0};
        int **adj = alloc_adj(n, sizes);
        if (!adj) {
            printf("Test 5 Error: allocation failed\n");
        } else {
            adj[0][0] = 1; adj[0][1] = 99; // invalid
            adj[1][0] = 2;
            int *order = NULL, out_size = 0;
            int rc = bfs(n, adj, sizes, 0, &order, &out_size, err, sizeof(err));
            if (rc == 0) {
                printf("Test 5: ");
                print_order(order, out_size);
                free(order);
            } else {
                printf("Test 5 Error: %s\n", err);
            }
            free_adj(n, adj);
        }
    }

    return 0;
}