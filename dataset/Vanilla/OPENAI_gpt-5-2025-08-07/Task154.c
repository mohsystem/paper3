#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* data;
    int size;
} IntArray;

typedef struct {
    int* a;
    int size;
    int cap;
} Vec;

static void vec_init(Vec* v) {
    v->a = NULL; v->size = 0; v->cap = 0;
}

static void vec_push(Vec* v, int x) {
    if (v->size == v->cap) {
        int ncap = v->cap ? v->cap * 2 : 4;
        int* na = (int*)realloc(v->a, ncap * sizeof(int));
        if (!na) exit(1);
        v->a = na; v->cap = ncap;
    }
    v->a[v->size++] = x;
}

static int int_cmp(const void* a, const void* b) {
    int x = *(const int*)a, y = *(const int*)b;
    return (x > y) - (x < y);
}

static void dfs_rec(int u, Vec* adj, int n, char* vis, IntArray* order) {
    vis[u] = 1;
    order->data[order->size++] = u;
    Vec* nbrs = &adj[u];
    for (int i = 0; i < nbrs->size; ++i) {
        int v = nbrs->a[i];
        if (!vis[v]) dfs_rec(v, adj, n, vis, order);
    }
}

// edges: m x 2 array of (u,v)
IntArray dfs(int n, const int edges[][2], int m, int start, int directed) {
    Vec* adj = (Vec*)malloc(n * sizeof(Vec));
    for (int i = 0; i < n; ++i) vec_init(&adj[i]);
    for (int i = 0; i < m; ++i) {
        int u = edges[i][0], v = edges[i][1];
        if (u >= 0 && u < n && v >= 0 && v < n) {
            vec_push(&adj[u], v);
            if (!directed) vec_push(&adj[v], u);
        }
    }
    for (int i = 0; i < n; ++i) {
        if (adj[i].size > 1) qsort(adj[i].a, adj[i].size, sizeof(int), int_cmp);
    }

    IntArray order;
    order.size = 0;
    order.data = (int*)malloc((n > 0 ? n : 1) * sizeof(int));
    if (!(start >= 0 && start < n)) {
        // cleanup
        for (int i = 0; i < n; ++i) free(adj[i].a);
        free(adj);
        return order;
    }

    char* vis = (char*)calloc(n, sizeof(char));
    dfs_rec(start, adj, n, vis, &order);

    free(vis);
    for (int i = 0; i < n; ++i) free(adj[i].a);
    free(adj);
    return order;
}

static void print_array(const char* label, const IntArray* arr) {
    printf("%s: [", label);
    for (int i = 0; i < arr->size; ++i) {
        if (i) printf(", ");
        printf("%d", arr->data[i]);
    }
    printf("]\n");
}

int main() {
    // Test 1: Line graph undirected
    int n1 = 5;
    int e1[][2] = {{0,1},{1,2},{2,3},{3,4}};
    IntArray r1 = dfs(n1, e1, 4, 0, 0);
    print_array("Test1", &r1);
    free(r1.data);

    // Test 2: Branching undirected
    int n2 = 6;
    int e2[][2] = {{0,1},{0,2},{1,3},{1,4},{2,5}};
    IntArray r2 = dfs(n2, e2, 5, 0, 0);
    print_array("Test2", &r2);
    free(r2.data);

    // Test 3: Cycle undirected
    int n3 = 4;
    int e3[][2] = {{0,1},{1,2},{2,0},{2,3}};
    IntArray r3 = dfs(n3, e3, 4, 0, 0);
    print_array("Test3", &r3);
    free(r3.data);

    // Test 4: Disconnected, start isolated
    int n4 = 5;
    int e4[][2] = {{0,1},{1,2}};
    IntArray r4 = dfs(n4, e4, 2, 3, 0);
    print_array("Test4", &r4);
    free(r4.data);

    // Test 5: Directed graph
    int n5 = 5;
    int e5[][2] = {{0,1},{0,2},{1,3},{3,4}};
    IntArray r5 = dfs(n5, e5, 4, 0, 1);
    print_array("Test5", &r5);
    free(r5.data);

    return 0;
}