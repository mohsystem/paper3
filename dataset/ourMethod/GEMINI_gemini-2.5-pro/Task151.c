#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Node structure for the BST
struct Node {
    int key;
    struct Node *left;
    struct Node *right;
};

// Function to create a new BST node
struct Node* createNode(int item) {
    struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
    if (temp == NULL) {
        perror("Failed to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    temp->key = item;
    temp->left = temp->right = NULL;
    return temp;
}

// Function to insert a new key in BST
struct Node* insert(struct Node* node, int key) {
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

// Function to search a given key in a BST
bool search(struct Node* root, int key) {
    if (root == NULL) {
        return false;
    }
    if (root->key == key) {
        return true;
    }
    if (key < root->key) {
        return search(root->left, key);
    }
    return search(root->right, key);
}

// Helper function to find the minimum value node in a subtree
struct Node* minValueNode(struct Node* node) {
    struct Node* current = node;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}

// Function to delete a key from a BST
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

// Function to free the entire tree
void freeTree(struct Node* node) {
    if (node == NULL) {
        return;
    }
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

int main() {
    struct Node* root = NULL;

    // Test Case 1: Insert and search
    printf("Test Case 1: Insert and Search\n");
    root = insert(root, 50);
    root = insert(root, 30);
    root = insert(root, 70);
    printf("Search for 30: %s\n", search(root, 30) ? "true" : "false");
    printf("Search for 100: %s\n", search(root, 100) ? "true" : "false");
    printf("---\n");

    // Test Case 2: Delete a leaf node
    printf("Test Case 2: Delete a leaf node\n");
    root = insert(root, 20);
    printf("Search for 20 before delete: %s\n", search(root, 20) ? "true" : "false");
    root = deleteNode(root, 20);
    printf("Search for 20 after delete: %s\n", search(root, 20) ? "true" : "false");
    printf("---\n");

    // Test Case 3: Delete a node with one child
    printf("Test Case 3: Delete a node with one child\n");
    root = insert(root, 40);
    printf("Search for 30 before delete: %s\n", search(root, 30) ? "true" : "false");
    printf("Search for 40 before delete: %s\n", search(root, 40) ? "true" : "false");
    root = deleteNode(root, 30);
    printf("Search for 30 after delete: %s\n", search(root, 30) ? "true" : "false");
    printf("Search for 40 after delete: %s\n", search(root, 40) ? "true" : "false");
    printf("---\n");

    // Test Case 4: Delete a node with two children
    printf("Test Case 4: Delete a node with two children\n");
    root = insert(root, 60);
    root = insert(root, 80);
    printf("Search for 70 before delete: %s\n", search(root, 70) ? "true" : "false");
    root = deleteNode(root, 70);
    printf("Search for 70 after delete: %s\n", search(root, 70) ? "true" : "false");
    printf("Search for 80 (new subtree root): %s\n", search(root, 80) ? "true" : "false");
    printf("---\n");

    // Test Case 5: Delete root node
    printf("Test Case 5: Delete root node\n");
    printf("Search for 50 before delete: %s\n", search(root, 50) ? "true" : "false");
    root = deleteNode(root, 50);
    printf("Search for 50 after delete: %s\n", search(root, 50) ? "true" : "false");
    printf("Search for 60 (new root): %s\n", search(root, 60) ? "true" : "false");
    printf("---\n");

    freeTree(root);
    return 0;
}