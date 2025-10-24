
#include <stdio.h>
#include <stdlib.h>

typedef struct TreeNode {
    int val;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

TreeNode* createNode(int val) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if (node == NULL) return NULL;
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
    TreeNode* tree1 = createNode(1);
    tree1->right = createNode(2);
    tree1->right->left = createNode(3);
    printf("Test Case 1:\\n");
    size = preorderTraversal(tree1, result);
    printf("Preorder: "); printArray(result, size);
    size = inorderTraversal(tree1, result);
    printf("Inorder: "); printArray(result, size);
    size = postorderTraversal(tree1, result);
    printf("Postorder: "); printArray(result, size);
    
    // Test Case 2: Balanced tree
    TreeNode* tree2 = createNode(1);
    tree2->left = createNode(2);
    tree2->right = createNode(3);
    tree2->left->left = createNode(4);
    tree2->left->right = createNode(5);
    printf("\\nTest Case 2:\\n");
    size = preorderTraversal(tree2, result);
    printf("Preorder: "); printArray(result, size);
    size = inorderTraversal(tree2, result);
    printf("Inorder: "); printArray(result, size);
    size = postorderTraversal(tree2, result);
    printf("Postorder: "); printArray(result, size);
    
    // Test Case 3: Single node
    TreeNode* tree3 = createNode(42);
    printf("\\nTest Case 3:\\n");
    size = preorderTraversal(tree3, result);
    printf("Preorder: "); printArray(result, size);
    size = inorderTraversal(tree3, result);
    printf("Inorder: "); printArray(result, size);
    size = postorderTraversal(tree3, result);
    printf("Postorder: "); printArray(result, size);
    
    // Test Case 4: Empty tree
    TreeNode* tree4 = NULL;
    printf("\\nTest Case 4:\\n");
    size = preorderTraversal(tree4, result);
    printf("Preorder: "); printArray(result, size);
    size = inorderTraversal(tree4, result);
    printf("Inorder: "); printArray(result, size);
    size = postorderTraversal(tree4, result);
    printf("Postorder: "); printArray(result, size);
    
    // Test Case 5: Left skewed tree
    TreeNode* tree5 = createNode(5);
    tree5->left = createNode(4);
    tree5->left->left = createNode(3);
    tree5->left->left->left = createNode(2);
    tree5->left->left->left->left = createNode(1);
    printf("\\nTest Case 5:\\n");
    size = preorderTraversal(tree5, result);
    printf("Preorder: "); printArray(result, size);
    size = inorderTraversal(tree5, result);
    printf("Inorder: "); printArray(result, size);
    size = postorderTraversal(tree5, result);
    printf("Postorder: "); printArray(result, size);
    
    return 0;
}
