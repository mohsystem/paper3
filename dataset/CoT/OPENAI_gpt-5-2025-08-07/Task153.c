/* Secure BFS implementation in C with test cases. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Result type to return BFS order and its size */
typedef struct {
    int* order;
    int size;
} BFSResult;

/* Safe allocation helpers */
static void* xcalloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0) {
        nmemb = nmemb ? nmemb : 1;
        size = size ? size : 1;
    }
    if (nmemb > SIZE_MAX / size) return NULL; /* overflow check */
    void* p = calloc(nmemb, size);
    return p;
}

static int* xmalloc_int_array(size_t count) {
    if (count == 0) count = 1;
    if (count > SIZE_MAX / sizeof(int)) return NULL;
    return (int*)malloc(count * sizeof(int));
}

/* BFS on graph represented as:
   - n: number of nodes (0..n-1)
   - adj: array of int* of length n; adj[u] points to neighbors array
   - adj_len: array of ints of length n; adj_len[u] is number of neighbors
   Returns BFSResult with allocated order array that caller must free.
   If start is out of bounds, returns order containing only start.
*/
BFSResult bfs_c(int n, int** adj, int* adj_len, int start) {
    BFSResult res;
    res.order = NULL;
    res.size = 0;

    if (n < 0) return res;

    if (start < 0 || start >= n) {
        res.order = xmalloc_int_array(1);
        if (res.order) {
            res.order[0] = start;
            res.size = 1;
        }
        return res;
    }

    int* order = xmalloc_int_array((size_t)n);
    char* visited = (char*)xcalloc((size_t)n, sizeof(char));
    int* queue = xmalloc_int_array((size_t)n);
    if (!order || !visited || !queue) {
        free(order); free(visited); free(queue);
        return res;
    }

    int qh = 0, qt = 0;
    int ord_sz = 0;

    visited[start] = 1;
    queue[qt++] = start;

    while (qh < qt) {
        int cur = queue[qh++];
        order[ord_sz++] = cur;

        int* neighbors = NULL;
        int deg = 0;
        if (adj != NULL && adj_len != NULL && cur >= 0 && cur < n) {
            neighbors = adj[cur];
            deg = adj_len[cur];
            if (deg < 0) deg = 0;
        }

        for (int i = 0; i < deg; ++i) {
            int nb = neighbors[i];
            if (nb < 0 || nb >= n) continue; /* defensive bounds check */
            if (!visited[nb]) {
                visited[nb] = 1;
                if (qt < n) { /* avoid overflow for safety */
                    queue[qt++] = nb;
                }
            }
        }
    }

    // shrink to fit
    int* shrink = (int*)realloc(order, (size_t)ord_sz * sizeof(int));
    if (shrink) order = shrink;

    res.order = order;
    res.size = ord_sz;

    free(visited);
    free(queue);
    return res;
}

/* Utilities for testing */
static void print_result(const char* label, BFSResult r) {
    printf("%s: [", label);
    for (int i = 0; i < r.size; ++i) {
        if (i) printf(", ");
        printf("%d", r.order[i]);
    }
    printf("]\n");
}

static void free_bfs_result(BFSResult r) {
    free(r.order);
}

/* Helper to add undirected edge to adjacency containers built with dynamic arrays for tests */
static void add_undirected_edge(int u, int v, int** adj, int* adj_len, int* adj_cap, int n) {
    if (u >= 0 && u < n && v >= 0 && v < n) {
        // ensure capacity for u
        if (adj_len[u] >= adj_cap[u]) {
            int newcap = adj_cap[u] ? adj_cap[u] * 2 : 4;
            int* tmp = (int*)realloc(adj[u], (size_t)newcap * sizeof(int));
            if (!tmp) return;
            adj[u] = tmp;
            adj_cap[u] = newcap;
        }
        // avoid duplicate
        int dup = 0;
        for (int i = 0; i < adj_len[u]; ++i) if (adj[u][i] == v) { dup = 1; break; }
        if (!dup) adj[u][adj_len[u]++] = v;

        // ensure capacity for v
        if (adj_len[v] >= adj_cap[v]) {
            int newcap = adj_cap[v] ? adj_cap[v] * 2 : 4;
            int* tmp = (int*)realloc(adj[v], (size_t)newcap * sizeof(int));
            if (!tmp) return;
            adj[v] = tmp;
            adj_cap[v] = newcap;
        }
        dup = 0;
        for (int i = 0; i < adj_len[v]; ++i) if (adj[v][i] == u) { dup = 1; break; }
        if (!dup) adj[v][adj_len[v]++] = u;
    }
}

