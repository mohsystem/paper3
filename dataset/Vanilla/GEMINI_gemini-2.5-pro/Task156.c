#include <stdio.h>
#include <stdlib.h>

// Definition for a binary tree node.
struct TreeNode {
    int val;
    struct TreeNode *left;
    struct TreeNode *right;
};

// Helper function to create a new node
struct TreeNode* createNode(int val) {
    struct TreeNode* newNode = (struct TreeNode*)malloc(sizeof(struct TreeNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->val = val;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

// Preorder Traversal: Root -> Left -> Right
void preorderTraversal(struct TreeNode* root) {
    if (root == NULL) {
        return;
    }
    printf("%d ", root->val);
    preorderTraversal(root->left);
    preorderTraversal(root->right);
}

// Inorder Traversal: Left -> Root -> Right
void inorderTraversal(struct TreeNode* root) {
    if (root == NULL) {
        return;
    }
    inorderTraversal(root->left);
    printf("%d ", root->val);
    inorderTraversal(root->right);
}

// Postorder Traversal: Left -> Right -> Root
void postorderTraversal(struct TreeNode* root) {
    if (root == NULL) {
        return;
    }
    postorderTraversal(root->left);
    postorderTraversal(root->right);
    printf("%d ", root->val);
}

// Helper function to free the tree memory
void freeTree(struct TreeNode* node) {
    if (node == NULL) {
        return;
    }
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

int main() {
    // --- Test Case 1: Standard Tree ---
    printf("--- Test Case 1 ---\n");
    struct TreeNode* root1 = createNode(1);
    root1->left = createNode(2);
    root1->right = createNode(3);
    root1->left->left = createNode(4);
    root1->left->right = createNode(5);
    printf("Preorder:  "); preorderTraversal(root1); printf("\n");
    printf("Inorder:   "); inorderTraversal(root1); printf("\n");
    printf("Postorder: "); postorderTraversal(root1); printf("\n");
    freeTree(root1);
    printf("\n");

    // --- Test Case 2: Null Tree ---
    printf("--- Test Case 2 ---\n");
    struct TreeNode* root2 = NULL;
    printf("Preorder:  "); preorderTraversal(root2); printf("\n");
    printf("Inorder:   "); inorderTraversal(root2); printf("\n");
    printf("Postorder: "); postorderTraversal(root2); printf("\n");
    printf("\n");

    // --- Test Case 3: Single Node Tree ---
    printf("--- Test Case 3 ---\n");
    struct TreeNode* root3 = createNode(10);
    printf("Preorder:  "); preorderTraversal(root3); printf("\n");
    printf("Inorder:   "); inorderTraversal(root3); printf("\n");
    printf("Postorder: "); postorderTraversal(root3); printf("\n");
    freeTree(root3);
    printf("\n");

    // --- Test Case 4: Left-skewed Tree ---
    printf("--- Test Case 4 ---\n");
    struct TreeNode* root4 = createNode(4);
    root4->left = createNode(3);
    root4->left->left = createNode(2);
    root4->left->left->left = createNode(1);
    printf("Preorder:  "); preorderTraversal(root4); printf("\n");
    printf("Inorder:   "); inorderTraversal(root4); printf("\n");
    printf("Postorder: "); postorderTraversal(root4); printf("\n");
    freeTree(root4);
    printf("\n");

    // --- Test Case 5: Complex Tree ---
    printf("--- Test Case 5 ---\n");
    struct TreeNode* root5 = createNode(10);
    root5->left = createNode(5);
    root5->right = createNode(15);
    root5->left->left = createNode(3);
    root5->left->right = createNode(7);
    root5->right->right = createNode(18);
    printf("Preorder:  "); preorderTraversal(root5); printf("\n");
    printf("Inorder:   "); inorderTraversal(root5); printf("\n");
    printf("Postorder: "); postorderTraversal(root5); printf("\n");
    freeTree(root5);
    printf("\n");

    return 0;
}