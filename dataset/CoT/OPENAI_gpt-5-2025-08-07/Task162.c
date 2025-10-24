#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

/*
Chain-of-Through process:
1) Problem understanding: Compute shortest paths from a start node using Dijkstra on non-negative weighted graphs.
2) Security: Validate inputs, avoid overflows using safe INF, check bounds, handle unreachable nodes.
3) Secure coding: Build adjacency with validation, linear selection Dijkstra to avoid complex heap vulnerabilities, safe memory management.
4) Code review: Ensure checks for invalid edges and indices, prevent addition overflow, free all allocations.
5) Secure output: Final code with 5 tests demonstrating correct behavior.
*/

struct Edge {
    int u;
    int v;
    long long w;
};

// Returns 0 on success, non-zero on invalid input. Outputs distances and parents via provided arrays.
int dijkstra(int n, const struct Edge* edges, int m, int start, long long* dist_out, int* parent_out) {
    if (n <= 0 || !dist_out || !parent_out) return -1;
    if (start < 0 || start >= n) return -2;

    // Count valid edges per node for adjacency
    int* deg = (int*)calloc((size_t)n, sizeof(int));
    if (!deg) return -3;

    int valid_m = 0;
    for (int i = 0; i < m; i++) {
        int u = edges[i].u, v = edges[i].v;
        long long w = edges[i].w;
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        if (w < 0) continue;
        deg[u]++;
        valid_m++;
    }

    int* offset = (int*)calloc((size_t)(n + 1), sizeof(int));
    if (!offset) { free(deg); return -3; }
    for (int i = 0; i < n; i++) offset[i + 1] = offset[i] + deg[i];

    int total = offset[n];
    int* to = (int*)malloc((size_t)total * sizeof(int));
    long long* w = (long long*)malloc((size_t)total * sizeof(long long));
    if (!to || !w) {
        free(deg); free(offset); if (to) free(to); if (w) free(w);
        return -3;
    }
    int* cur = (int*)calloc((size_t)n, sizeof(int));
    if (!cur) {
        free(deg); free(offset); free(to); free(w);
        return -3;
    }

    // Fill adjacency
    for (int i = 0; i < m; i++) {
        int u = edges[i].u, v = edges[i].v;
        long long ww = edges[i].w;
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        if (ww < 0) continue;
        int idx = offset[u] + cur[u];
        to[idx] = v;
        w[idx] = ww;
        cur[u]++;
    }

    const long long INF = LLONG_MAX / 4;
    for (int i = 0; i < n; i++) {
        dist_out[i] = INF;
        parent_out[i] = -1;
    }
    dist_out[start] = 0;

    unsigned char* used = (unsigned char*)calloc((size_t)n, sizeof(unsigned char));
    if (!used) {
        free(deg); free(offset); free(to); free(w); free(cur);
        return -3;
    }

    for (int it = 0; it < n; it++) {
        int u = -1;
        long long best = INF;
        for (int i = 0; i < n; i++) {
            if (!used[i] && dist_out[i] < best) {
                best = dist_out[i];
                u = i;
            }
        }
        if (u == -1) break;
        used[u] = 1;

        for (int j = offset[u]; j < offset[u + 1]; j++) {
            int v = to[j];
            long long ww = w[j];
            if (dist_out[u] <= INF - ww) {
                long long nd = dist_out[u] + ww;
                if (nd < dist_out[v]) {
                    dist_out[v] = nd;
                    parent_out[v] = u;
                }
            }
        }
    }

    free(deg);
    free(offset);
    free(to);
    free(w);
    free(cur);
    free(used);
    (void)valid_m; // not used further, avoid unused warning
    return 0;
}

static void print_ll_array(const long long* a, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        if (i) printf(", ");
        if (a[i] >= LLONG_MAX / 8) printf("INF");
        else printf("%lld", a[i]);
    }
    printf("]\n");
}

static void print_int_array(const int* a, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        if (i) printf(", ");
        printf("%d", a[i]);
    }
    printf("]\n");
}

int main(void) {
    // Test 1
    {
        int n = 5;
        struct Edge edges[] = {
            {0,1,10},{0,2,3},{1,2,1},{1,3,2},{2,1,4},{2,3,8},{2,4,2},{3,4,7},{4,3,9}
        };
        long long dist[5]; int parent[5];
        dijkstra(n, edges, (int)(sizeof(edges)/sizeof(edges[0])), 0, dist, parent);
        printf("Test 1 distances:\n"); print_ll_array(dist, n);
        printf("Test 1 parents:\n"); print_int_array(parent, n);
    }

    // Test 2 (zero-weight)
    {
        int n = 4;
        struct Edge edges[] = {
            {0,1,0},{0,2,5},{1,2,1},{1,3,4},{2,3,0}
        };
        long long dist[4]; int parent[4];
        dijkstra(n, edges, (int)(sizeof(edges)/sizeof(edges[0])), 0, dist, parent);
        printf("Test 2 distances:\n"); print_ll_array(dist, n);
        printf("Test 2 parents:\n"); print_int_array(parent, n);
    }

    // Test 3 (disconnected)
    {
        int n = 5;
        struct Edge edges[] = {
            {0,1,2},{1,2,2}
        };
        long long dist[5]; int parent[5];
        dijkstra(n, edges, (int)(sizeof(edges)/sizeof(edges[0])), 0, dist, parent);
        printf("Test 3 distances:\n"); print_ll_array(dist, n);
        printf("Test 3 parents:\n"); print_int_array(parent, n);
    }

    // Test 4 (undirected classic: both directions)
    {
        int n = 6;
        struct Edge edges[] = {
            {0,1,7},{1,0,7},{0,2,9},{2,0,9},{0,5,14},{5,0,14},
            {1,2,10},{2,1,10},{1,3,15},{3,1,15},{2,3,11},{3,2,11},
            {2,5,2},{5,2,2},{3,4,6},{4,3,6},{4,5,9},{5,4,9}
        };
        long long dist[6]; int parent[6];
        dijkstra(n, edges, (int)(sizeof(edges)/sizeof(edges[0])), 0, dist, parent);
        printf("Test 4 distances:\n"); print_ll_array(dist, n);
        printf("Test 4 parents:\n"); print_int_array(parent, n);
    }

    // Test 5 (directed asymmetric)
    {
        int n = 5;
        struct Edge edges[] = {
            {0,1,2},{1,2,3},{0,2,10},{2,3,1},{3,4,1},{1,4,100}
        };
        long long dist[5]; int parent[5];
        dijkstra(n, edges, (int)(sizeof(edges)/sizeof(edges[0])), 0, dist, parent);
        printf("Test 5 distances:\n"); print_ll_array(dist, n);
        printf("Test 5 parents:\n"); print_int_array(parent, n);
    }

    return 0;
}