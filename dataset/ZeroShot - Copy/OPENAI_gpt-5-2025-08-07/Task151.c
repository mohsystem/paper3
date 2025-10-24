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

typedef struct {
    Node** data;
    size_t size;
    size_t cap;
} NodeStack;

static void ns_init(NodeStack* s) {
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}
static int ns_push(NodeStack* s, Node* n) {
    if (s->size == s->cap) {
        size_t newCap = s->cap ? s->cap * 2 : 8;
        Node** tmp = (Node**)realloc(s->data, newCap * sizeof(Node*));
        if (!tmp) return 0;
        s->data = tmp;
        s->cap = newCap;
    }
    s->data[s->size++] = n;
    return 1;
}
static Node* ns_pop(NodeStack* s) {
    if (s->size == 0) return NULL;
    return s->data[--s->size];
}
static int ns_empty(NodeStack* s) { return s->size == 0; }
static void ns_free(NodeStack* s) { free(s->data); s->data = NULL; s->size = s->cap = 0; }

typedef struct {
    int* data;
    size_t size;
    size_t cap;
} IntVec;

static void iv_init(IntVec* v) { v->data = NULL; v->size = 0; v->cap = 0; }
static int iv_push(IntVec* v, int x) {
    if (v->size == v->cap) {
        size_t newCap = v->cap ? v->cap * 2 : 16;
        int* tmp = (int*)realloc(v->data, newCap * sizeof(int));
        if (!tmp) return 0;
        v->data = tmp;
        v->cap = newCap;
    }
    v->data[v->size++] = x;
    return 1;
}

static Node* make_node(int key) {
    Node* n = (Node*)calloc(1, sizeof(Node));
    if (!n) return NULL;
    n->key = key;
    return n;
}

void bst_init(BST* t) { t->root = NULL; }

int bst_insert(BST* t, int key) {
    if (!t) return 0;
    if (!t->root) {
        t->root = make_node(key);
        return t->root != NULL;
    }
    Node* parent = NULL;
    Node* curr = t->root;
    while (curr) {
        parent = curr;
        if (key == curr->key) return 0;
        curr = (key < curr->key) ? curr->left : curr->right;
    }
    Node* n = make_node(key);
    if (!n) return 0;
    if (key < parent->key) parent->left = n;
    else parent->right = n;
    return 1;
}

int bst_search(BST* t, int key) {
    if (!t) return 0;
    Node* curr = t->root;
    while (curr) {
        if (key == curr->key) return 1;
        curr = (key < curr->key) ? curr->left : curr->right;
    }
    return 0;
}

int bst_delete(BST* t, int key) {
    if (!t) return 0;
    Node* parent = NULL;
    Node* curr = t->root;
    while (curr && curr->key != key) {
        parent = curr;
        curr = (key < curr->key) ? curr->left : curr->right;
    }
    if (!curr) return 0;

    if (curr->left && curr->right) {
        Node* succParent = curr;
        Node* succ = curr->right;
        while (succ->left) {
            succParent = succ;
            succ = succ->left;
        }
        curr->key = succ->key;
        parent = succParent;
        curr = succ;
    }

    Node* child = curr->left ? curr->left : curr->right;
    if (!parent) {
        t->root = child;
    } else if (parent->left == curr) {
        parent->left = child;
    } else {
        parent->right = child;
    }
    free(curr);
    return 1;
}

int bst_inorder(BST* t, int** outArr, size_t* outSize) {
    if (!t || !outArr || !outSize) return 0;
    *outArr = NULL;
    *outSize = 0;

    IntVec vec; iv_init(&vec);
    NodeStack st; ns_init(&st);

    Node* curr = t->root;
    while (curr || !ns_empty(&st)) {
        while (curr) {
            if (!ns_push(&st, curr)) { ns_free(&st); free(vec.data); return 0; }
            curr = curr->left;
        }
        curr = ns_pop(&st);
        if (!iv_push(&vec, curr->key)) { ns_free(&st); free(vec.data); return 0; }
        curr = curr->right;
    }
    ns_free(&st);
    *outArr = vec.data;
    *outSize = vec.size;
    return 1;
}

void bst_free(BST* t) {
    if (!t || !t->root) return;
    NodeStack st; ns_init(&st);
    ns_push(&st, t->root);
    while (!ns_empty(&st)) {
        Node* n = ns_pop(&st);
        if (n->left) ns_push(&st, n->left);
        if (n->right) ns_push(&st, n->right);
        free(n);
    }
    ns_free(&st);
    t->root = NULL;
}

static void print_array(const int* a, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", a[i]);
    }
    printf("]");
}

int main(void) {
    BST tree; bst_init(&tree);
    int initial[] = {5, 3, 7, 2, 4, 6, 8};
    size_t initial_len = sizeof(initial) / sizeof(initial[0]);
    for (size_t i = 0; i < initial_len; ++i) {
        if (!bst_insert(&tree, initial[i])) {
            fprintf(stderr, "Insert failed for %d\n", initial[i]);
            bst_free(&tree);
            return 1;
        }
    }

    printf("Search 4 (expect 1): %d\n", bst_search(&tree, 4));
    printf("Search 10 (expect 0): %d\n", bst_search(&tree, 10));

    int* arr = NULL; size_t n = 0;

    bst_delete(&tree, 2);
    if (!bst_inorder(&tree, &arr, &n)) { bst_free(&tree); return 1; }
    printf("After deleting 2 (leaf): "); print_array(arr, n); printf("\n");
    free(arr); arr = NULL;

    bst_delete(&tree, 3);
    if (!bst_inorder(&tree, &arr, &n)) { bst_free(&tree); return 1; }
    printf("After deleting 3 (one child): "); print_array(arr, n); printf("\n");
    free(arr); arr = NULL;

    bst_delete(&tree, 5);
    if (!bst_inorder(&tree, &arr, &n)) { bst_free(&tree); return 1; }
    printf("After deleting 5 (two children): "); print_array(arr, n); printf("\n");
    free(arr); arr = NULL;

    bst_free(&tree);
    return 0;
}