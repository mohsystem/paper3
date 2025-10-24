
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct BinarySearchTree {
    Node* root;
} BinarySearchTree;

Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

Node* insertHelper(Node* root, int data) {
    if (root == NULL) {
        return createNode(data);
    }
    
    if (data < root->data) {
        root->left = insertHelper(root->left, data);
    } else if (data > root->data) {
        root->right = insertHelper(root->right, data);
    }
    
    return root;
}

void insert(BinarySearchTree* bst, int data) {
    bst->root = insertHelper(bst->root, data);
}

bool searchHelper(Node* root, int data) {
    if (root == NULL) {
        return false;
    }
    
    if (root->data == data) {
        return true;
    }
    
    if (data < root->data) {
        return searchHelper(root->left, data);
    } else {
        return searchHelper(root->right, data);
    }
}

bool search(BinarySearchTree* bst, int data) {
    return searchHelper(bst->root, data);
}

Node* findMin(Node* root) {
    while (root->left != NULL) {
        root = root->left;
    }
    return root;
}

Node* deleteHelper(Node* root, int data) {
    if (root == NULL) {
        return NULL;
    }
    
    if (data < root->data) {
        root->left = deleteHelper(root->left, data);
    } else if (data > root->data) {
        root->right = deleteHelper(root->right, data);
    } else {
        // Node to be deleted found
        
        // Case 1: No child or one child
        if (root->left == NULL) {
            Node* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Node* temp = root->left;
            free(root);
            return temp;
        }
        
        // Case 2: Two children
        // Find inorder successor (smallest in right subtree)
        Node* successor = findMin(root->right);
        root->data = successor->data;
        root->right = deleteHelper(root->right, successor->data);
    }
    
    return root;
}

void deleteNode(BinarySearchTree* bst, int data) {
    bst->root = deleteHelper(bst->root, data);
}

void inorderHelper(Node* root, int* result, int* index) {
    if (root != NULL) {
        inorderHelper(root->left, result, index);
        result[(*index)++] = root->data;
        inorderHelper(root->right, result, index);
    }
}

int* inorder(BinarySearchTree* bst, int* size) {
    int* result = (int*)malloc(100 * sizeof(int));
    *size = 0;
    inorderHelper(bst->root, result, size);
    return result;
}

void printArray(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]\\n");
}

BinarySearchTree* createBST() {
    BinarySearchTree* bst = (BinarySearchTree*)malloc(sizeof(BinarySearchTree));
    bst->root = NULL;
    return bst;
}

int main() {
    // Test Case 1: Basic insert and search
    printf("Test Case 1: Basic insert and search\\n");
    BinarySearchTree* bst1 = createBST();
    insert(bst1, 50);
    insert(bst1, 30);
    insert(bst1, 70);
    insert(bst1, 20);
    insert(bst1, 40);
    printf("Search 40: %s\\n", search(bst1, 40) ? "true" : "false");
    printf("Search 60: %s\\n", search(bst1, 60) ? "true" : "false");
    int size1;
    int* result1 = inorder(bst1, &size1);
    printf("Inorder: ");
    printArray(result1, size1);
    free(result1);
    printf("\\n");
    
    // Test Case 2: Delete leaf node
    printf("Test Case 2: Delete leaf node\\n");
    BinarySearchTree* bst2 = createBST();
    insert(bst2, 50);
    insert(bst2, 30);
    insert(bst2, 70);
    insert(bst2, 20);
    insert(bst2, 40);
    deleteNode(bst2, 20);
    int size2;
    int* result2 = inorder(bst2, &size2);
    printf("After deleting 20: ");
    printArray(result2, size2);
    printf("Search 20: %s\\n", search(bst2, 20) ? "true" : "false");
    free(result2);
    printf("\\n");
    
    // Test Case 3: Delete node with one child
    printf("Test Case 3: Delete node with one child\\n");
    BinarySearchTree* bst3 = createBST();
    insert(bst3, 50);
    insert(bst3, 30);
    insert(bst3, 70);
    insert(bst3, 60);
    deleteNode(bst3, 70);
    int size3;
    int* result3 = inorder(bst3, &size3);
    printf("After deleting 70: ");
    printArray(result3, size3);
    free(result3);
    printf("\\n");
    
    // Test Case 4: Delete node with two children
    printf("Test Case 4: Delete node with two children\\n");
    BinarySearchTree* bst4 = createBST();
    insert(bst4, 50);
    insert(bst4, 30);
    insert(bst4, 70);
    insert(bst4, 20);
    insert(bst4, 40);
    insert(bst4, 60);
    insert(bst4, 80);
    deleteNode(bst4, 50);
    int size4;
    int* result4 = inorder(bst4, &size4);
    printf("After deleting 50: ");
    printArray(result4, size4);
    free(result4);
    printf("\\n");
    
    // Test Case 5: Complex operations
    printf("Test Case 5: Complex operations\\n");
    BinarySearchTree* bst5 = createBST();
    int values[] = {15, 10, 20, 8, 12, 17, 25};
    for (int i = 0; i < 7; i++) {
        insert(bst5, values[i]);
    }
    int size5;
    int* result5 = inorder(bst5, &size5);
    printf("Initial tree: ");
    printArray(result5, size5);
    free(result5);
    printf("Search 12: %s\\n", search(bst5, 12) ? "true" : "false");
    deleteNode(bst5, 15);
    int size6;
    int* result6 = inorder(bst5, &size6);
    printf("After deleting 15: ");
    printArray(result6, size6);
    free(result6);
    insert(bst5, 14);
    int size7;
    int* result7 = inorder(bst5, &size7);
    printf("After inserting 14: ");
    printArray(result7, size7);
    free(result7);
    
    return 0;
}
