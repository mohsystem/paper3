#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

long long* dijkstra(int n, const int (*edges)[3], int m, int start) {
    const long long INF = LLONG_MAX / 4;
    long long *adj = (long long*)malloc((size_t)n * (size_t)n * sizeof(long long));
    if (!adj) return NULL;
    for (int i = 0; i < n*n; ++i) adj[i] = INF;
    for (int i = 0; i < n; ++i) adj[i*(size_t)n + i] = 0;
    for (int i = 0; i < m; ++i) {
        int u = edges[i][0], v = edges[i][1], w = edges[i][2];
        if (u >= 0 && u < n && v >= 0 && v < n) {
            long long cur = adj[u*(size_t)n + v];
            if ((long long)w < cur) adj[u*(size_t)n + v] = (long long)w;
        }
    }

    long long *dist = (long long*)malloc((size_t)n * sizeof(long long));
    char *vis = (char*)calloc((size_t)n, sizeof(char));
    if (!dist || !vis) {
        free(adj);
        free(dist);
        free(vis);
        return NULL;
    }
    for (int i = 0; i < n; ++i) dist[i] = INF;
    dist[start] = 0;

    for (int i = 0; i < n; ++i) {
        int u = -1;
        long long best = INF;
        for (int j = 0; j < n; ++j) {
            if (!vis[j] && dist[j] < best) {
                best = dist[j];
                u = j;
            }
        }
        if (u == -1) break;
        vis[u] = 1;
        for (int v = 0; v < n; ++v) {
            long long w = adj[u*(size_t)n + v];
            if (!vis[v] && w < INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
            }
        }
    }

    free(adj);
    free(vis);
    return dist;
}

static void printDistances(long long *dist, int n) {
    for (int i = 0; i < n; ++i) {
        if (i) printf(" ");
        if (dist[i] >= LLONG_MAX / 8) printf("INF");
        else printf("%lld", dist[i]);
    }
    printf("\n");
}

int main(void) {
    // Test 1: Directed graph
    int n1 = 5;
    int edges1[][3] = {
        {0,1,10},{0,2,3},{1,2,1},{2,1,4},{1,3,2},{2,3,2},{3,4,7},{2,4,8},{4,3,9}
    };
    long long *d1 = dijkstra(n1, edges1, (int)(sizeof(edges1)/sizeof(edges1[0])), 0);
    printDistances(d1, n1); // Expected: 0 7 3 5 11
    free(d1);

    // Test 2: Undirected graph
    int n2 = 4;
    int edges2[][3] = {
        {0,1,5},{1,0,5},{1,2,6},{2,1,6},{0,3,10},{3,0,10},{2,3,2},{3,2,2}
    };
    long long *d2 = dijkstra(n2, edges2, (int)(sizeof(edges2)/sizeof(edges2[0])), 0);
    printDistances(d2, n2); // Expected: 0 5 11 10
    free(d2);

    // Test 3: Unreachable nodes
    int n3 = 4;
    int edges3[][3] = {
        {0,1,2}
    };
    long long *d3 = dijkstra(n3, edges3, (int)(sizeof(edges3)/sizeof(edges3[0])), 0);
    printDistances(d3, n3); // Expected: 0 2 INF INF
    free(d3);

    // Test 4: Zero-weight edges
    int n4 = 5;
    int edges4[][3] = {
        {0,1,0},{1,2,0},{2,3,0},{3,4,0}
    };
    long long *d4 = dijkstra(n4, edges4, (int)(sizeof(edges4)/sizeof(edges4[0])), 0);
    printDistances(d4, n4); // Expected: 0 0 0 0 0
    free(d4);

    // Test 5: Undirected classic
    int n5 = 6;
    int edges5[][3] = {
        {0,1,7},{1,0,7},{0,2,9},{2,0,9},{0,5,14},{5,0,14},
        {1,2,10},{2,1,10},{1,3,15},{3,1,15},{2,3,11},{3,2,11},
        {2,5,2},{5,2,2},{3,4,6},{4,3,6},{4,5,9},{5,4,9}
    };
    long long *d5 = dijkstra(n5, edges5, (int)(sizeof(edges5)/sizeof(edges5[0])), 0);
    printDistances(d5, n5); // Expected: 0 7 9 20 20 11
    free(d5);

    return 0;
}