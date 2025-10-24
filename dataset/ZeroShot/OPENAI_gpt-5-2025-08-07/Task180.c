#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int u;
    int v;
} Edge;

typedef struct {
    int n;
    int* childCounts;
    int** children;
} OrientedTree;

static int cmp_edge(const void* a, const void* b) {
    const Edge* ea = (const Edge*)a;
    const Edge* eb = (const Edge*)b;
    if (ea->u != eb->u) return (ea->u < eb->u) ? -1 : 1;
    if (ea->v != eb->v) return (ea->v < eb->v) ? -1 : 1;
    return 0;
}

static void free_oriented_tree(OrientedTree* t) {
    if (!t) return;
    if (t->children) {
        for (int i = 0; i < t->n; i++) {
            free(t->children[i]);
        }
        free(t->children);
    }
    free(t->childCounts);
    t->n = 0;
    t->children = NULL;
    t->childCounts = NULL;
}

static void print_oriented_tree(const OrientedTree* t) {
    if (!t || t->n <= 0 || t->children == NULL || t->childCounts == NULL) {
        printf("Invalid or empty result\n");
        return;
    }
    for (int i = 0; i < t->n; i++) {
        printf("%d: [", i);
        for (int j = 0; j < t->childCounts[i]; j++) {
            if (j) printf(", ");
            printf("%d", t->children[i][j]);
        }
        printf("]\n");
    }
}

static void free_adj(int** adj, int n, int* deg) {
    if (adj) {
        for (int i = 0; i < n; i++) free(adj[i]);
        free(adj);
    }
    free(deg);
}

OrientedTree reparent_tree_c(int n, const Edge* edges, int m, int root) {
    OrientedTree invalid = {0, NULL, NULL};
    if (n < 1 || edges == NULL || m != n - 1 || root < 0 || root >= n) return invalid;

    // Validate edges: range, self-loops, duplicates
    Edge* norm = (Edge*)malloc(sizeof(Edge) * m);
    if (!norm) return invalid;

    for (int i = 0; i < m; i++) {
        int u = edges[i].u, v = edges[i].v;
        if (u < 0 || u >= n || v < 0 || v >= n) { free(norm); return invalid; }
        if (u == v) { free(norm); return invalid; }
        if (u < v) { norm[i].u = u; norm[i].v = v; }
        else { norm[i].u = v; norm[i].v = u; }
    }
    qsort(norm, m, sizeof(Edge), cmp_edge);
    for (int i = 1; i < m; i++) {
        if (norm[i].u == norm[i-1].u && norm[i].v == norm[i-1].v) { free(norm); return invalid; }
    }

    // Build undirected adjacency
    int* deg = (int*)calloc(n, sizeof(int));
    if (!deg) { free(norm); return invalid; }

    for (int i = 0; i < m; i++) {
        deg[edges[i].u]++;
        deg[edges[i].v]++;
    }

    int** adj = (int**)malloc(sizeof(int*) * n);
    int* fill = (int*)calloc(n, sizeof(int));
    if (!adj || !fill) {
        free(norm);
        free_adj(adj, 0, deg);
        free(fill);
        return invalid;
    }
    for (int i = 0; i < n; i++) {
        adj[i] = (int*)malloc(sizeof(int) * deg[i]);
        if (!adj[i]) {
            for (int k = 0; k < i; k++) free(adj[k]);
            free(adj);
            free(deg);
            free(fill);
            free(norm);
            return invalid;
        }
    }
    for (int i = 0; i < m; i++) {
        int u = edges[i].u, v = edges[i].v;
        adj[u][fill[u]++] = v;
        adj[v][fill[v]++] = u;
    }
    free(fill);
    free(norm);

    // BFS to orient away from root
    int* parent = (int*)malloc(sizeof(int) * n);
    int* queue = (int*)malloc(sizeof(int) * n);
    if (!parent || !queue) {
        free(parent);
        free(queue);
        free_adj(adj, n, deg);
        return invalid;
    }
    for (int i = 0; i < n; i++) parent[i] = -1;
    int head = 0, tail = 0;
    queue[tail++] = root;
    parent[root] = root;

    OrientedTree res;
    res.n = n;
    res.childCounts = (int*)calloc(n, sizeof(int));
    res.children = (int**)malloc(sizeof(int*) * n);
    if (!res.childCounts || !res.children) {
        free_oriented_tree(&res);
        free(parent);
        free(queue);
        free_adj(adj, n, deg);
        OrientedTree inval2 = {0, NULL, NULL};
        return inval2;
    }
    for (int i = 0; i < n; i++) {
        res.children[i] = (int*)malloc(sizeof(int) * deg[i]); // upper bound
        if (!res.children[i]) {
            // free partially allocated
            for (int k = 0; k < i; k++) free(res.children[k]);
            free(res.children);
            free(res.childCounts);
            free(parent);
            free(queue);
            free_adj(adj, n, deg);
            OrientedTree inval2 = {0, NULL, NULL};
            return inval2;
        }
    }

    int visited = 0;
    while (head < tail) {
        int u = queue[head++];
        visited++;
        for (int i = 0; i < deg[u]; i++) {
            int v = adj[u][i];
            if (v != parent[u]) {
                parent[v] = u;
                res.children[u][res.childCounts[u]++] = v;
                queue[tail++] = v;
            }
        }
    }

    free(parent);
    free(queue);
    free_adj(adj, n, deg);

    if (visited != n) {
        free_oriented_tree(&res);
        OrientedTree inval2 = {0, NULL, NULL};
        return inval2;
    }
    return res;
}

static void run_test(int n, Edge* edges, int m, int root, const char* label) {
    printf("%s\n", label);
    OrientedTree t = reparent_tree_c(n, edges, m, root);
    print_oriented_tree(&t);
    free_oriented_tree(&t);
}

int main(void) {
    // Test 1: Example tree reparented at 6
    int n1 = 10;
    Edge e1[] = {
        {0,1},{0,2},{0,3},{1,4},{1,5},{2,6},{2,7},{3,8},{3,9}
    };
    run_test(n1, e1, sizeof(e1)/sizeof(e1[0]), 6, "Test 1:");

    // Test 2: Chain 0-1-2-3 reparented at 3
    int n2 = 4;
    Edge e2[] = { {0,1},{1,2},{2,3} };
    run_test(n2, e2, sizeof(e2)/sizeof(e2[0]), 3, "Test 2:");

    // Test 3: Single node
    int n3 = 1;
    Edge e3[] = { };
    run_test(n3, e3, 0, 0, "Test 3:");

    // Test 4: Star centered at 0 reparented at 3
    int n4 = 5;
    Edge e4[] = { {0,1},{0,2},{0,3},{0,4} };
    run_test(n4, e4, sizeof(e4)/sizeof(e4[0]), 3, "Test 4:");

    // Test 5: Invalid (wrong number of edges)
    int n5 = 3;
    Edge e5[] = { {0,1} };
    run_test(n5, e5, sizeof(e5)/sizeof(e5[0]), 0, "Test 5:");

    return 0;
}