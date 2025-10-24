#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    int value;
    struct Node** children;
    size_t count;
    size_t cap;
    struct Node* parent; // used only during building on the original tree
} Node;

static Node* node_new(int value) {
    Node* n = (Node*)calloc(1, sizeof(Node));
    if (!n) return NULL;
    n->value = value;
    n->children = NULL;
    n->count = 0;
    n->cap = 0;
    n->parent = NULL;
    return n;
}

static int node_add_child(Node* parent, Node* child) {
    if (!parent || !child) return -1;
    if (parent->count == parent->cap) {
        size_t new_cap = parent->cap == 0 ? 4 : parent->cap * 2;
        Node** tmp = (Node**)realloc(parent->children, new_cap * sizeof(Node*));
        if (!tmp) return -1;
        parent->children = tmp;
        parent->cap = new_cap;
    }
    parent->children[parent->count++] = child;
    return 0;
}

static void assign_parents(Node* node, Node* parent) {
    if (!node) return;
    node->parent = parent;
    for (size_t i = 0; i < node->count; i++) {
        assign_parents(node->children[i], node);
    }
}

static Node* find(Node* node, int target) {
    if (!node) return NULL;
    if (node->value == target) return node;
    for (size_t i = 0; i < node->count; i++) {
        Node* f = find(node->children[i], target);
        if (f) return f;
    }
    return NULL;
}

static Node* build_reoriented(Node* node, Node* prev) {
    Node* clone = node_new(node->value);
    if (!clone) return NULL;
    for (size_t i = 0; i < node->count; i++) {
        Node* c = node->children[i];
        if (c != prev) {
            Node* cc = build_reoriented(c, node);
            if (!cc || node_add_child(clone, cc) != 0) {
                // Free partially built subtree
                // Define a local free function
                // Forward declaration not needed; implement inline
                // We'll implement a helper to free tree
                // For reuse, define below and call here after prototype
                // But since it's below, declare prototype above
                ;
            }
            if (!cc || node_add_child(clone, cc) != 0) {
                // If adding failed, free cc and clone
                // Free cc subtree
                // We'll define free_tree and use it
                return NULL; // placeholder, actual freeing done after function is defined
            }
        }
    }
    if (node->parent && node->parent != prev) {
        Node* pc = build_reoriented(node->parent, node);
        if (!pc || node_add_child(clone, pc) != 0) {
            return NULL; // placeholder; freeing handled after helper defined
        }
    }
    return clone;
}

// Forward declare free_tree for use inside build_reoriented (we'll re-implement build safely below)
static void free_tree(Node* node);

static Node* build_reoriented_safe(Node* node, Node* prev) {
    Node* clone = node_new(node->value);
    if (!clone) return NULL;

    // process children neighbors
    for (size_t i = 0; i < node->count; i++) {
        Node* c = node->children[i];
        if (c != prev) {
            Node* cc = build_reoriented_safe(c, node);
            if (!cc) {
                free_tree(clone);
                return NULL;
            }
            if (node_add_child(clone, cc) != 0) {
                free_tree(cc);
                free_tree(clone);
                return NULL;
            }
        }
    }
    // process parent neighbor
    if (node->parent && node->parent != prev) {
        Node* pc = build_reoriented_safe(node->parent, node);
        if (!pc) {
            free_tree(clone);
            return NULL;
        }
        if (node_add_child(clone, pc) != 0) {
            free_tree(pc);
            free_tree(clone);
            return NULL;
        }
    }
    return clone;
}

static Node* reparent(Node* root, int target_value) {
    if (!root) return NULL;
    assign_parents(root, NULL);
    Node* target = find(root, target_value);
    if (!target) return NULL;
    return build_reoriented_safe(target, NULL);
}

static void free_tree(Node* node) {
    if (!node) return;
    for (size_t i = 0; i < node->count; i++) {
        free_tree(node->children[i]);
    }
    free(node->children);
    free(node);
}

static size_t int_len(int v) {
    if (v == 0) return 1;
    size_t len = 0;
    int x = v;
    if (x < 0) {
        len++;
        x = -x;
    }
    while (x > 0) {
        len++;
        x /= 10;
    }
    return len;
}

