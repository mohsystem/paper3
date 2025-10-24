#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Edge {
    int u;
    int v;
};

struct BFSResult {
    int* order;
    size_t size;
};

static int cmp_int(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

static void print_result(const struct BFSResult* r) {
    printf("[");
    for (size_t i = 0; i < r->size; ++i) {
        if (i) printf(", ");
        printf("%d", r->order[i]);
    }
    printf("]");
}

struct BFSResult bfs(int n, const struct Edge* edges, size_t m, int start, int undirected) {
    struct BFSResult res = {NULL, 0};
    if (n <= 0 || start < 0 || start >= n) {
        return res;
    }

    size_t* deg = (size_t*)calloc((size_t)n, sizeof(size_t));
    if (!deg) return res;

    // First pass: count valid degrees
    for (size_t i = 0; i < m; ++i) {
        int u = edges[i].u;
        int v = edges[i].v;
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        deg[u]++;
        if (undirected && u != v) deg[v]++;
    }

    int** adj = (int**)calloc((size_t)n, sizeof(int*));
    size_t* idx = (size_t*)calloc((size_t)n, sizeof(size_t));
    if (!adj || !idx) {
        free(deg);
        free(adj);
        free(idx);
        return res;
    }

    int alloc_ok = 1;
    for (int i = 0; i < n; ++i) {
        if (deg[i] > 0) {
            adj[i] = (int*)malloc(deg[i] * sizeof(int));
            if (!adj[i]) {
                alloc_ok = 0;
                break;
            }
        } else {
            adj[i] = NULL;
        }
    }
    if (!alloc_ok) {
        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj);
        free(deg);
        free(idx);
        return res;
    }

    // Second pass: fill adjacency
    for (size_t i = 0; i < m; ++i) {
        int u = edges[i].u;
        int v = edges[i].v;
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        adj[u][idx[u]++] = v;
        if (undirected && u != v) adj[v][idx[v]++] = u;
    }

    // Sort and deduplicate adjacency lists
    for (int i = 0; i < n; ++i) {
        if (deg[i] > 0) {
            qsort(adj[i], deg[i], sizeof(int), cmp_int);
            size_t newlen = 0;
            for (size_t k = 0; k < deg[i]; ++k) {
                if (k == 0 || adj[i][k] != adj[i][k - 1]) {
                    adj[i][newlen++] = adj[i][k];
                }
            }
            deg[i] = newlen;
        }
    }

    // BFS
    char* visited = (char*)calloc((size_t)n, 1);
    int* queue = (int*)malloc((size_t)n * sizeof(int));
    int* order = (int*)malloc((size_t)n * sizeof(int));
    if (!visited || !queue || !order) {
        free(visited);
        free(queue);
        free(order);
        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj);
        free(deg);
        free(idx);
        return res;
    }

    size_t head = 0, tail = 0, osz = 0;
    visited[start] = 1;
    queue[tail++] = start;

    while (head < tail) {
        int u = queue[head++];
        order[osz++] = u;
        for (size_t k = 0; k < deg[u]; ++k) {
            int v = adj[u][k];
            if (!visited[v]) {
                visited[v] = 1;
                queue[tail++] = v;
            }
        }
    }

    // Cleanup adjacency
    for (int i = 0; i < n; ++i) free(adj[i]);
    free(adj);
    free(deg);
    free(idx);
    free(visited);
    free(queue);

    res.order = order;
    res.size = osz;
    return res;
}

static void free_bfs_result(struct BFSResult* r) {
    if (r && r->order) {
        free(r->order);
        r->order = NULL;
        r->size = 0;
    }
}

int main(void) {
    // Test 1: Simple undirected chain
    {
        struct Edge edges[] = {{0,1},{1,2},{2,3}};
        struct BFSResult r = bfs(4, edges, sizeof(edges)/sizeof(edges[0]), 0, 1);
        printf("Test 1: "); print_result(&r); printf("\n");
        free_bfs_result(&r);
    }
    // Test 2: Undirected graph with cycles
    {
        struct Edge edges[] = {{0,1},{0,2},{1,2},{2,3}};
        struct BFSResult r = bfs(4, edges, sizeof(edges)/sizeof(edges[0]), 1, 1);
        printf("Test 2: "); print_result(&r); printf("\n");
        free_bfs_result(&r);
    }
    // Test 3: Disconnected graph
    {
        struct Edge edges[] = {{3,4}};
        struct BFSResult r = bfs(5, edges, sizeof(edges)/sizeof(edges[0]), 3, 1);
        printf("Test 3: "); print_result(&r); printf("\n");
        free_bfs_result(&r);
    }
    // Test 4: Directed cycle
    {
        struct Edge edges[] = {{0,1},{1,2},{2,0}};
        struct BFSResult r = bfs(3, edges, sizeof(edges)/sizeof(edges[0]), 1, 0);
        printf("Test 4: "); print_result(&r); printf("\n");
        free_bfs_result(&r);
    }
    // Test 5: Invalid edges and self-loop
    {
        struct Edge edges[] = {{-1,2},{1,4},{0,0}};
        struct BFSResult r = bfs(4, edges, sizeof(edges)/sizeof(edges[0]), 0, 1);
        printf("Test 5: "); print_result(&r); printf("\n");
        free_bfs_result(&r);
    }
    return 0;
}