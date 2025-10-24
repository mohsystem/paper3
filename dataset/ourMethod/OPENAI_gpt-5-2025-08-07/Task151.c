#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct Node {
    int key;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct BST {
    Node* root;
} BST;

static int is_int_safe(int value) {
    (void)value; // All int values acceptable in this implementation
    return 1;
}

static Node* create_node(int key) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return NULL;
    n->key = key;
    n->left = NULL;
    n->right = NULL;
    return n;
}

void bst_init(BST* bst) {
    if (bst) bst->root = NULL;
}

static void free_rec(Node* node) {
    if (!node) return;
    free_rec(node->left);
    free_rec(node->right);
    free(node);
}

void bst_free(BST* bst) {
    if (!bst) return;
    free_rec(bst->root);
    bst->root = NULL;
}

int bst_insert(BST* bst, int key) {
    if (!bst || !is_int_safe(key)) return 0;
    if (!bst->root) {
        bst->root = create_node(key);
        return bst->root != NULL;
    }
    Node* curr = bst->root;
    Node* parent = NULL;
    while (curr) {
        if (key == curr->key) return 0; // no duplicates
        parent = curr;
        if (key < curr->key) curr = curr->left;
        else curr = curr->right;
    }
    Node* n = create_node(key);
    if (!n) return 0;
    if (key < parent->key) parent->left = n;
    else parent->right = n;
    return 1;
}

int bst_search(BST* bst, int key) {
    if (!bst || !is_int_safe(key)) return 0;
    Node* curr = bst->root;
    while (curr) {
        if (key == curr->key) return 1;
        curr = (key < curr->key) ? curr->left : curr->right;
    }
    return 0;
}

static Node* min_node(Node* node) {
    Node* curr = node;
    while (curr && curr->left) curr = curr->left;
    return curr;
}

static Node* delete_min(Node* node) {
    if (!node) return NULL;
    if (!node->left) {
        Node* r = node->right;
        free(node);
        return r;
    }
    node->left = delete_min(node->left);
    return node;
}

static Node* delete_rec(Node* node, int key, int* found) {
    if (!node) return NULL;
    if (key < node->key) {
        node->left = delete_rec(node->left, key, found);
    } else if (key > node->key) {
        node->right = delete_rec(node->right, key, found);
    } else {
        *found = 1;
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
        Node* succ = min_node(node->right);
        Node* newNode = create_node(succ->key);
        if (!newNode) return node; // allocation failed; keep original node to avoid data loss
        newNode->left = node->left;
        newNode->right = delete_min(node->right);
        free(node);
        return newNode;
    }
    return node;
}

int bst_delete(BST* bst, int key) {
    if (!bst || !is_int_safe(key)) return 0;
    int found = 0;
    bst->root = delete_rec(bst->root, key, &found);
    return found;
}

// Dynamic string builder utilities
static int append_str(char** buf, size_t* cap, size_t* len, const char* s) {
    if (!buf || !cap || !len || !s) return 0;
    size_t slen = strlen(s);
    if (*len + slen + 1 > *cap) {
        size_t new_cap = (*cap == 0) ? 64 : *cap;
        while (*len + slen + 1 > new_cap) {
            if (new_cap > SIZE_MAX / 2) return 0; // prevent overflow
            new_cap *= 2;
        }
        char* new_buf = (char*)realloc(*buf, new_cap);
        if (!new_buf) return 0;
        *buf = new_buf;
        *cap = new_cap;
    }
    memcpy(*buf + *len, s, slen);
    *len += slen;
    (*buf)[*len] = '\0';
    return 1;
}

static void inorder_build(Node* node, char** buf, size_t* cap, size_t* len) {
    if (!node) return;
    inorder_build(node->left, buf, cap, len);
    char tmp[32];
    int n = snprintf(tmp, sizeof(tmp), "%d ", node->key);
    if (n < 0 || (size_t)n >= sizeof(tmp)) return; // formatting error or truncated; fail silently
    (void)append_str(buf, cap, len, tmp);
    inorder_build(node->right, buf, cap, len);
}

char* bst_inorder_to_string(BST* bst) {
    if (!bst) return NULL;
    char* buf = NULL;
    size_t cap = 0;
    size_t len = 0;
    inorder_build(bst->root, &buf, &cap, &len);
    if (!buf) {
        buf = (char*)malloc(1);
        if (buf) buf[0] = '\0';
    }
    return buf;
}

int main(void) {
    BST bst;
    bst_init(&bst);

    // Test Case 1: Insert elements and print inorder
    int values[] = {50, 30, 70, 20, 40, 60, 80};
    size_t values_count = sizeof(values) / sizeof(values[0]);
    for (size_t i = 0; i < values_count; ++i) {
        printf("Insert %d: %s\n", values[i], bst_insert(&bst, values[i]) ? "true" : "false");
    }
    char* s1 = bst_inorder_to_string(&bst);
    printf("Inorder after inserts: %s\n", s1 ? s1 : "");
    free(s1);

    // Test Case 2: Search existing and non-existing keys
    printf("Search 60: %s\n", bst_search(&bst, 60) ? "true" : "false");
    printf("Search 25: %s\n", bst_search(&bst, 25) ? "true" : "false");

    // Test Case 3: Delete a leaf node
    printf("Delete 20 (leaf): %s\n", bst_delete(&bst, 20) ? "true" : "false");
    char* s2 = bst_inorder_to_string(&bst);
    printf("Inorder after deleting 20: %s\n", s2 ? s2 : "");
    free(s2);

    // Test Case 4: Delete a node with one child
    printf("Delete 30 (one child): %s\n", bst_delete(&bst, 30) ? "true" : "false");
    char* s3 = bst_inorder_to_string(&bst);
    printf("Inorder after deleting 30: %s\n", s3 ? s3 : "");
    free(s3);

    // Test Case 5: Delete a node with two children and try duplicate insert
    printf("Delete 50 (two children): %s\n", bst_delete(&bst, 50) ? "true" : "false");
    char* s4 = bst_inorder_to_string(&bst);
    printf("Inorder after deleting 50: %s\n", s4 ? s4 : "");
    free(s4);
    printf("Insert duplicate 70: %s\n", bst_insert(&bst, 70) ? "true" : "false");

    bst_free(&bst);
    return 0;
}