static size_t calc_size(Node* node) {
    if (!node) return 4; // "null"
    size_t len = int_len(node->value);
    if (node->count > 0) {
        len += 1; // '('
        for (size_t i = 0; i < node->count; i++) {
            len += calc_size(node->children[i]);
            if (i + 1 < node->count) len += 1; // ','
        }
        len += 1; // ')'
    }
    return len;
}

static void append_int(char* buf, size_t cap, size_t* idx, int v) {
    if (!buf || *idx >= cap) return;
    int written = snprintf(buf + *idx, cap - *idx, "%d", v);
    if (written < 0) return;
    size_t w = (size_t)written;
    if (*idx + w >= cap) {
        *idx = cap - 1;
    } else {
        *idx += w;
    }
}

static void write_tree(Node* node, char* buf, size_t cap, size_t* idx) {
    if (!node) {
        const char* s = "null";
        size_t sl = 4;
        if (*idx + sl >= cap) sl = (cap > *idx) ? (cap - *idx - 1) : 0;
        if (sl > 0) {
            memcpy(buf + *idx, s, sl);
            *idx += sl;
            buf[*idx] = '\0';
        }
        return;
    }
    append_int(buf, cap, idx, node->value);
    if (node->count > 0) {
        if (*idx < cap - 1) buf[(*idx)++] = '(';
        for (size_t i = 0; i < node->count; i++) {
            write_tree(node->children[i], buf, cap, idx);
            if (i + 1 < node->count) {
                if (*idx < cap - 1) buf[(*idx)++] = ',';
            }
        }
        if (*idx < cap - 1) buf[(*idx)++] = ')';
        if (*idx < cap) buf[*idx] = '\0';
    }
}

static char* tree_to_string(Node* node) {
    size_t size = calc_size(node);
    char* buf = (char*)malloc(size + 1);
    if (!buf) return NULL;
    size_t idx = 0;
    if (size > 0) buf[0] = '\0';
    write_tree(node, buf, size + 1, &idx);
    buf[size] = '\0';
    return buf;
}

// Build example tree:
//        0
//      / | \
//     1  2  3
//    / \ / \ / \
//   4  5 6  7 8  9
static Node* build_example_tree(void) {
    Node* n0 = node_new(0);
    Node* n1 = node_new(1);
    Node* n2 = node_new(2);
    Node* n3 = node_new(3);
    Node* n4 = node_new(4);
    Node* n5 = node_new(5);
    Node* n6 = node_new(6);
    Node* n7 = node_new(7);
    Node* n8 = node_new(8);
    Node* n9 = node_new(9);

    node_add_child(n0, n1); node_add_child(n0, n2); node_add_child(n0, n3);
    node_add_child(n1, n4); node_add_child(n1, n5);
    node_add_child(n2, n6); node_add_child(n2, n7);
    node_add_child(n3, n8); node_add_child(n3, n9);
    return n0;
}

static Node* build_single_node(int value) {
    return node_new(value);
}

int main(void) {
    // Test 1: Reparent on 6
    Node* root = build_example_tree();
    Node* r1 = reparent(root, 6);
    char* s1 = tree_to_string(r1);
    printf("%s\n", s1 ? s1 : "null");
    free(s1);
    free_tree(r1);

    // Test 2: Reparent on 0
    Node* r2 = reparent(root, 0);
    char* s2 = tree_to_string(r2);
    printf("%s\n", s2 ? s2 : "null");
    free(s2);
    free_tree(r2);

    // Test 3: Reparent on 9
    Node* r3 = reparent(root, 9);
    char* s3 = tree_to_string(r3);
    printf("%s\n", s3 ? s3 : "null");
    free(s3);
    free_tree(r3);

    // Test 4: Single node tree
    Node* single = build_single_node(42);
    Node* r4 = reparent(single, 42);
    char* s4 = tree_to_string(r4);
    printf("%s\n", s4 ? s4 : "null");
    free(s4);
    free_tree(r4);
    free_tree(single);

    // Test 5: Target not found
    Node* r5 = reparent(root, 100);
    char* s5 = tree_to_string(r5);
    printf("%s\n", s5 ? s5 : "null");
    free(s5);
    free_tree(r5);

    free_tree(root);
    return 0;
}