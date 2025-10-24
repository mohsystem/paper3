#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int key;
    struct Node *left;
    struct Node *right;
} Node;

// Utility function to create a new BST node
Node* createNode(int key) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        // Handle memory allocation failure
        perror("Failed to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    newNode->key = key;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

// Function to insert a new key in the BST
Node* insert(Node* node, int key) {
    if (node == NULL) {
        return createNode(key);
    }
    if (key < node->key) {
        node->left = insert(node->left, key);
    } else if (key > node->key) {
        node->right = insert(node->right, key);
    }
    return node;
}

// Function to search for a key in the BST
Node* search(Node* root, int key) {
    if (root == NULL || root->key == key) {
        return root;
    }
    if (key < root->key) {
        return search(root->left, key);
    }
    return search(root->right, key);
}

// Function to find the node with the minimum key value in a given subtree
Node* minValueNode(Node* node) {
    Node* current = node;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}

// Function to delete a key from the BST
Node* deleteNode(Node* root, int key) {
    if (root == NULL) {
        return root;
    }
    if (key < root->key) {
        root->left = deleteNode(root->left, key);
    } else if (key > root->key) {
        root->right = deleteNode(root->right, key);
    } else {
        // Node with only one child or no child
        if (root->left == NULL) {
            Node* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Node* temp = root->left;
            free(root);
            return temp;
        }
        // Node with two children
        Node* temp = minValueNode(root->right);
        root->key = temp->key;
        root->right = deleteNode(root->right, temp->key);
    }
    return root;
}

// Function to do inorder traversal of BST
void inorder(Node *root) {
    if (root != NULL) {
        inorder(root->left);
        printf("%d ", root->key);
        inorder(root->right);
    }
}

// Function to free the entire tree to prevent memory leaks
void freeTree(Node* node) {
    if (node == NULL) return;
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

int main() {
    Node *root = NULL;
    /*
     *        50
     *       /  \
     *      30   70
     *     / \   / \
     *    20 40 60  80
     */
    root = insert(root, 50);
    insert(root, 30);
    insert(root, 20);
    insert(root, 40);
    insert(root, 70);
    insert(root, 60);
    insert(root, 80);

    printf("Initial tree (inorder):\n");
    inorder(root);
    printf("\n");

    // Test Case 1: Search for an element that exists
    printf("Test Case 1: Search for 40\n");
    printf("Found 40: %s\n", search(root, 40) ? "true" : "false");

    // Test Case 2: Search for an element that does not exist
    printf("\nTest Case 2: Search for 90\n");
    printf("Found 90: %s\n", search(root, 90) ? "true" : "false");
    
    // Test Case 3: Delete a leaf node (20)
    printf("\nTest Case 3: Delete 20 (leaf node)\n");
    root = deleteNode(root, 20);
    printf("Tree after deleting 20: ");
    inorder(root);
    printf("\n");

    // Test Case 4: Delete a node with one child (30)
    printf("\nTest Case 4: Delete 30 (node with one child)\n");
    root = deleteNode(root, 30);
    printf("Tree after deleting 30: ");
    inorder(root);
    printf("\n");
    
    // Test Case 5: Delete a node with two children (50 - the root)
    printf("\nTest Case 5: Delete 50 (node with two children - root)\n");
    root = deleteNode(root, 50);
    printf("Tree after deleting 50: ");
    inorder(root);
    printf("\n");

    // Clean up all allocated memory
    freeTree(root);

    return 0;
}