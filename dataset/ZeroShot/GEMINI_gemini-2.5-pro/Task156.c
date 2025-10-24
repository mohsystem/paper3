#include <stdio.h>
#include <stdlib.h>

// Node structure for the binary tree
typedef struct Node {
    int data;
    struct Node* left;
    struct Node* right;
} Node;

// Utility function to create a new node
Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }
    newNode->data = data;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

// Utility function to free the tree memory to prevent leaks
void freeTree(Node* node) {
    if (node == NULL) {
        return;
    }
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

/**
 * Performs a preorder traversal of the binary tree.
 * Root -> Left -> Right
 * @param node The root node of the tree/subtree.
 */
void preorderTraversal(Node* node) {
    if (node == NULL) {
        return;
    }
    printf("%d ", node->data);
    preorderTraversal(node->left);
    preorderTraversal(node->right);
}

/**
 * Performs an inorder traversal of the binary tree.
 * Left -> Root -> Right
 * @param node The root node of the tree/subtree.
 */
void inorderTraversal(Node* node) {
    if (node == NULL) {
        return;
    }
    inorderTraversal(node->left);
    printf("%d ", node->data);
    inorderTraversal(node->right);
}

/**
 * Performs a postorder traversal of the binary tree.
 * Left -> Right -> Root
 * @param node The root node of the tree/subtree.
 */
void postorderTraversal(Node* node) {
    if (node == NULL) {
        return;
    }
    postorderTraversal(node->left);
    postorderTraversal(node->right);
    printf("%d ", node->data);
}


int main() {
    // Test Case 1: A complete binary tree
    printf("--- Test Case 1: Complete Binary Tree ---\n");
    Node* root1 = createNode(1);
    if(root1) {
        root1->left = createNode(2);
        root1->right = createNode(3);
        if(root1->left) {
            root1->left->left = createNode(4);
            root1->left->right = createNode(5);
        }
        if(root1->right) {
            root1->right->left = createNode(6);
            root1->right->right = createNode(7);
        }
    }
    printf("Preorder:  ");
    preorderTraversal(root1);
    printf("\n");
    printf("Inorder:   ");
    inorderTraversal(root1);
    printf("\n");
    printf("Postorder: ");
    postorderTraversal(root1);
    printf("\n\n");
    freeTree(root1);

    // Test Case 2: A right-skewed tree
    printf("--- Test Case 2: Right-Skewed Tree ---\n");
    Node* root2 = createNode(10);
    if(root2) {
        root2->right = createNode(20);
        if(root2->right) {
            root2->right->right = createNode(30);
            if(root2->right->right) {
                root2->right->right->right = createNode(40);
            }
        }
    }
    printf("Preorder:  ");
    preorderTraversal(root2);
    printf("\n");
    printf("Inorder:   ");
    inorderTraversal(root2);
    printf("\n");
    printf("Postorder: ");
    postorderTraversal(root2);
    printf("\n\n");
    freeTree(root2);
    
    // Test Case 3: A left-skewed tree
    printf("--- Test Case 3: Left-Skewed Tree ---\n");
    Node* root3 = createNode(50);
    if(root3) {
        root3->left = createNode(40);
        if(root3->left) {
            root3->left->left = createNode(30);
            if(root3->left->left) {
                root3->left->left->left = createNode(20);
            }
        }
    }
    printf("Preorder:  ");
    preorderTraversal(root3);
    printf("\n");
    printf("Inorder:   ");
    inorderTraversal(root3);
    printf("\n");
    printf("Postorder: ");
    postorderTraversal(root3);
    printf("\n\n");
    freeTree(root3);

    // Test Case 4: An empty tree (NULL root)
    printf("--- Test Case 4: Empty Tree ---\n");
    Node* root4 = NULL;
    printf("Preorder:  (empty)\n");
    printf("Inorder:   (empty)\n");
    printf("Postorder: (empty)\n\n");
    freeTree(root4);

    // Test Case 5: A tree with only one node
    printf("--- Test Case 5: Single Node Tree ---\n");
    Node* root5 = createNode(100);
    printf("Preorder:  ");
    preorderTraversal(root5);
    printf("\n");
    printf("Inorder:   ");
    inorderTraversal(root5);
    printf("\n");
    printf("Postorder: ");
    postorderTraversal(root5);
    printf("\n\n");
    freeTree(root5);

    return 0;
}