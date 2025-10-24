#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Node structure
struct Node {
    int key;
    struct Node *left, *right;
};

// Create a new BST node
struct Node* newNode(int item) {
    struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
    if (temp == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    temp->key = item;
    temp->left = temp->right = NULL;
    return temp;
}

// Search for a key in the BST
bool search(struct Node* root, int key) {
    if (root == NULL) {
        return false;
    }
    if (root->key == key) {
        return true;
    }
    if (root->key < key) {
        return search(root->right, key);
    }
    return search(root->left, key);
}

// Insert a new key in the BST
struct Node* insert(struct Node* node, int key) {
    if (node == NULL) {
        return newNode(key);
    }
    if (key < node->key) {
        node->left = insert(node->left, key);
    } else if (key > node->key) {
        node->right = insert(node->right, key);
    }
    return node;
}

// Find the node with the minimum key value in a given subtree
struct Node* minValueNode(struct Node* node) {
    struct Node* current = node;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}

// Delete a key from the BST
struct Node* deleteNode(struct Node* root, int key) {
    if (root == NULL) {
        return root;
    }
    if (key < root->key) {
        root->left = deleteNode(root->left, key);
    } else if (key > root->key) {
        root->right = deleteNode(root->right, key);
    } else {
        if (root->left == NULL) {
            struct Node* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            struct Node* temp = root->left;
            free(root);
            return temp;
        }
        struct Node* temp = minValueNode(root->right);
        root->key = temp->key;
        root->right = deleteNode(root->right, temp->key);
    }
    return root;
}

// Inorder traversal of the BST
void inorder(struct Node* root) {
    if (root != NULL) {
        inorder(root->left);
        printf("%d ", root->key);
        inorder(root->right);
    }
}

// Free the entire tree
void freeTree(struct Node* node) {
    if (node == NULL) return;
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

int main() {
    struct Node* root = NULL;

    // Test Case 1: Insertion
    printf("--- Test Case 1: Insertion ---\n");
    int keysToInsert[] = {50, 30, 20, 40, 70, 60, 80};
    int n = sizeof(keysToInsert)/sizeof(keysToInsert[0]);
    for(int i = 0; i < n; i++) {
        root = insert(root, keysToInsert[i]);
    }
    printf("Inorder traversal of the initial BST: ");
    inorder(root);
    printf("\n");

    // Test Case 2: Search
    printf("\n--- Test Case 2: Search ---\n");
    printf("Search for 60: %s\n", search(root, 60) ? "Found" : "Not Found");
    printf("Search for 90: %s\n", search(root, 90) ? "Found" : "Not Found");

    // Test Case 3: Delete a leaf node
    printf("\n--- Test Case 3: Delete a leaf node (20) ---\n");
    root = deleteNode(root, 20);
    printf("Inorder traversal after deleting 20: ");
    inorder(root);
    printf("\n");

    // Test Case 4: Delete a node with one child
    printf("\n--- Test Case 4: Delete a node with one child (30) ---\n");
    root = deleteNode(root, 30);
    printf("Inorder traversal after deleting 30: ");
    inorder(root);
    printf("\n");

    // Test Case 5: Delete a node with two children
    printf("\n--- Test Case 5: Delete a node with two children (50) ---\n");
    root = deleteNode(root, 50);
    printf("Inorder traversal after deleting 50: ");
    inorder(root);
    printf("\n");

    // Free the allocated memory for the tree
    freeTree(root);

    return 0;
}