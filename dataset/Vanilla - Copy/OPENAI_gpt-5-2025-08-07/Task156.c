#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Node {
    int val;
    struct Node* left;
    struct Node* right;
} Node;

Node* newNode(int val) {
    Node* n = (Node*)malloc(sizeof(Node));
    n->val = val;
    n->left = n->right = NULL;
    return n;
}

int countNodes(Node* root) {
    if (!root) return 0;
    return 1 + countNodes(root->left) + countNodes(root->right);
}

void fillPreorder(Node* root, int* arr, int* idx) {
    if (!root) return;
    arr[(*idx)++] = root->val;
    fillPreorder(root->left, arr, idx);
    fillPreorder(root->right, arr, idx);
}

void fillInorder(Node* root, int* arr, int* idx) {
    if (!root) return;
    fillInorder(root->left, arr, idx);
    arr[(*idx)++] = root->val;
    fillInorder(root->right, arr, idx);
}

void fillPostorder(Node* root, int* arr, int* idx) {
    if (!root) return;
    fillPostorder(root->left, arr, idx);
    fillPostorder(root->right, arr, idx);
    arr[(*idx)++] = root->val;
}

// Returns a dynamically allocated array with traversal result; sets returnSize.
// Caller is responsible for freeing the returned array.
int* dfs(Node* root, const char* order, int* returnSize) {
    if (returnSize == NULL) return NULL;
    if (!root) {
        *returnSize = 0;
        return NULL;
    }
    int n = countNodes(root);
    int* arr = (int*)malloc(sizeof(int) * n);
    int idx = 0;

    // Lowercase copy of order
    char buf[32];
    int i = 0;
    for (; i < 31 && order && order[i] != '\0'; ++i) buf[i] = (char)tolower((unsigned char)order[i]);
    buf[i] = '\0';

    if (strcmp(buf, "preorder") == 0) {
        fillPreorder(root, arr, &idx);
    } else if (strcmp(buf, "inorder") == 0) {
        fillInorder(root, arr, &idx);
    } else if (strcmp(buf, "postorder") == 0) {
        fillPostorder(root, arr, &idx);
    } else {
        // Unknown order: default to preorder
        fillPreorder(root, arr, &idx);
    }

    *returnSize = idx;
    return arr;
}

int* preorder(Node* root, int* returnSize) { return dfs(root, "preorder", returnSize); }
int* inorder(Node* root, int* returnSize) { return dfs(root, "inorder", returnSize); }
int* postorder(Node* root, int* returnSize) { return dfs(root, "postorder", returnSize); }

void printArray(const char* label, int* arr, int n) {
    printf("%s: [", label);
    for (int i = 0; i < n; ++i) {
        printf("%d", arr[i]);
        if (i + 1 < n) printf(", ");
    }
    printf("]\n");
}

void freeTree(Node* root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

int main() {
    // Test 1: Balanced tree
    Node* root1 = newNode(1);
    root1->left = newNode(2);
    root1->right = newNode(3);
    root1->left->left = newNode(4);
    root1->left->right = newNode(5);
    root1->right->left = newNode(6);
    root1->right->right = newNode(7);
    int n1 = 0;
    int* a1 = dfs(root1, "preorder", &n1);
    printArray("Test1 Preorder", a1, n1);
    free(a1);
    freeTree(root1);

    // Test 2: Single node
    Node* root2 = newNode(10);
    int n2 = 0;
    int* a2 = dfs(root2, "inorder", &n2);
    printArray("Test2 Inorder", a2, n2);
    free(a2);
    freeTree(root2);

    // Test 3: Left-skewed
    Node* root3 = newNode(5);
    root3->left = newNode(4);
    root3->left->left = newNode(3);
    root3->left->left->left = newNode(2);
    root3->left->left->left->left = newNode(1);
    int n3 = 0;
    int* a3 = dfs(root3, "postorder", &n3);
    printArray("Test3 Postorder", a3, n3);
    free(a3);
    freeTree(root3);

    // Test 4: Right-skewed
    Node* root4 = newNode(1);
    root4->right = newNode(2);
    root4->right->right = newNode(3);
    root4->right->right->right = newNode(4);
    root4->right->right->right->right = newNode(5);
    int n4 = 0;
    int* a4 = dfs(root4, "preorder", &n4);
    printArray("Test4 Preorder", a4, n4);
    free(a4);
    freeTree(root4);

    // Test 5: Empty tree
    Node* root5 = NULL;
    int n5 = 0;
    int* a5 = dfs(root5, "inorder", &n5);
    printArray("Test5 Inorder (empty)", a5, n5);
    // a5 is NULL; no free needed
    // root5 is NULL; no free needed

    return 0;
}