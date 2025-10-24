#include <stdio.h>
#include <stdlib.h>

typedef struct TreeNode {
    int val;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

// Function to create a new tree node
TreeNode* createNode(int val) {
    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
    if (newNode == NULL) {
        // Handle malloc failure
        perror("Failed to allocate memory for a new node");
        exit(EXIT_FAILURE);
    }
    newNode->val = val;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

// Function to free the entire tree to prevent memory leaks
void freeTree(TreeNode* root) {
    if (root == NULL) {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

// Preorder traversal: Root -> Left -> Right
void preorderTraversal(TreeNode* root) {
    if (root == NULL) {
        return;
    }
    printf("%d ", root->val);
    preorderTraversal(root->left);
    preorderTraversal(root->right);
}

// Inorder traversal: Left -> Root -> Right
void inorderTraversal(TreeNode* root) {
    if (root == NULL) {
        return;
    }
    inorderTraversal(root->left);
    printf("%d ", root->val);
    inorderTraversal(root->right);
}

// Postorder traversal: Left -> Right -> Root
void postorderTraversal(TreeNode* root) {
    if (root == NULL) {
        return;
    }
    postorderTraversal(root->left);
    postorderTraversal(root->right);
    printf("%d ", root->val);
}

void runTestCase(const char* name, TreeNode* root) {
    printf("--- %s ---\n", name);
    
    printf("Preorder:  [");
    preorderTraversal(root);
    printf("]\n");

    printf("Inorder:   [");
    inorderTraversal(root);
    printf("]\n");

    printf("Postorder: [");
    postorderTraversal(root);
    printf("]\n\n");
}

int main() {
    // Test Case 1: A balanced binary tree
    TreeNode* root1 = createNode(4);
    root1->left = createNode(2);
    root1->right = createNode(7);
    root1->left->left = createNode(1);
    root1->left->right = createNode(3);
    root1->right->left = createNode(6);
    root1->right->right = createNode(9);
    runTestCase("Test Case 1: Balanced Tree", root1);
    freeTree(root1);

    // Test Case 2: A left-skewed tree
    TreeNode* root2 = createNode(3);
    root2->left = createNode(2);
    root2->left->left = createNode(1);
    runTestCase("Test Case 2: Left-Skewed Tree", root2);
    freeTree(root2);

    // Test Case 3: A right-skewed tree
    TreeNode* root3 = createNode(1);
    root3->right = createNode(2);
    root3->right->right = createNode(3);
    runTestCase("Test Case 3: Right-Skewed Tree", root3);
    freeTree(root3);
    
    // Test Case 4: A single node tree
    TreeNode* root4 = createNode(5);
    runTestCase("Test Case 4: Single Node Tree", root4);
    freeTree(root4);

    // Test Case 5: An empty tree
    TreeNode* root5 = NULL;
    runTestCase("Test Case 5: Empty Tree", root5);
    // No need to free root5 as it is NULL

    return 0;
}