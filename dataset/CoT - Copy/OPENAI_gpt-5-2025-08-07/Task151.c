#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 Chain-of-Through Steps:
 1) Implement BST with insert, delete, search
 2) Security: avoid unsafe input, check allocations, manage memory
 3-5) Secure coding, review, finalize
*/

typedef struct Node {
    int key;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct {
    Node* root;
} BST;

static Node* make_node(int key) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return NULL;
    n->key = key;
    n->left = n->right = NULL;
    return n;
}

void bst_init(BST* t) {
    if (t) t->root = NULL;
}

static Node* insert_rec(Node* node, int key, int* inserted) {
    if (!node) {
        Node* n = make_node(key);
        if (n) *inserted = 1;
        return n;
    }
    if (key < node->key) node->left = insert_rec(node->left, key, inserted);
    else if (key > node->key) node->right = insert_rec(node->right, key, inserted);
    else *inserted = 0; // duplicate
    return node;
}

int bst_insert(BST* t, int key) {
    if (!t) return 0;
    int inserted = 0;
    t->root = insert_rec(t->root, key, &inserted);
    return inserted;
}

int bst_search(const BST* t, int key) {
    if (!t) return 0;
    Node* cur = t->root;
    while (cur) {
        if (key == cur->key) return 1;
        cur = (key < cur->key) ? cur->left : cur->right;
    }
    return 0;
}

static Node* min_value_node(Node* node) {
    Node* cur = node;
    while (cur && cur->left) cur = cur->left;
    return cur;
}

static Node* delete_rec(Node* node, int key, int* deleted) {
    if (!node) return NULL;
    if (key < node->key) {
        node->left = delete_rec(node->left, key, deleted);
    } else if (key > node->key) {
        node->right = delete_rec(node->right, key, deleted);
    } else {
        *deleted = 1;
        if (!node->left) {
            Node* r = node->right;
            free(node);
            return r;
        } else if (!node->right) {
            Node* l = node->left;
            free(node);
            return l;
        } else {
            Node* succ = min_value_node(node->right);
            node->key = succ->key;
            int dummy = 0;
            node->right = delete_rec(node->right, succ->key, &dummy);
        }
    }
    return node;
}

int bst_delete(BST* t, int key) {
    if (!t) return 0;
    int deleted = 0;
    t->root = delete_rec(t->root, key, &deleted);
    return deleted;
}

static int bst_size_rec(const Node* n) {
    if (!n) return 0;
    int left = bst_size_rec(n->left);
    int right = bst_size_rec(n->right);
    return left + right + 1;
}

int bst_size(const BST* t) {
    if (!t) return 0;
    return bst_size_rec(t->root);
}

static void inorder_fill(const Node* n, int* arr, int* idx, int cap) {
    if (!n || !arr || !idx) return;
    inorder_fill(n->left, arr, idx, cap);
    if (*idx < cap) {
        arr[*idx] = n->key;
        (*idx)++;
    }
    inorder_fill(n->right, arr, idx, cap);
}

// Returns a newly allocated array with inorder traversal, sets out_count. Caller must free.
int* bst_inorder(const BST* t, int* out_count) {
    if (out_count) *out_count = 0;
    if (!t || !out_count) return NULL;
    int n = bst_size(t);
    *out_count = n;
    if (n <= 0) return NULL;
    int* arr = (int*)malloc(sizeof(int) * (size_t)n);
    if (!arr) {
        *out_count = 0;
        return NULL;
    }
    int idx = 0;
    inorder_fill(t->root, arr, &idx, n);
    return arr;
}

static void destroy_rec(Node* n) {
    if (!n) return;
    destroy_rec(n->left);
    destroy_rec(n->right);
    free(n);
}

void bst_destroy(BST* t) {
    if (!t) return;
    destroy_rec(t->root);
    t->root = NULL;
}

static void print_array(const char* prefix, const int* arr, int n) {
    printf("%s[", prefix ? prefix : "");
    for (int i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    // Test case 1: Insert and inorder
    BST bst;
    bst_init(&bst);
    int values[] = {50, 30, 20, 40, 70, 60, 80};
    for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
        bst_insert(&bst, values[i]);
    }
    int count = 0;
    int* order = bst_inorder(&bst, &count);
    print_array("Inorder after inserts: ", order, count);
    free(order);

    // Test case 2: Search existing
    printf("Search 40: %s\n", bst_search(&bst, 40) ? "true" : "false");

    // Test case 3: Search non-existing
    printf("Search 25: %s\n", bst_search(&bst, 25) ? "true" : "false");

    // Test case 4: Delete leaf (20)
    printf("Delete 20: %s\n", bst_delete(&bst, 20) ? "true" : "false");
    order = bst_inorder(&bst, &count);
    print_array("Inorder after deleting 20: ", order, count);
    free(order);

    // Test case 5: Delete node with one child (30) and two children (50)
    printf("Delete 30: %s\n", bst_delete(&bst, 30) ? "true" : "false");
    printf("Delete 50: %s\n", bst_delete(&bst, 50) ? "true" : "false");
    order = bst_inorder(&bst, &count);
    print_array("Inorder after deleting 30 and 50: ", order, count);
    free(order);

    bst_destroy(&bst);
    return 0;
}