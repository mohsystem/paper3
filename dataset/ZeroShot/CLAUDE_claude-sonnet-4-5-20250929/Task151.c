
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct BST {
    Node* root;
} BST;

Node* createNode(int value) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = value;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

Node* insertHelper(Node* node, int value) {
    if (node == NULL) {
        return createNode(value);
    }
    
    if (value < node->data) {
        node->left = insertHelper(node->left, value);
    } else if (value > node->data) {
        node->right = insertHelper(node->right, value);
    }
    
    return node;
}

void insert(BST* bst, int value) {
    bst->root = insertHelper(bst->root, value);
}

Node* searchHelper(Node* node, int value) {
    if (node == NULL || node->data == value) {
        return node;
    }
    
    if (value < node->data) {
        return searchHelper(node->left, value);
    }
    
    return searchHelper(node->right, value);
}

bool search(BST* bst, int value) {
    return searchHelper(bst->root, value) != NULL;
}

Node* findMin(Node* node) {
    while (node->left != NULL) {
        node = node->left;
    }
    return node;
}

Node* deleteHelper(Node* node, int value) {
    if (node == NULL) {
        return NULL;
    }
    
    if (value < node->data) {
        node->left = deleteHelper(node->left, value);
    } else if (value > node->data) {
        node->right = deleteHelper(node->right, value);
    } else {
        if (node->left == NULL) {
            Node* temp = node->right;
            free(node);
            return temp;
        } else if (node->right == NULL) {
            Node* temp = node->left;
            free(node);
            return temp;
        }
        
        Node* minNode = findMin(node->right);
        node->data = minNode->data;
        node->right = deleteHelper(node->right, minNode->data);
    }
    
    return node;
}

void deleteNode(BST* bst, int value) {
    bst->root = deleteHelper(bst->root, value);
}

void inorderHelper(Node* node) {
    if (node != NULL) {
        inorderHelper(node->left);
        printf("%d ", node->data);
        inorderHelper(node->right);
    }
}

void inorder(BST* bst) {
    inorderHelper(bst->root);
    printf("\\n");
}

BST* createBST() {
    BST* bst = (BST*)malloc(sizeof(BST));
    bst->root = NULL;
    return bst;
}

int main() {
    // Test Case 1: Basic insert and search
    printf("Test Case 1: Basic insert and search\\n");
    BST* bst1 = createBST();
    insert(bst1, 50);
    insert(bst1, 30);
    insert(bst1, 70);
    insert(bst1, 20);
    insert(bst1, 40);
    printf("Inorder: ");
    inorder(bst1);
    printf("Search 40: %s\\n", search(bst1, 40) ? "true" : "false");
    printf("Search 60: %s\\n", search(bst1, 60) ? "true" : "false");
    printf("\\n");
    
    // Test Case 2: Delete leaf node
    printf("Test Case 2: Delete leaf node\\n");
    BST* bst2 = createBST();
    insert(bst2, 50);
    insert(bst2, 30);
    insert(bst2, 70);
    insert(bst2, 20);
    printf("Before delete: ");
    inorder(bst2);
    deleteNode(bst2, 20);
    printf("After delete 20: ");
    inorder(bst2);
    printf("\\n");
    
    // Test Case 3: Delete node with one child
    printf("Test Case 3: Delete node with one child\\n");
    BST* bst3 = createBST();
    insert(bst3, 50);
    insert(bst3, 30);
    insert(bst3, 70);
    insert(bst3, 60);
    printf("Before delete: ");
    inorder(bst3);
    deleteNode(bst3, 70);
    printf("After delete 70: ");
    inorder(bst3);
    printf("\\n");
    
    // Test Case 4: Delete node with two children
    printf("Test Case 4: Delete node with two children\\n");
    BST* bst4 = createBST();
    insert(bst4, 50);
    insert(bst4, 30);
    insert(bst4, 70);
    insert(bst4, 20);
    insert(bst4, 40);
    insert(bst4, 60);
    insert(bst4, 80);
    printf("Before delete: ");
    inorder(bst4);
    deleteNode(bst4, 50);
    printf("After delete 50: ");
    inorder(bst4);
    printf("\\n");
    
    // Test Case 5: Multiple operations
    printf("Test Case 5: Multiple operations\\n");
    BST* bst5 = createBST();
    insert(bst5, 15);
    insert(bst5, 10);
    insert(bst5, 20);
    insert(bst5, 8);
    insert(bst5, 12);
    insert(bst5, 17);
    insert(bst5, 25);
    printf("Initial tree: ");
    inorder(bst5);
    printf("Search 12: %s\\n", search(bst5, 12) ? "true" : "false");
    deleteNode(bst5, 15);
    printf("After delete 15: ");
    inorder(bst5);
    insert(bst5, 18);
    printf("After insert 18: ");
    inorder(bst5);
    
    return 0;
}
