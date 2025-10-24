/*
 Chain-of-Through process:
 1) Problem: Implement DFS traversals (preorder, inorder, postorder) on a binary tree.
 2) Security: Validate traversal order, avoid recursion depth by iterative approach, safe memory handling.
 3) Secure coding: Bounds checks, robust parsing, error-aware allocations.
 4) Review: Ensure no leaks, handle null/empty inputs, avoid undefined behavior.
 5) Output: Final code with iterative traversals and safe tree construction/destruction.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

typedef struct TreeNode {
    int val;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

typedef struct {
    int* data;
    size_t size;
    size_t cap;
} IntArray;

static void ia_init(IntArray* a) {
    a->data = NULL;
    a->size = 0;
    a->cap = 0;
}
static int ia_reserve(IntArray* a, size_t needed) {
    if (a->cap >= needed) return 1;
    size_t newcap = a->cap ? a->cap : 8;
    while (newcap < needed) {
        if (newcap > SIZE_MAX / 2) return 0;
        newcap *= 2;
    }
    int* nd = (int*)realloc(a->data, newcap * sizeof(int));
    if (!nd) return 0;
    a->data = nd;
    a->cap = newcap;
    return 1;
}
static int ia_push(IntArray* a, int v) {
    if (!ia_reserve(a, a->size + 1)) return 0;
    a->data[a->size++] = v;
    return 1;
}
static void ia_free(IntArray* a) {
    free(a->data);
    a->data = NULL;
    a->size = a->cap = 0;
}

typedef struct {
    TreeNode** data;
    size_t size;
    size_t cap;
} NodeStack;

static void ns_init(NodeStack* s) {
    s->data = NULL; s->size = 0; s->cap = 0;
}
static int ns_reserve(NodeStack* s, size_t needed) {
    if (s->cap >= needed) return 1;
    size_t newcap = s->cap ? s->cap : 8;
    while (newcap < needed) {
        if (newcap > SIZE_MAX / 2) return 0;
        newcap *= 2;
    }
    TreeNode** nd = (TreeNode**)realloc(s->data, newcap * sizeof(TreeNode*));
    if (!nd) return 0;
    s->data = nd; s->cap = newcap; return 1;
}
static int ns_push(NodeStack* s, TreeNode* n) {
    if (!ns_reserve(s, s->size + 1)) return 0;
    s->data[s->size++] = n; return 1;
}
static TreeNode* ns_pop(NodeStack* s) {
    if (s->size == 0) return NULL;
    return s->data[--s->size];
}
static TreeNode* ns_peek(NodeStack* s) {
    if (s->size == 0) return NULL;
    return s->data[s->size - 1];
}
static void ns_free(NodeStack* s) {
    free(s->data); s->data = NULL; s->size = s->cap = 0;
}

static int to_order_code(const char* order) {
    if (!order) return 0; // default preorder
    size_t n = strlen(order);
    char* tmp = (char*)malloc(n + 1);
    if (!tmp) return 0;
    for (size_t i = 0; i < n; ++i) tmp[i] = (char)tolower((unsigned char)order[i]);
    tmp[n] = '\0';
    int code = 0;
    if (strcmp(tmp, "inorder") == 0) code = 1;
    else if (strcmp(tmp, "postorder") == 0) code = 2;
    else code = 0; // preorder default
    free(tmp);
    return code;
}

static int safe_parse_int(const char* s, int* out) {
    if (!s || !out) return 0;
    errno = 0;
    char* end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return 0;
    if (v < INT_MIN || v > INT_MAX) return 0;
    *out = (int)v;
    return 1;
}

// Build tree from level-order array; "null" string denotes missing node.
TreeNode* build_tree_from_level_order(const char* values[], size_t n) {
    if (n == 0 || !values || strcmp(values[0], "null") == 0) return NULL;
    int root_val;
    if (!safe_parse_int(values[0], &root_val)) return NULL;
    TreeNode* root = (TreeNode*)calloc(1, sizeof(TreeNode));
    if (!root) return NULL;
    root->val = root_val;

    TreeNode** q = (TreeNode**)calloc(n > 1 ? n : 1, sizeof(TreeNode*));
    if (!q) { free(root); return NULL; }
    size_t head = 0, tail = 0;
    q[tail++] = root;

    size_t i = 1;
    while (head < tail && i < n) {
        TreeNode* cur = q[head++];
        if (cur) {
            // left child
            if (i < n && strcmp(values[i], "null") != 0) {
                int lv;
                if (!safe_parse_int(values[i], &lv)) { /* skip invalid */ }
                else {
                    cur->left = (TreeNode*)calloc(1, sizeof(TreeNode));
                    if (cur->left) {
                        cur->left->val = lv;
                        q[tail++] = cur->left;
                    }
                }
            }
            i++;
            // right child
            if (i < n && strcmp(values[i], "null") != 0) {
                int rv;
                if (!safe_parse_int(values[i], &rv)) { /* skip invalid */ }
                else {
                    cur->right = (TreeNode*)calloc(1, sizeof(TreeNode));
                    if (cur->right) {
                        cur->right->val = rv;
                        q[tail++] = cur->right;
                    }
                }
            }
            i++;
        }
    }
    free(q);
    return root;
}

