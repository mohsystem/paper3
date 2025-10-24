#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { int u, v; } Edge;
typedef struct { int parent, child; } Pair;
typedef struct { Pair* arr; int size; } Result;

Result reparent(int n, const Edge* edges, int m, int root) {
    // compute degrees
    int* deg = (int*)calloc(n, sizeof(int));
    for (int i = 0; i < m; ++i) {
        deg[edges[i].u]++;
        deg[edges[i].v]++;
    }
    // prefix sum to allocate adjacency in one pool
    int* start = (int*)malloc(n * sizeof(int));
    int total = 0;
    for (int i = 0; i < n; ++i) {
        start[i] = total;
        total += deg[i];
    }
    int* cur = (int*)calloc(n, sizeof(int));
    int* adj = (int*)malloc(total * sizeof(int));
    // fill adjacency
    for (int i = 0; i < m; ++i) {
        int u = edges[i].u, v = edges[i].v;
        adj[start[u] + cur[u]++] = v;
        adj[start[v] + cur[v]++] = u;
    }
    // BFS
    char* vis = (char*)calloc(n, sizeof(char));
    int* q = (int*)malloc(n * sizeof(int));
    int qh = 0, qt = 0;
    Result res;
    res.size = n > 0 ? n - 1 : 0;
    res.arr = res.size > 0 ? (Pair*)malloc(res.size * sizeof(Pair)) : NULL;
    int ridx = 0;

    vis[root] = 1;
    q[qt++] = root;
    while (qh < qt) {
        int u = q[qh++];
        int begin = start[u], end = start[u] + deg[u];
        for (int i = begin; i < end; ++i) {
            int v = adj[i];
            if (!vis[v]) {
                vis[v] = 1;
                if (ridx < res.size) {
                    res.arr[ridx].parent = u;
                    res.arr[ridx].child = v;
                    ridx++;
                }
                q[qt++] = v;
            }
        }
    }

    // cleanup
    free(deg); free(start); free(cur); free(adj); free(vis); free(q);
    return res;
}

void printDirectedEdges(const Result* r) {
    printf("[");
    for (int i = 0; i < r->size; ++i) {
        printf("(%d->%d)", r->arr[i].parent, r->arr[i].child);
        if (i + 1 < r->size) printf(", ");
    }
    printf("]\n");
}

int main() {
    // Test case 1
    int n1 = 10;
    Edge edges1[] = {
        {0,1},{0,2},{0,3},{1,4},{1,5},{2,6},{2,7},{3,8},{3,9}
    };
    printf("Test 1: root=6\n");
    Result r1 = reparent(n1, edges1, sizeof(edges1)/sizeof(edges1[0]), 6);
    printDirectedEdges(&r1);
    free(r1.arr);

    // Test case 2
    printf("Test 2: root=0\n");
    Result r2 = reparent(n1, edges1, sizeof(edges1)/sizeof(edges1[0]), 0);
    printDirectedEdges(&r2);
    free(r2.arr);

    // Test case 3
    int n3 = 5;
    Edge edges3[] = {{0,1},{1,2},{2,3},{3,4}};
    printf("Test 3: root=3\n");
    Result r3 = reparent(n3, edges3, sizeof(edges3)/sizeof(edges3[0]), 3);
    printDirectedEdges(&r3);
    free(r3.arr);

    // Test case 4
    int n4 = 6;
    Edge edges4[] = {{0,1},{0,2},{0,3},{0,4},{0,5}};
    printf("Test 4: root=4\n");
    Result r4 = reparent(n4, edges4, sizeof(edges4)/sizeof(edges4[0]), 4);
    printDirectedEdges(&r4);
    free(r4.arr);

    // Test case 5
    int n5 = 7;
    Edge edges5[] = {{0,1},{1,2},{1,3},{3,4},{2,5},{5,6}};
    printf("Test 5: root=4\n");
    Result r5 = reparent(n5, edges5, sizeof(edges5)/sizeof(edges5[0]), 4);
    printDirectedEdges(&r5);
    free(r5.arr);

    return 0;
}