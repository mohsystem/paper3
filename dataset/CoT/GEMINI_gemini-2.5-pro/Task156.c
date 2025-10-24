#include <stdio.h>
#include <stdlib.h>

// Node structure for the binary tree
struct Node {
    int data;
    struct Node* left;
    struct Node* right;
};

// Utility function to create a new node
struct Node* createNode(int data) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    newNode->data = data;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

// Preorder traversal: Root -> Left -> Right
void preorderTraversal(struct Node* node) {
    if (node == NULL) {
        return;
    }
    printf("%d ", node->data);
    preorderTraversal(node->left);
    preorderTraversal(node->right);
}

// Inorder traversal: Left -> Root -> Right
void inorderTraversal(struct Node* node) {
    if (node == NULL) {
        return;
    }
    inorderTraversal(node->left);
    printf("%d ", node->data);
    inorderTraversal(node->right);
}

// Postorder traversal: Left -> Right -> Root
void postorderTraversal(struct Node* node) {
    if (node == NULL) {
        return;
    }
    postorderTraversal(node->left);
    postorderTraversal(node->right);
    printf("%d ", node->data);
}

// Function to free the entire tree to prevent memory leaks
void freeTree(struct Node* node) {
    if (node == NULL) {
        return;
    }
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

int main() {
    // --- Test Case 1: A standard binary tree ---
    printf("--- Test Case 1: Standard Tree ---\n");
    struct Node* root1 = createNode(4);
    root1->left = createNode(2);
    root1->right = createNode(5);
    root1->left->left = createNode(1);
    root1->left->right = createNode(3);

    printf("Preorder: "); preorderTraversal(root1); printf("\n");
    printf("Inorder:  "); inorderTraversal(root1); printf("\n");
    printf("Postorder:"); postorderTraversal(root1); printf("\n\n");
    freeTree(root1);

    // --- Test Case 2: A right-skewed tree ---
    printf("--- Test Case 2: Right-Skewed Tree ---\n");
    struct Node* root2 = createNode(1);
    root2->right = createNode(2);
    root2->right->right = createNode(3);

    printf("Preorder: "); preorderTraversal(root2); printf("\n");
    printf("Inorder:  "); inorderTraversal(root2); printf("\n");
    printf("Postorder:"); postorderTraversal(root2); printf("\n\n");
    freeTree(root2);

    // --- Test Case 3: A left-skewed tree ---
    printf("--- Test Case 3: Left-Skewed Tree ---\n");
    struct Node* root3 = createNode(3);
    root3->left = createNode(2);
    root3->left->left = createNode(1);

    printf("Preorder: "); preorderTraversal(root3); printf("\n");
    printf("Inorder:  "); inorderTraversal(root3); printf("\n");
    printf("Postorder:"); postorderTraversal(root3); printf("\n\n");
    freeTree(root3);

    // --- Test Case 4: A tree with only a root node ---
    printf("--- Test Case 4: Single Node Tree ---\n");
    struct Node* root4 = createNode(10);

    printf("Preorder: "); preorderTraversal(root4); printf("\n");
    printf("Inorder:  "); inorderTraversal(root4); printf("\n");
    printf("Postorder:"); postorderTraversal(root4); printf("\n\n");
    freeTree(root4);

    // --- Test Case 5: An empty tree (NULL root) ---
    printf("--- Test Case 5: Empty Tree ---\n");
    struct Node* root5 = NULL;

    printf("Preorder: "); preorderTraversal(root5); printf("\n");
    printf("Inorder:  "); inorderTraversal(root5); printf("\n");
    printf("Postorder:"); postorderTraversal(root5); printf("\n");

    return 0;
}