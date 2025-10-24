#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct Node {
    int key;
    struct Node *left;
    struct Node *right;
};

// Utility function to create a new BST node
struct Node* newNode(int item) {
    struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
    if (temp == NULL) {
        // Handle memory allocation failure
        perror("Failed to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    temp->key = item;
    temp->left = temp->right = NULL;
    return temp;
}

// A recursive function to insert a new key in BST
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

// A recursive function to search a given key in BST
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

// Helper function to find the node with minimum key value found in that tree
struct Node* minValueNode(struct Node* node) {
    struct Node* current = node;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}

// A recursive function to delete a key from BST
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

// Utility function to do inorder traversal of BST
void inorder(struct Node* root) {
    if (root != NULL) {
        inorder(root->left);
        printf("%d ", root->key);
        inorder(root->right);
    }
}

// Utility function to free the entire tree to prevent memory leaks
void freeTree(struct Node* node) {
    if (node == NULL) return;
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

int main() {
    struct Node* root = NULL;

    // --- Test Case 1: Insertion and Search ---
    printf("--- Test Case 1: Insertion and Search ---\n");
    root = insert(root, 50);
    insert(root, 30);
    insert(root, 20);
    insert(root, 40);
    insert(root, 70);
    insert(root, 60);
    insert(root, 80);
    printf("Inorder traversal: ");
    inorder(root);
    printf("\n");
    printf("Search for 60: %s\n", search(root, 60) ? "Found" : "Not Found");
    printf("Search for 90: %s\n", search(root, 90) ? "Found" : "Not Found");
    printf("\n");

    // --- Test Case 2: Deleting a leaf node (20) ---
    printf("--- Test Case 2: Deleting a leaf node (20) ---\n");
    printf("Deleting 20...\n");
    root = deleteNode(root, 20);
    printf("Inorder traversal: ");
    inorder(root);
    printf("\n\n");
    
    // --- Test Case 3: Deleting a node with one child (30) ---
    printf("--- Test Case 3: Deleting a node with one child (30) ---\n");
    printf("Deleting 30...\n");
    root = deleteNode(root, 30);
    printf("Inorder traversal: ");
    inorder(root);
    printf("\n\n");

    // --- Test Case 4: Deleting a node with two children (50) ---
    printf("--- Test Case 4: Deleting a node with two children (50) ---\n");
    printf("Deleting 50 (root)...\n");
    root = deleteNode(root, 50);
    printf("Inorder traversal: ");
    inorder(root);
    printf("\n\n");

    // --- Test Case 5: Deleting another node and searching for a deleted key ---
    printf("--- Test Case 5: Deleting another node and checking ---\n");
    printf("Deleting 70...\n");
    root = deleteNode(root, 70);
    printf("Inorder traversal: ");
    inorder(root);
    printf("\n");
    printf("Search for 50: %s\n", search(root, 50) ? "Found" : "Not Found");

    // Free all allocated memory
    freeTree(root);

    return 0;
}