int main(void) {
    // Test 1: Simple chain 0-1-2-3 (undirected)
    {
        int n = 4;
        int** adj = (int**)xcalloc((size_t)n, sizeof(int*));
        int* adj_len = (int*)xcalloc((size_t)n, sizeof(int));
        int* adj_cap = (int*)xcalloc((size_t)n, sizeof(int));
        add_undirected_edge(0,1,adj,adj_len,adj_cap,n);
        add_undirected_edge(1,2,adj,adj_len,adj_cap,n);
        add_undirected_edge(2,3,adj,adj_len,adj_cap,n);
        BFSResult r = bfs_c(n, adj, adj_len, 0);
        print_result("Test1", r);
        free_bfs_result(r);
        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj); free(adj_len); free(adj_cap);
    }

    // Test 2: Cycle 0-1-2-0 with branch 1-3 (undirected)
    {
        int n = 4;
        int** adj = (int**)xcalloc((size_t)n, sizeof(int*));
        int* adj_len = (int*)xcalloc((size_t)n, sizeof(int));
        int* adj_cap = (int*)xcalloc((size_t)n, sizeof(int));
        add_undirected_edge(0,1,adj,adj_len,adj_cap,n);
        add_undirected_edge(1,2,adj,adj_len,adj_cap,n);
        add_undirected_edge(2,0,adj,adj_len,adj_cap,n);
        add_undirected_edge(1,3,adj,adj_len,adj_cap,n);
        BFSResult r = bfs_c(n, adj, adj_len, 0);
        print_result("Test2", r);
        free_bfs_result(r);
        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj); free(adj_len); free(adj_cap);
    }

    // Test 3: Disconnected with isolated node 5
    {
        int n = 6;
        int** adj = (int**)xcalloc((size_t)n, sizeof(int*));
        int* adj_len = (int*)xcalloc((size_t)n, sizeof(int));
        int* adj_cap = (int*)xcalloc((size_t)n, sizeof(int));
        // no edges for node 5
        BFSResult r = bfs_c(n, adj, adj_len, 5);
        print_result("Test3", r);
        free_bfs_result(r);
        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj); free(adj_len); free(adj_cap);
    }

    // Test 4: Ordered neighbors 0 -> [3,1,2], with deeper nodes
    {
        int n = 6;
        int** adj = (int**)xcalloc((size_t)n, sizeof(int*));
        int* adj_len = (int*)xcalloc((size_t)n, sizeof(int));
        int* adj_cap = (int*)xcalloc((size_t)n, sizeof(int));

        // Manually push neighbors in desired order for node 0
        // ensure capacity for node 0
        int cap0 = 4;
        adj[0] = (int*)malloc((size_t)cap0 * sizeof(int));
        if (adj[0]) {
            adj[0][0] = 3; adj[0][1] = 1; adj[0][2] = 2;
            adj_len[0] = 3;
        }
        // Node 1 -> [4]
        adj[1] = (int*)malloc(sizeof(int));
        if (adj[1]) { adj[1][0] = 4; adj_len[1] = 1; }
        // Node 2 -> []
        // Node 3 -> [5]
        adj[3] = (int*)malloc(sizeof(int));
        if (adj[3]) { adj[3][0] = 5; adj_len[3] = 1; }
        // Node 4 -> []
        // Node 5 -> []

        BFSResult r = bfs_c(n, adj, adj_len, 0);
        print_result("Test4", r);
        free_bfs_result(r);

        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj); free(adj_len); free(adj_cap);
    }

    // Test 5: Start not present in graph (start index 99 beyond n)
    {
        int n = 2;
        int** adj = (int**)xcalloc((size_t)n, sizeof(int*));
        int* adj_len = (int*)xcalloc((size_t)n, sizeof(int));
        int* adj_cap = (int*)xcalloc((size_t)n, sizeof(int));
        add_undirected_edge(0,1,adj,adj_len,adj_cap,n);
        BFSResult r = bfs_c(n, adj, adj_len, 99);
        print_result("Test5", r);
        free_bfs_result(r);
        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj); free(adj_len); free(adj_cap);
    }

    return 0;
}