#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 Performs DFS on a graph with n nodes.
 graph: adjacency lists, where graph[i] is an array of neighbor indices for node i
 sizes: sizes[i] is the number of neighbors in graph[i]
 start: starting node
 outLen: output length of traversal
 Returns a newly allocated array of traversal order (caller must free), or NULL if invalid.
*/
int* dfs(int n, int** graph, const int* sizes, int start, int* outLen) {
    if (outLen == NULL) return NULL;
    *outLen = 0;
    if (n <= 0 || graph == NULL || sizes == NULL || start < 0 || start >= n) {
        return NULL;
    }

    // Sanitize adjacency: bounds-check, deduplicate, sort ascending
    int** sgraph = (int**)calloc((size_t)n, sizeof(int*));
    int* ssizes = (int*)calloc((size_t)n, sizeof(int));
    if (!sgraph || !ssizes) {
        free(sgraph);
        free(ssizes);
        return NULL;
    }

    for (int i = 0; i < n; ++i) {
        unsigned char* seen = (unsigned char*)calloc((size_t)n, sizeof(unsigned char));
        if (!seen) {
            for (int k = 0; k < i; ++k) free(sgraph[k]);
            free(sgraph);
            free(ssizes);
            return NULL;
        }
        int count = 0;
        if (graph[i] != NULL && sizes[i] > 0) {
            for (int j = 0; j < sizes[i]; ++j) {
                int v = graph[i][j];
                if (v >= 0 && v < n) {
                    if (!seen[v]) {
                        seen[v] = 1;
                        // count later to maintain ascending order
                    }
                }
            }
        }
        for (int v = 0; v < n; ++v) {
            if (seen[v]) count++;
        }
        ssizes[i] = count;
        if (count > 0) {
            sgraph[i] = (int*)malloc((size_t)count * sizeof(int));
            if (!sgraph[i]) {
                free(seen);
                for (int k = 0; k < i; ++k) free(sgraph[k]);
                free(sgraph);
                free(ssizes);
                return NULL;
            }
            int idx = 0;
            for (int v = 0; v < n; ++v) {
                if (seen[v]) {
                    sgraph[i][idx++] = v; // ascending order
                }
            }
        } else {
            sgraph[i] = NULL;
        }
        free(seen);
    }

    int* traversal = (int*)malloc((size_t)n * sizeof(int));
    unsigned char* visited = (unsigned char*)calloc((size_t)n, sizeof(unsigned char));
    int* stack = (int*)malloc((size_t)n * sizeof(int));
    if (!traversal || !visited || !stack) {
        free(traversal);
        free(visited);
        free(stack);
        for (int i = 0; i < n; ++i) free(sgraph[i]);
        free(sgraph);
        free(ssizes);
        return NULL;
    }

    int tlen = 0;
    int top = -1;
    stack[++top] = start;

    while (top >= 0) {
        int v = stack[top--];
        if (v < 0 || v >= n) {
            continue;
        }
        if (!visited[v]) {
            visited[v] = 1;
            traversal[tlen++] = v;
            // push neighbors in reverse order to visit ascending
            int sz = ssizes[v];
            int* nbrs = sgraph[v];
            for (int i = sz - 1; i >= 0; --i) {
                int nb = nbrs[i];
                if (!visited[nb]) {
                    if (top + 1 >= n) {
                        // stack overflow protection: enlarge
                        int newCap = n * 2;
                        int* newStack = (int*)realloc(stack, (size_t)newCap * sizeof(int));
                        if (newStack) {
                            stack = newStack;
                            // Update n to new capacity to avoid future overflow
                            n = newCap;
                        } else {
                            // On realloc failure, clean up and abort traversal safely
                            break;
                        }
                    }
                    stack[++top] = nb;
                }
            }
        }
    }

    // Clean up sanitized graph
    for (int i = 0; i < n; ++i) free(sgraph[i]);
    free(sgraph);
    free(ssizes);
    free(visited);
    free(stack);

    *outLen = tlen;
    // shrink traversal to exact size
    int* out = (int*)malloc((size_t)tlen * sizeof(int));
    if (!out) {
        free(traversal);
        *outLen = 0;
        return NULL;
    }
    memcpy(out, traversal, (size_t)tlen * sizeof(int));
    free(traversal);
    return out;
}

static void print_arr(const int* arr, int len) {
    printf("[");
    for (int i = 0; i < len; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    // Test 1: Simple chain 0-1-2
    {
        int n = 3;
        int a0[] = {1};
        int a1[] = {0, 2};
        int a2[] = {1};
        int* g[] = {a0, a1, a2};
        int sizes[] = {1, 2, 1};
        int outLen = 0;
        int* res = dfs(n, g, sizes, 0, &outLen);
        print_arr(res ? res : (int[]){}, outLen);
        free(res);
    }

    // Test 2: Cycle 0-1-2-3-0
    {
        int n = 4;
        int a0[] = {1, 3};
        int a1[] = {0, 2};
        int a2[] = {1, 3};
        int a3[] = {2, 0};
        int* g[] = {a0, a1, a2, a3};
        int sizes[] = {2, 2, 2, 2};
        int outLen = 0;
        int* res = dfs(n, g, sizes, 1, &outLen);
        print_arr(res ? res : (int[]){}, outLen);
        free(res);
    }

    // Test 3: Disconnected graph
    {
        int n = 5;
        int a0[] = {};
        int a1[] = {2};
        int a2[] = {1};
        int a3[] = {4};
        int a4[] = {3};
        int* g[] = {a0, a1, a2, a3, a4};
        int sizes[] = {0, 1, 1, 1, 1};
        int outLen = 0;
        int* res = dfs(n, g, sizes, 3, &outLen);
        print_arr(res ? res : (int[]){}, outLen);
        free(res);
    }

    // Test 4: Self-loop and out-of-range neighbor
    {
        int n = 4;
        int a0[] = {0, 1};     // self-loop at 0
        int a1[] = {0, 2, 4};  // 4 out-of-range
        int a2[] = {1, 3};
        int a3[] = {2};
        int* g[] = {a0, a1, a2, a3};
        int sizes[] = {2, 3, 2, 1};
        int outLen = 0;
        int* res = dfs(n, g, sizes, 0, &outLen);
        print_arr(res ? res : (int[]){}, outLen);
        free(res);
    }

    // Test 5: Star graph centered at 0
    {
        int n = 6;
        int a0[] = {1, 2, 3, 4, 5};
        int a1[] = {0};
        int a2[] = {0};
        int a3[] = {0};
        int a4[] = {0};
        int a5[] = {0};
        int* g[] = {a0, a1, a2, a3, a4, a5};
        int sizes[] = {5, 1, 1, 1, 1, 1};
        int outLen = 0;
        int* res = dfs(n, g, sizes, 2, &outLen);
        print_arr(res ? res : (int[]){}, outLen);
        free(res);
    }

    return 0;
}