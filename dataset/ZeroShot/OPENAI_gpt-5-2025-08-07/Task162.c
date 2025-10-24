#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define INF (LLONG_MAX/4)

typedef struct {
    long long dist;
    int node;
} HeapItem;

typedef struct {
    HeapItem* data;
    int size;
    int capacity;
} MinHeap;

static MinHeap* heap_create(int capacity) {
    MinHeap* h = (MinHeap*)calloc(1, sizeof(MinHeap));
    if (!h) return NULL;
    h->data = (HeapItem*)malloc(sizeof(HeapItem) * (capacity > 1 ? capacity : 2));
    if (!h->data) { free(h); return NULL; }
    h->size = 0;
    h->capacity = capacity > 1 ? capacity : 2;
    return h;
}

static void heap_free(MinHeap* h) {
    if (h) {
        free(h->data);
        free(h);
    }
}

static void heap_swap(HeapItem* a, HeapItem* b) {
    HeapItem t = *a; *a = *b; *b = t;
}

static int heap_push(MinHeap* h, HeapItem item) {
    if (h->size >= h->capacity) {
        int newcap = h->capacity * 2;
        HeapItem* nd = (HeapItem*)realloc(h->data, sizeof(HeapItem) * newcap);
        if (!nd) return 0;
        h->data = nd;
        h->capacity = newcap;
    }
    int i = h->size++;
    h->data[i] = item;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (h->data[p].dist <= h->data[i].dist) break;
        heap_swap(&h->data[p], &h->data[i]);
        i = p;
    }
    return 1;
}

static int heap_pop(MinHeap* h, HeapItem* out) {
    if (h->size == 0) return 0;
    *out = h->data[0];
    h->data[0] = h->data[--h->size];
    int i = 0;
    while (1) {
        int l = 2*i + 1, r = 2*i + 2, smallest = i;
        if (l < h->size && h->data[l].dist < h->data[smallest].dist) smallest = l;
        if (r < h->size && h->data[r].dist < h->data[smallest].dist) smallest = r;
        if (smallest == i) break;
        heap_swap(&h->data[i], &h->data[smallest]);
        i = smallest;
    }
    return 1;
}

// Returns distances array (size n) on success; caller must free.
// On invalid input, returns NULL.
long long* dijkstra(int n, int m, const int from[], const int to[], const long long w[], int start) {
    if (n <= 0 || m < 0 || !from || !to || !w) return NULL;
    if (start < 0 || start >= n) return NULL;

    // Validate edges
    for (int i = 0; i < m; ++i) {
        if (from[i] < 0 || from[i] >= n) return NULL;
        if (to[i] < 0 || to[i] >= n) return NULL;
        if (w[i] < 0) return NULL; // Dijkstra requires non-negative
    }

    // Build adjacency via CSR-like (head/next)
    int* head = (int*)malloc(sizeof(int) * n);
    int* next = (int*)malloc(sizeof(int) * m);
    int* eto  = (int*)malloc(sizeof(int) * m);
    long long* ew = (long long*)malloc(sizeof(long long) * m);
    if (!head || !next || !eto || !ew) {
        free(head); free(next); free(eto); free(ew);
        return NULL;
    }
    for (int i = 0; i < n; ++i) head[i] = -1;
    for (int i = 0; i < m; ++i) {
        eto[i] = to[i];
        ew[i] = w[i];
        next[i] = head[from[i]];
        head[from[i]] = i;
    }

    long long* dist = (long long*)malloc(sizeof(long long) * n);
    char* visited = (char*)calloc(n, sizeof(char));
    if (!dist || !visited) {
        free(head); free(next); free(eto); free(ew);
        free(dist); free(visited);
        return NULL;
    }
    for (int i = 0; i < n; ++i) dist[i] = INF;
    dist[start] = 0;

    MinHeap* pq = heap_create(m + 4);
    if (!pq) {
        free(head); free(next); free(eto); free(ew);
        free(dist); free(visited);
        return NULL;
    }
    heap_push(pq, (HeapItem){0LL, start});

    HeapItem cur;
    while (heap_pop(pq, &cur)) {
        int u = cur.node;
        if (visited[u]) continue;
        visited[u] = 1;

        for (int ei = head[u]; ei != -1; ei = next[ei]) {
            int v = eto[ei];
            long long wgt = ew[ei];
            if (visited[v]) continue;
            if (cur.dist > INF - wgt) continue; // prevent overflow
            long long nd = cur.dist + wgt;
            if (nd < dist[v]) {
                dist[v] = nd;
                heap_push(pq, (HeapItem){nd, v});
            }
        }
    }

    heap_free(pq);
    free(head); free(next); free(eto); free(ew);
    free(visited);
    return dist;
}

static void print_result(long long* dist, int n) {
    for (int i = 0; i < n; ++i) {
        if (i) printf(" ");
        if (dist[i] >= INF) printf("INF");
        else printf("%lld", dist[i]);
    }
    printf("\n");
}

int main(void) {
    // Test case 1
    {
        int n = 5, s = 0;
        int m = 6;
        int from[] = {0,0,1,1,2,3};
        int to[]   = {1,2,2,3,3,4};
        long long w[] = {2,5,1,2,1,3};
        long long* dist = dijkstra(n, m, from, to, w, s);
        if (!dist) return 1;
        print_result(dist, n); // 0 2 3 4 7
        free(dist);
    }
    // Test case 2
    {
        int n = 5, s = 0;
        int m = 4;
        int from[] = {0,1,2,3};
        int to[]   = {1,2,3,4};
        long long w[] = {1,1,1,1};
        long long* dist = dijkstra(n, m, from, to, w, s);
        if (!dist) return 1;
        print_result(dist, n); // 0 1 2 3 4
        free(dist);
    }
    // Test case 3
    {
        int n = 5, s = 0;
        int m = 7;
        int from[] = {0,0,2,2,1,3,4};
        int to[]   = {1,2,1,3,3,4,1};
        long long w[] = {10,3,1,2,4,2,1};
        long long* dist = dijkstra(n, m, from, to, w, s);
        if (!dist) return 1;
        print_result(dist, n); // 0 4 3 5 7
        free(dist);
    }
    // Test case 4 (disconnected)
    {
        int n = 5, s = 0;
        int m = 2;
        int from[] = {1,2};
        int to[]   = {2,3};
        long long w[] = {2,2};
        long long* dist = dijkstra(n, m, from, to, w, s);
        if (!dist) return 1;
        print_result(dist, n); // 0 INF INF INF INF
        free(dist);
    }
    // Test case 5 (classic)
    {
        int n = 6, s = 0;
        int m = 9;
        int from[] = {0,0,0,1,1,2,2,3,4};
        int to[]   = {1,2,5,2,3,3,5,4,5};
        long long w[] = {7,9,14,10,15,11,2,6,9};
        long long* dist = dijkstra(n, m, from, to, w, s);
        if (!dist) return 1;
        print_result(dist, n); // 0 7 9 20 26 11
        free(dist);
    }
    return 0;
}