// DFS traversal, iterative. order: "preorder", "inorder", "postorder"
IntArray dfs(TreeNode* root, const char* order) {
    IntArray res; ia_init(&res);
    int ord = to_order_code(order); // 0=pre, 1=in, 2=post
    if (!root) return res;

    if (ord == 0) { // preorder
        NodeStack st; ns_init(&st);
        ns_push(&st, root);
        while (st.size) {
            TreeNode* n = ns_pop(&st);
            ia_push(&res, n->val);
            if (n->right) ns_push(&st, n->right);
            if (n->left) ns_push(&st, n->left);
        }
        ns_free(&st);
    } else if (ord == 1) { // inorder
        NodeStack st; ns_init(&st);
        TreeNode* cur = root;
        while (cur || st.size) {
            while (cur) { ns_push(&st, cur); cur = cur->left; }
            TreeNode* n = ns_pop(&st);
            ia_push(&res, n->val);
            cur = n->right;
        }
        ns_free(&st);
    } else { // postorder
        NodeStack st; ns_init(&st);
        TreeNode* cur = root;
        TreeNode* last = NULL;
        while (cur || st.size) {
            if (cur) {
                ns_push(&st, cur);
                cur = cur->left;
            } else {
                TreeNode* peek = ns_peek(&st);
                if (peek->right && last != peek->right) {
                    cur = peek->right;
                } else {
                    ia_push(&res, peek->val);
                    last = ns_pop(&st);
                }
            }
        }
        ns_free(&st);
    }
    return res;
}

static void print_int_array(const IntArray* a) {
    printf("[");
    for (size_t i = 0; i < a->size; ++i) {
        if (i) printf(", ");
        printf("%d", a->data[i]);
    }
    printf("]");
}

static void free_tree(TreeNode* root) {
    if (!root) return;
    NodeStack st; ns_init(&st);
    TreeNode* cur = root;
    TreeNode* last = NULL;
    while (cur || st.size) {
        if (cur) { ns_push(&st, cur); cur = cur->left; }
        else {
            TreeNode* peek = ns_peek(&st);
            if (peek->right && last != peek->right) {
                cur = peek->right;
            } else {
                last = ns_pop(&st);
                free(last);
            }
        }
    }
    ns_free(&st);
}

int main(void) {
    const char* t1[] = {"1","2","3","4","5","null","6"};
    const char* t2[] = {"1","null","2","3"};
    const char* t3[] = {};
    const char* t4[] = {"10","5","15","null","7","12","20"};
    const char* t5[] = {"42"};
    const struct { const char** arr; size_t n; } tests[5] = {
        {t1, 7}, {t2, 4}, {t3, 0}, {t4, 7}, {t5, 1}
    };
    const char* orders[] = {"preorder","inorder","postorder"};

    for (int i = 0; i < 5; ++i) {
        TreeNode* root = build_tree_from_level_order(tests[i].arr, tests[i].n);
        printf("Case %d:\n", i+1);
        for (int j = 0; j < 3; ++j) {
            IntArray r = dfs(root, orders[j]);
            printf(" %s: ", orders[j]);
            print_int_array(&r);
            printf("\n");
            ia_free(&r);
        }
        free_tree(root);
    }
    return 0;
}