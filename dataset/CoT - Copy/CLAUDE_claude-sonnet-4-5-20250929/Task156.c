
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct TreeNode {
    int val;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

TreeNode* createNode(int val) {
    if (val < INT_MIN || val > INT_MAX) {
        return NULL;
    }
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if (node == NULL) {
        return NULL;
    }
    node->val = val;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(TreeNode* root) {
    if (root == NULL) {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

void preorderHelper(TreeNode* node, int* result, int* index) {
    if (node == NULL) {
        return;
    }
    result[(*index)++] = node->val;
    preorderHelper(node->left, result, index);
    preorderHelper(node->right, result, index);
}

int preorderTraversal(TreeNode* root, int* result) {
    if (root == NULL || result == NULL) {
        return 0;
    }
    int index = 0;
    preorderHelper(root, result, &index);
    return index;
}

void inorderHelper(TreeNode* node, int* result, int* index) {
    if (node == NULL) {
        return;
    }
    inorderHelper(node->left, result, index);
    result[(*index)++] = node->val;
    inorderHelper(node->right, result, index);
}

int inorderTraversal(TreeNode* root, int* result) {
    if (root == NULL || result == NULL) {
        return 0;
    }
    int index = 0;
    inorderHelper(root, result, &index);
    return index;
}

void postorderHelper(TreeNode* node, int* result, int* index) {
    if (node == NULL) {
        return;
    }
    postorderHelper(node->left, result, index);
    postorderHelper(node->right, result, index);
    result[(*index)++] = node->val;
}

int postorderTraversal(TreeNode* root, int* result) {
    if (root == NULL || result == NULL) {
        return 0;
    }
    int index = 0;
    postorderHelper(root, result, &index);
    return index;
}

void printArray(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]\\n");
}

int main() {
    int result[100];
    int size;
    
    // Test Case 1: Single node tree
    TreeNode* test1 = createNode(1);
    printf("Test 1 - Preorder: ");
    size = preorderTraversal(test1, result);
    printArray(result, size);
    printf("Test 1 - Inorder: ");
    size = inorderTraversal(test1, result);
    printArray(result, size);
    printf("Test 1 - Postorder: ");
    size = postorderTraversal(test1, result);
    printArray(result, size);
    freeTree(test1);
    
    // Test Case 2: Balanced tree
    TreeNode* test2 = createNode(1);
    test2->left = createNode(2);
    test2->right = createNode(3);
    test2->left->left = createNode(4);
    test2->left->right = createNode(5);
    printf("\\nTest 2 - Preorder: ");
    size = preorderTraversal(test2, result);
    printArray(result, size);
    printf("Test 2 - Inorder: ");
    size = inorderTraversal(test2, result);
    printArray(result, size);
    printf("Test 2 - Postorder: ");
    size = postorderTraversal(test2, result);
    printArray(result, size);
    freeTree(test2);
    
    // Test Case 3: Left-skewed tree
    TreeNode* test3 = createNode(1);
    test3->left = createNode(2);
    test3->left->left = createNode(3);
    printf("\\nTest 3 - Preorder: ");
    size = preorderTraversal(test3, result);
    printArray(result, size);
    printf("Test 3 - Inorder: ");
    size = inorderTraversal(test3, result);
    printArray(result, size);
    printf("Test 3 - Postorder: ");
    size = postorderTraversal(test3, result);
    printArray(result, size);
    freeTree(test3);
    
    // Test Case 4: Right-skewed tree
    TreeNode* test4 = createNode(1);
    test4->right = createNode(2);
    test4->right->right = createNode(3);
    printf("\\nTest 4 - Preorder: ");
    size = preorderTraversal(test4, result);
    printArray(result, size);
    printf("Test 4 - Inorder: ");
    size = inorderTraversal(test4, result);
    printArray(result, size);
    printf("Test 4 - Postorder: ");
    size = postorderTraversal(test4, result);
    printArray(result, size);
    freeTree(test4);
    
    // Test Case 5: Empty tree
    TreeNode* test5 = NULL;
    printf("\\nTest 5 - Preorder: ");
    size = preorderTraversal(test5, result);
    printArray(result, size);
    printf("Test 5 - Inorder: ");
    size = inorderTraversal(test5, result);
    printArray(result, size);
    printf("Test 5 - Postorder: ");
    size = postorderTraversal(test5, result);
    printArray(result, size);
    
    return 0;
}
