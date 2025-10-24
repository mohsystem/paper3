#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Node {
    int val;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct {
    int* data;
    size_t size;
    size_t capacity;
} IntArray;

static void ia_init(IntArray* a) {
    a->data = NULL;
    a->size = 0;
    a->capacity = 0;
}

static void ia_free(IntArray* a) {
    free(a->data);
    a->data = NULL;
    a->size = 0;
    a->capacity = 0;
}

static int ia_grow(IntArray* a, size_t mincap) {
    size_t newcap = a->capacity ? a->capacity : 16;
    while (newcap < mincap) {
        if (newcap > (SIZE_MAX / 2)) {
            return 0; // overflow
        }
        newcap *= 2;
    }
    int* nd = (int*)realloc(a->data, newcap * sizeof(int));
    if (!nd) return 0;
    a->data = nd;
    a->capacity = newcap;
    return 1;
}

static int ia_append(IntArray* a, int v) {
    if (a->size == a->capacity) {
        if (!ia_grow(a, a->size + 1)) return 0;
    }
    a->data[a->size++] = v;
    return 1;
}

static int strieq(const char* a, const char* b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

Node* build_tree(const int* values, const int* present, size_t n) {
    if ((values == NULL && n > 0) || (present == NULL && n > 0)) {
        return NULL;
    }
    if (n == 0) return NULL;
    if (present[0] == 0) return NULL;

    Node** nodes = (Node**)calloc(n, sizeof(Node*));
    if (!nodes) return NULL;

    for (size_t i = 0; i < n; ++i) {
        if (present[i]) {
            nodes[i] = (Node*)malloc(sizeof(Node));
            if (!nodes[i]) {
                // cleanup
                for (size_t j = 0; j < n; ++j) {
                    if (nodes[j]) free(nodes[j]);
                }
                free(nodes);
                return NULL;
            }
            nodes[i]->val = values[i];
            nodes[i]->left = NULL;
            nodes[i]->right = NULL;
        }
    }
    for (size_t i = 0; i < n; ++i) {
        if (!nodes[i]) continue;
        size_t li = 2 * i + 1;
        size_t ri = 2 * i + 2;
        if (li < n && nodes[li]) nodes[i]->left = nodes[li];
        if (ri < n && nodes[ri]) nodes[i]->right = nodes[ri];
    }
    Node* root = nodes[0];
    free(nodes);
    return root;
}

static int preorder(Node* node, IntArray* out) {
    if (!node) return 1;
    if (!ia_append(out, node->val)) return 0;
    if (!preorder(node->left, out)) return 0;
    if (!preorder(node->right, out)) return 0;
    return 1;
}

static int inorder(Node* node, IntArray* out) {
    if (!node) return 1;
    if (!inorder(node->left, out)) return 0;
    if (!ia_append(out, node->val)) return 0;
    if (!inorder(node->right, out)) return 0;
    return 1;
}

static int postorder(Node* node, IntArray* out) {
    if (!node) return 1;
    if (!postorder(node->left, out)) return 0;
    if (!postorder(node->right, out)) return 0;
    if (!ia_append(out, node->val)) return 0;
    return 1;
}

IntArray traverse(Node* root, const char* order) {
    IntArray res;
    ia_init(&res);
    if (!order) {
        return res;
    }
    if (strieq(order, "preorder")) {
        if (!preorder(root, &res)) { ia_free(&res); ia_init(&res); }
    } else if (strieq(order, "inorder")) {
        if (!inorder(root, &res)) { ia_free(&res); ia_init(&res); }
    } else if (strieq(order, "postorder")) {
        if (!postorder(root, &res)) { ia_free(&res); ia_init(&res); }
    } else {
        // unsupported order; return empty
    }
    return res;
}

void free_tree(Node* node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

static void print_array(const IntArray* a) {
    printf("[");
    for (size_t i = 0; i < a->size; ++i) {
        if (i) printf(",");
        printf("%d", a->data[i]);
    }
    printf("]");
}

int main(void) {
    int v1[] = {1,2,3,4,5,6,7};
    int p1[] = {1,1,1,1,1,1,1};
    size_t n1 = sizeof(v1)/sizeof(v1[0]);

    int v2[] = {1,2,3,0,5,0,7};
    int p2[] = {1,1,1,0,1,0,1};
    size_t n2 = sizeof(v2)/sizeof(v2[0]);

    int v3[] = {42};
    int p3[] = {1};
    size_t n3 = sizeof(v3)/sizeof(v3[0]);

    int* v4 = NULL;
    int* p4 = NULL;
    size_t n4 = 0;

    int v5[] = {1,0,2,0,0,0,3};
    int p5[] = {1,0,1,0,0,0,1};
    size_t n5 = sizeof(v5)/sizeof(v5[0]);

    const char* orders[] = {"preorder","inorder","postorder"};
    const size_t orders_cnt = sizeof(orders)/sizeof(orders[0]);

    Node* roots[5];
    roots[0] = build_tree(v1, p1, n1);
    roots[1] = build_tree(v2, p2, n2);
    roots[2] = build_tree(v3, p3, n3);
    roots[3] = build_tree(v4, p4, n4);
    roots[4] = build_tree(v5, p5, n5);

    for (int t = 0; t < 5; ++t) {
        printf("Test #%d\n", t + 1);
        for (size_t oi = 0; oi < orders_cnt; ++oi) {
            IntArray res = traverse(roots[t], orders[oi]);
            printf("%s: ", orders[oi]);
            print_array(&res);
            printf("\n");
            ia_free(&res);
        }
        printf("\n");
    }

    for (int t = 0; t < 5; ++t) {
        free_tree(roots[t]);
    }
    return 0;
}