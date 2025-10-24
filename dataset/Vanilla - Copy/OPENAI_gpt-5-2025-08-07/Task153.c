#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int* order;
    int size;
} BFSResult;

static int cmp_int(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

BFSResult Task153_bfs(int n, int edges[][2], int m, int start) {
    BFSResult res;
    res.order = NULL;
    res.size = 0;
    if (n <= 0 || start < 0 || start >= n) return res;

    int* deg = (int*)calloc(n, sizeof(int));
    if (!deg) return res;

    // Count degrees (undirected)
    for (int i = 0; i < m; ++i) {
        int u = edges[i][0], v = edges[i][1];
        if (0 <= u && u < n && 0 <= v && v < n) {
            deg[u]++; deg[v]++;
        }
    }

    int** adj = (int**)malloc(n * sizeof(int*));
    int* idx = (int*)calloc(n, sizeof(int));
    if (!adj || !idx) {
        free(deg);
        if (adj) free(adj);
        if (idx) free(idx);
        return res;
    }
    for (int i = 0; i < n; ++i) {
        adj[i] = (int*)malloc((deg[i] > 0 ? deg[i] : 1) * sizeof(int));
        if (!adj[i]) {
            for (int j = 0; j < i; ++j) free(adj[j]);
            free(adj);
            free(deg);
            free(idx);
            return res;
        }
    }

    for (int i = 0; i < m; ++i) {
        int u = edges[i][0], v = edges[i][1];
        if (0 <= u && u < n && 0 <= v && v < n) {
            adj[u][idx[u]++] = v;
            adj[v][idx[v]++] = u;
        }
    }

    for (int i = 0; i < n; ++i) {
        if (deg[i] > 0) qsort(adj[i], deg[i], sizeof(int), cmp_int);
    }

    char* vis = (char*)calloc(n, sizeof(char));
    int* q = (int*)malloc(n * sizeof(int));
    int* order = (int*)malloc(n * sizeof(int));
    if (!vis || !q || !order) {
        for (int i = 0; i < n; ++i) free(adj[i]);
        free(adj); free(deg); free(idx);
        if (vis) free(vis);
        if (q) free(q);
        if (order) free(order);
        return res;
    }

    int head = 0, tail = 0, osz = 0;
    vis[start] = 1;
    q[tail++] = start;

    while (head < tail) {
        int u = q[head++];
        order[osz++] = u;
        for (int k = 0; k < deg[u]; ++k) {
            int v = adj[u][k];
            if (!vis[v]) {
                vis[v] = 1;
                q[tail++] = v;
            }
        }
    }

    for (int i = 0; i < n; ++i) free(adj[i]);
    free(adj); free(deg); free(idx);
    free(vis);
    free(q);

    res.order = order;
    res.size = osz;
    return res;
}

static void print_result(BFSResult r) {
    for (int i = 0; i < r.size; ++i) {
        if (i) printf(" ");
        printf("%d", r.order[i]);
    }
    printf("\n");
}

int main(void) {
    int e1[][2] = {{0,1},{1,2},{2,3}};
    BFSResult r1 = Task153_bfs(4, e1, 3, 0);
    print_result(r1);
    free(r1.order);

    int e2[][2] = {{3,4},{0,1}};
    BFSResult r2 = Task153_bfs(5, e2, 2, 3);
    print_result(r2);
    free(r2.order);

    int e3[][2] = {{0,1},{1,2},{2,0},{2,3}};
    BFSResult r3 = Task153_bfs(4, e3, 4, 1);
    print_result(r3);
    free(r3.order);

    int e4[][2] = {{0,1},{1,2},{2,3},{3,0},{0,2},{4,5}};
    BFSResult r4 = Task153_bfs(6, e4, 6, 2);
    print_result(r4);
    free(r4.order);

    int e5[1][2]; // placeholder, m=0
    BFSResult r5 = Task153_bfs(1, e5, 0, 0);
    print_result(r5);
    free(r5.order);

    return 0;
}