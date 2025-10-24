#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define INF (LLONG_MAX / 4)

typedef struct {
    int v;
    long long w;
} Edge;

static long long* dijkstra(int n, const int (*edges)[3], int m, int start) {
    if (n <= 0 || start < 0 || start >= n) {
        return NULL;
    }
    if (m < 0) {
        return NULL;
    }

    int* deg = (int*)calloc((size_t)n, sizeof(int));
    if (!deg) return NULL;

    for (int i = 0; i < m; i++) {
        int u = edges[i][0];
        int v = edges[i][1];
        long long w = (long long)edges[i][2];
        if (u < 0 || u >= n || v < 0 || v >= n || w < 0 || w > INF / 2) {
            free(deg);
            return NULL;
        }
        deg[u]++;
    }

    Edge** adj = (Edge**)malloc((size_t)n * sizeof(Edge*));
    int* idx = (int*)calloc((size_t)n, sizeof(int));
    if (!adj || !idx) {
        free(deg);
        free(adj);
        free(idx);
        return NULL;
    }
    for (int i = 0; i < n; i++) {
        if (deg[i] > 0) {
            adj[i] = (Edge*)malloc((size_t)deg[i] * sizeof(Edge));
            if (!adj[i]) {
                for (int j = 0; j < i; j++) free(adj[j]);
                free(adj);
                free(deg);
                free(idx);
                return NULL;
            }
        } else {
            adj[i] = NULL;
        }
    }

    for (int i = 0; i < m; i++) {
        int u = edges[i][0];
        int v = edges[i][1];
        long long w = (long long)edges[i][2];
        int pos = idx[u]++;
        adj[u][pos].v = v;
        adj[u][pos].w = w;
    }

    long long* dist = (long long*)malloc((size_t)n * sizeof(long long));
    char* used = (char*)calloc((size_t)n, sizeof(char));
    if (!dist || !used) {
        free(dist);
        free(used);
        for (int i = 0; i < n; i++) free(adj[i]);
        free(adj);
        free(deg);
        free(idx);
        return NULL;
    }

    for (int i = 0; i < n; i++) dist[i] = INF;
    dist[start] = 0;

    for (int it = 0; it < n; it++) {
        int u = -1;
        long long best = INF;
        for (int i = 0; i < n; i++) {
            if (!used[i] && dist[i] < best) {
                best = dist[i];
                u = i;
            }
        }
        if (u == -1) break;
        used[u] = 1;

        for (int k = 0; k < deg[u]; k++) {
            int v = adj[u][k].v;
            long long w = adj[u][k].w;
            if (dist[u] <= INF - w) {
                long long alt = dist[u] + w;
                if (alt < dist[v]) {
                    dist[v] = alt;
                }
            }
        }
    }

    for (int i = 0; i < n; i++) free(adj[i]);
    free(adj);
    free(deg);
    free(idx);
    free(used);

    return dist;
}

static void print_result(const char* title, long long* dist, int n) {
    printf("%s\n", title);
    if (dist == NULL) {
        printf("Invalid input\n");
        return;
    }
    for (int i = 0; i < n; i++) {
        if (dist[i] >= INF) {
            printf("INF");
        } else {
            printf("%lld", dist[i]);
        }
        if (i + 1 != n) printf(" ");
    }
    printf("\n");
}

int main(void) {
    // Test 1: Simple directed graph
    {
        int n = 3;
        int edges[][3] = {
            {0, 1, 4},
            {0, 2, 1},
            {2, 1, 2}
        };
        long long* dist = dijkstra(n, edges, (int)(sizeof(edges) / sizeof(edges[0])), 0);
        print_result("Test 1", dist, n);
        free(dist);
    }

    // Test 2: Disconnected nodes
    {
        int n = 4;
        int edges[][3] = {
            {0, 1, 5}
        };
        long long* dist = dijkstra(n, edges, (int)(sizeof(edges) / sizeof(edges[0])), 0);
        print_result("Test 2", dist, n);
        free(dist);
    }

    // Test 3: Zero-weight edges
    {
        int n = 4;
        int edges[][3] = {
            {0, 1, 0},
            {1, 2, 0},
            {2, 3, 1}
        };
        long long* dist = dijkstra(n, edges, (int)(sizeof(edges) / sizeof(edges[0])), 0);
        print_result("Test 3", dist, n);
        free(dist);
    }

    // Test 4: Larger weights with alternative shorter path
    {
        int n = 5;
        int edges[][3] = {
            {0, 1, 1000000000},
            {1, 2, 1000000000},
            {0, 3, 1},
            {3, 4, 1},
            {4, 2, 1}
        };
        long long* dist = dijkstra(n, edges, (int)(sizeof(edges) / sizeof(edges[0])), 0);
        print_result("Test 4", dist, n);
        free(dist);
    }

    // Test 5: Invalid input (negative weight)
    {
        int n = 2;
        int edges[][3] = {
            {0, 1, -1}
        };
        long long* dist = dijkstra(n, edges, (int)(sizeof(edges) / sizeof(edges[0])), 0);
        print_result("Test 5", dist, n);
        free(dist);
    }

    return 0;
}