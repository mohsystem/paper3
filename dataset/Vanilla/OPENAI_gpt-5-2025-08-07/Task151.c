#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int key;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct {
    Node* root;
} BST;

Node* new_node(int key) {
    Node* n = (Node*)malloc(sizeof(Node));
    n->key = key;
    n->left = n->right = NULL;
    return n;
}

int search(BST* tree, int key) {
    Node* cur = tree->root;
    while (cur) {
        if (key == cur->key) return 1;
        if (key < cur->key) cur = cur->left; else cur = cur->right;
    }
    return 0;
}

Node* insert_rec(Node* node, int key, int* inserted) {
    if (!node) {
        *inserted = 1;
        return new_node(key);
    }
    if (key < node->key) node->left = insert_rec(node->left, key, inserted);
    else if (key > node->key) node->right = insert_rec(node->right, key, inserted);
    else *inserted = 0;
    return node;
}

int insert(BST* tree, int key) {
    int inserted = 0;
    tree->root = insert_rec(tree->root, key, &inserted);
    return inserted;
}

Node* delete_rec(Node* node, int key, int* deleted) {
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
        }
        if (!node->right) {
            Node* l = node->left;
            free(node);
            return l;
        }
        Node* succ = node->right;
        while (succ->left) succ = succ->left;
        node->key = succ->key;
        node->right = delete_rec(node->right, succ->key, deleted);
    }
    return node;
}

int delete_value(BST* tree, int key) {
    int deleted = 0;
    tree->root = delete_rec(tree->root, key, &deleted);
    return deleted;
}

void inorder_collect(Node* node, int* arr, int* idx) {
    if (!node) return;
    inorder_collect(node->left, arr, idx);
    arr[(*idx)++] = node->key;
    inorder_collect(node->right, arr, idx);
}

int count_nodes(Node* node) {
    if (!node) return 0;
    return 1 + count_nodes(node->left) + count_nodes(node->right);
}

int* inorder(BST* tree, int* out_size) {
    *out_size = count_nodes(tree->root);
    int* arr = (int*)malloc(sizeof(int) * (*out_size));
    int idx = 0;
    inorder_collect(tree->root, arr, &idx);
    return arr;
}

void free_rec(Node* node) {
    if (!node) return;
    free_rec(node->left);
    free_rec(node->right);
    free(node);
}

int main() {
    BST bst;
    bst.root = NULL;

    // Test 1: Insert values and print inorder
    int vals[] = {5, 3, 7, 2, 4, 6, 8};
    for (int i = 0; i < 7; ++i) {
        printf("%d ", insert(&bst, vals[i]));
    }
    printf("\n");
    int n = 0;
    int* arr = inorder(&bst, &n);
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
    free(arr);

    // Test 2: Search existing value
    printf("%d\n", search(&bst, 4));

    // Test 3: Search non-existing value
    printf("%d\n", search(&bst, 10));

    // Test 4: Delete leaf (2) and node with one child (3), then print inorder
    printf("%d\n", delete_value(&bst, 2));
    printf("%d\n", delete_value(&bst, 3));
    arr = inorder(&bst, &n);
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
    free(arr);

    // Test 5: Delete node with two children (7), try duplicate insert (5), delete non-existent (42)
    printf("%d\n", delete_value(&bst, 7));
    printf("%d\n", insert(&bst, 5));
    printf("%d\n", delete_value(&bst, 42));
    arr = inorder(&bst, &n);
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
    free(arr);

    free_rec(bst.root);
    return 0;
}