
#include <stdio.h>
#include <stdlib.h>

typedef struct TreeNode {
    int val;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

TreeNode* createNode(int val) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->val = val;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void preorderHelper(TreeNode* node, int* result, int* index) {
    if (node == NULL) return;
    result[(*index)++] = node->val;
    preorderHelper(node->left, result, index);
    preorderHelper(node->right, result, index);
}

int preorderTraversal(TreeNode* root, int* result) {
    int index = 0;
    preorderHelper(root, result, &index);
    return index;
}

void inorderHelper(TreeNode* node, int* result, int* index) {
    if (node == NULL) return;
    inorderHelper(node->left, result, index);
    result[(*index)++] = node->val;
    inorderHelper(node->right, result, index);
}

int inorderTraversal(TreeNode* root, int* result) {
    int index = 0;
    inorderHelper(root, result, &index);
    return index;
}

void postorderHelper(TreeNode* node, int* result, int* index) {
    if (node == NULL) return;
    postorderHelper(node->left, result, index);
    postorderHelper(node->right, result, index);
    result[(*index)++] = node->val;
}

int postorderTraversal(TreeNode* root, int* result) {
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
    
    // Test Case 1: Simple tree
    TreeNode* root1 = createNode(1);
    root1->left = createNode(2);
    root1->right = createNode(3);
    printf("Test Case 1:\\n");
    size = preorderTraversal(root1, result);
    printf("Preorder: "); printArray(result, size);
    size = inorderTraversal(root1, result);
    printf("Inorder: "); printArray(result, size);
    size = postorderTraversal(root1, result);
    printf("Postorder: "); printArray(result, size);
    printf("\\n");
    
    // Test Case 2: Left-skewed tree
    TreeNode* root2 = createNode(1);
    root2->left = createNode(2);
    root2->left->left = createNode(3);
    printf("Test Case 2:\\n");
    size = preorderTraversal(root2, result);
    printf("Preorder: "); printArray(result, size);
    size = inorderTraversal(root2, result);
    printf("Inorder: "); printArray(result, size);
    size = postorderTraversal(root2, result);
    printf("Postorder: "); printArray(result, size);
    printf("\\n");
    
    // Test Case 3: Right-skewed tree
    TreeNode* root3 = createNode(1);
    root3->right = createNode(2);
    root3->right->right = createNode(3);
    printf("Test Case 3:\\n");
    size = preorderTraversal(root3, result);
    printf("Preorder: "); printArray(result, size);
    size = inorderTraversal(root3, result);
    printf("Inorder: "); printArray(result, size);
    size = postorderTraversal(root3, result);
    printf("Postorder: "); printArray(result, size);
    printf("\\n");
    
    // Test Case 4: Complete binary tree
    TreeNode* root4 = createNode(1);
    root4->left = createNode(2);
    root4->right = createNode(3);
    root4->left->left = createNode(4);
    root4->left->right = createNode(5);
    root4->right->left = createNode(6);
    root4->right->right = createNode(7);
    printf("Test Case 4:\\n");
    size = preorderTraversal(root4, result);
    printf("Preorder: "); printArray(result, size);
    size = inorderTraversal(root4, result);
    printf("Inorder: "); printArray(result, size);
    size = postorderTraversal(root4, result);
    printf("Postorder: "); printArray(result, size);
    printf("\\n");
    
    // Test Case 5: Empty tree
    TreeNode* root5 = NULL;
    printf("Test Case 5:\\n");
    size = preorderTraversal(root5, result);
    printf("Preorder: "); printArray(result, size);
    size = inorderTraversal(root5, result);
    printf("Inorder: "); printArray(result, size);
    size = postorderTraversal(root5, result);
    printf("Postorder: "); printArray(result, size);
    
    return 0;
}
