#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct TreeNode {
    int val;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

typedef struct {
    int* data;
    size_t size;
} IntArray;

typedef struct {
    TreeNode** data;
    size_t size;
    size_t cap;
} NodeStack;

static int safe_mul_size_t(size_t a, size_t b, size_t* out) {
    if (a == 0 || b == 0) { *out = 0; return 1; }
    if (a > SIZE_MAX / b) return 0;
    *out = a * b;
    return 1;
}

static void stack_init(NodeStack* st) {
    st->data = NULL;
    st->size = 0;
    st->cap = 0;
}

static void stack_free(NodeStack* st) {
    free(st->data);
    st->data = NULL;
    st->size = st->cap = 0;
}

static int stack_grow(NodeStack* st) {
    size_t newcap = st->cap ? st->cap * 2 : 16;
    if (newcap < st->cap) return 0; // overflow
    size_t bytes;
    if (!safe_mul_size_t(newcap, sizeof(TreeNode*), &bytes)) return 0;
    TreeNode** nd = (TreeNode**)realloc(st->data, bytes);
    if (!nd) return 0;
    st->data = nd;
    st->cap = newcap;
    return 1;
}

static int stack_push(NodeStack* st, TreeNode* node) {
    if (st->size == st->cap) {
        if (!stack_grow(st)) return 0;
    }
    st->data[st->size++] = node;
    return 1;
}

static TreeNode* stack_pop(NodeStack* st) {
    if (st->size == 0) return NULL;
    return st->data[--st->size];
}

static TreeNode* stack_peek(NodeStack* st) {
    if (st->size == 0) return NULL;
    return st->data[st->size - 1];
}

static int stack_empty(NodeStack* st) {
    return st->size == 0;
}

static TreeNode* new_node(int v) {
    TreeNode* n = (TreeNode*)malloc(sizeof(TreeNode));
    if (!n) return NULL;
    n->val = v;
    n->left = n->right = NULL;
    return n;
}

static void free_tree(TreeNode* root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

static size_t count_nodes(TreeNode* root) {
    if (!root) return 0;
    return 1 + count_nodes(root->left) + count_nodes(root->right);
}

static void to_lower_trim(const char* in, char* out, size_t out_cap) {
    if (!in || !out || out_cap == 0) return;
    size_t len = strlen(in);
    size_t start = 0;
    while (start < len && isspace((unsigned char)in[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)in[end - 1])) end--;
    size_t j = 0;
    for (size_t i = start; i < end && j + 1 < out_cap; ++i) {
        out[j++] = (char)tolower((unsigned char)in[i]);
    }
    out[j] = '\0';
}

IntArray dfs(TreeNode* root, const char* order) {
    IntArray result = {NULL, 0};
    char ord[32] = {0};
    to_lower_trim(order ? order : "", ord, sizeof(ord));
    size_t n = count_nodes(root);
    if (n == 0) {
        result.data = NULL;
        result.size = 0;
        return result;
    }
    size_t bytes;
    if (!safe_mul_size_t(n, sizeof(int), &bytes)) {
        result.data = NULL;
        result.size = 0;
        return result;
    }
    int* out = (int*)malloc(bytes);
    if (!out) {
        result.data = NULL;
        result.size = 0;
        return result;
    }
    size_t idx = 0;

    if (strcmp(ord, "preorder") == 0) {
        NodeStack st; stack_init(&st);
        if (root) {
            if (!stack_push(&st, root)) { free(out); stack_free(&st); return (IntArray){NULL,0}; }
            while (!stack_empty(&st)) {
                TreeNode* node = stack_pop(&st);
                out[idx++] = node->val;
                if (node->right) { if (!stack_push(&st, node->right)) { free(out); stack_free(&st); return (IntArray){NULL,0}; } }
                if (node->left) { if (!stack_push(&st, node->left)) { free(out); stack_free(&st); return (IntArray){NULL,0}; } }
            }
        }
        stack_free(&st);
    } else if (strcmp(ord, "inorder") == 0) {
        NodeStack st; stack_init(&st);
        TreeNode* curr = root;
        while (curr || !stack_empty(&st)) {
            while (curr) {
                if (!stack_push(&st, curr)) { free(out); stack_free(&st); return (IntArray){NULL,0}; }
                curr = curr->left;
            }
            TreeNode* node = stack_pop(&st);
            out[idx++] = node->val;
            curr = node->right;
        }
        stack_free(&st);
    } else if (strcmp(ord, "postorder") == 0) {
        NodeStack st; stack_init(&st);
        TreeNode* curr = root;
        TreeNode* last = NULL;
        while (curr || !stack_empty(&st)) {
            if (curr) {
                if (!stack_push(&st, curr)) { free(out); stack_free(&st); return (IntArray){NULL,0}; }
                curr = curr->left;
            } else {
                TreeNode* peek = stack_peek(&st);
                if (peek->right && last != peek->right) {
                    curr = peek->right;
                } else {
                    out[idx++] = peek->val;
                    last = stack_pop(&st);
                }
            }
        }
        stack_free(&st);
    } else {
        free(out);
        // Unsupported order: return empty
        return (IntArray){NULL, 0};
    }

    result.data = out;
    result.size = idx;
    return result;
}

static void print_array(const char* label, const IntArray arr) {
    printf("%s: [", label);
    for (size_t i = 0; i < arr.size; ++i) {
        if (i) printf(", ");
        printf("%d", arr.data[i]);
    }
    printf("]\n");
}

int main(void) {
    // Test 1: Empty tree
    TreeNode* t1 = NULL;
    IntArray r1 = dfs(t1, "preorder");
    print_array("Test1 - Preorder (empty)", r1);
    free(r1.data);

    // Test 2: Single node
    TreeNode* t2 = new_node(42);
    IntArray r2 = dfs(t2, "inorder");
    print_array("Test2 - Inorder (single)", r2);
    free(r2.data);
    free_tree(t2);

    // Test 3: Balanced tree
    TreeNode* t3 = new_node(4);
    t3->left = new_node(2);
    t3->right = new_node(6);
    t3->left->left = new_node(1);
    t3->left->right = new_node(3);
    t3->right->left = new_node(5);
    t3->right->right = new_node(7);
    IntArray r3 = dfs(t3, "inorder");
    print_array("Test3 - Inorder (balanced)", r3);
    free(r3.data);
    free_tree(t3);

    // Test 4: Left-skewed
    TreeNode* t4 = new_node(5);
    t4->left = new_node(4);
    t4->left->left = new_node(3);
    t4->left->left->left = new_node(2);
    t4->left->left->left->left = new_node(1);
    IntArray r4 = dfs(t4, "postorder");
    print_array("Test4 - Postorder (left-skewed)", r4);
    free(r4.data);
    free_tree(t4);

    // Test 5: Mixed tree
    TreeNode* t5 = new_node(8);
    t5->left = new_node(3);
    t5->right = new_node(10);
    t5->left->left = new_node(1);
    t5->left->right = new_node(6);
    t5->left->right->left = new_node(4);
    t5->left->right->right = new_node(7);
    t5->right->right = new_node(14);
    t5->right->right->left = new_node(13);
    IntArray r5 = dfs(t5, "preorder");
    print_array("Test5 - Preorder (mixed)", r5);
    free(r5.data);
    free_tree(t5);

    return 0;
}