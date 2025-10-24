
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Node structure for binary tree
typedef struct TreeNode {
    int value;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

// Result structure to hold traversal results
typedef struct {
    int* data;
    size_t size;
    size_t capacity;
} TraversalResult;

// Constants for security limits
#define MAX_DEPTH 10000
#define MAX_NODES 1000000
#define INITIAL_CAPACITY 16

// Create a new tree node - validates memory allocation
TreeNode* createNode(int value) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if (node == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for tree node\\n");
        return NULL;
    }
    // Initialize all fields to prevent undefined behavior
    node->value = value;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Initialize traversal result structure with bounds checking
bool initResult(TraversalResult* result) {
    if (result == NULL) {
        return false;
    }
    
    // Security: validate initial capacity is reasonable
    if (INITIAL_CAPACITY > MAX_NODES) {
        fprintf(stderr, "Error: Initial capacity exceeds maximum\\n");
        return false;
    }
    
    result->data = (int*)malloc(INITIAL_CAPACITY * sizeof(int));
    if (result->data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for result\\n");
        return false;
    }
    
    result->size = 0;
    result->capacity = INITIAL_CAPACITY;
    return true;
}

// Add value to result with bounds checking and dynamic growth
bool addToResult(TraversalResult* result, int value) {
    if (result == NULL || result->data == NULL) {
        return false;
    }
    
    // Security: prevent excessive memory usage
    if (result->size >= MAX_NODES) {
        fprintf(stderr, "Error: Maximum node count exceeded\\n");
        return false;
    }
    
    // Grow array if needed with overflow protection
    if (result->size >= result->capacity) {
        // Security: check for overflow before multiplication
        if (result->capacity > SIZE_MAX / 2) {
            fprintf(stderr, "Error: Capacity overflow prevented\\n");
            return false;
        }
        
        size_t newCapacity = result->capacity * 2;
        
        // Security: cap at MAX_NODES
        if (newCapacity > MAX_NODES) {
            newCapacity = MAX_NODES;
        }
        
        // Security: check for overflow in byte calculation
        if (newCapacity > SIZE_MAX / sizeof(int)) {
            fprintf(stderr, "Error: Size calculation overflow prevented\\n");
            return false;
        }
        
        int* newData = (int*)realloc(result->data, newCapacity * sizeof(int));
        if (newData == NULL) {
            fprintf(stderr, "Error: Failed to reallocate memory\\n");
            return false;
        }
        
        result->data = newData;
        result->capacity = newCapacity;
    }
    
    result->data[result->size++] = value;
    return true;
}

// Free result structure - ensures memory is freed exactly once
void freeResult(TraversalResult* result) {
    if (result != NULL && result->data != NULL) {
        free(result->data);
        result->data = NULL;
        result->size = 0;
        result->capacity = 0;
    }
}

// Free tree recursively with depth checking
void freeTree(TreeNode* node, int depth) {
    if (node == NULL) {
        return;
    }
    
    // Security: prevent stack overflow from excessive recursion
    if (depth > MAX_DEPTH) {
        fprintf(stderr, "Warning: Maximum depth reached during tree freeing\\n");
        return;
    }
    
    freeTree(node->left, depth + 1);
    freeTree(node->right, depth + 1);
    free(node);
}

// Preorder traversal helper with security checks
bool preorderHelper(TreeNode* node, TraversalResult* result, int depth) {
    // Security: prevent stack overflow
    if (depth > MAX_DEPTH) {
        fprintf(stderr, "Error: Maximum tree depth exceeded\\n");
        return false;
    }
    
    if (node == NULL) {
        return true;
    }
    
    // Visit root
    if (!addToResult(result, node->value)) {
        return false;
    }
    
    // Visit left subtree
    if (!preorderHelper(node->left, result, depth + 1)) {
        return false;
    }
    
    // Visit right subtree
    if (!preorderHelper(node->right, result, depth + 1)) {
        return false;
    }
    
    return true;
}

// Inorder traversal helper with security checks
bool inorderHelper(TreeNode* node, TraversalResult* result, int depth) {
    if (depth > MAX_DEPTH) {
        fprintf(stderr, "Error: Maximum tree depth exceeded\\n");
        return false;
    }
    
    if (node == NULL) {
        return true;
    }
    
    // Visit left subtree
    if (!inorderHelper(node->left, result, depth + 1)) {
        return false;
    }
    
    // Visit root
    if (!addToResult(result, node->value)) {
        return false;
    }
    
    // Visit right subtree
    if (!inorderHelper(node->right, result, depth + 1)) {
        return false;
    }
    
    return true;
}

// Postorder traversal helper with security checks
bool postorderHelper(TreeNode* node, TraversalResult* result, int depth) {
    if (depth > MAX_DEPTH) {
        fprintf(stderr, "Error: Maximum tree depth exceeded\\n");
        return false;
    }
    
    if (node == NULL) {
        return true;
    }
    
    // Visit left subtree
    if (!postorderHelper(node->left, result, depth + 1)) {
        return false;
    }
    
    // Visit right subtree
    if (!postorderHelper(node->right, result, depth + 1)) {
        return false;
    }
    
    // Visit root
    if (!addToResult(result, node->value)) {
        return false;
    }
    
    return true;
}

// Public preorder traversal function
bool preorder(TreeNode* root, TraversalResult* result) {
    if (result == NULL) {
        return false;
    }
    
    if (!initResult(result)) {
        return false;
    }
    
    if (!preorderHelper(root, result, 0)) {
        freeResult(result);
        return false;
    }
    
    return true;
}

// Public inorder traversal function
bool inorder(TreeNode* root, TraversalResult* result) {
    if (result == NULL) {
        return false;
    }
    
    if (!initResult(result)) {
        return false;
    }
    
    if (!inorderHelper(root, result, 0)) {
        freeResult(result);
        return false;
    }
    
    return true;
}

// Public postorder traversal function
bool postorder(TreeNode* root, TraversalResult* result) {
    if (result == NULL) {
        return false;
    }
    
    if (!initResult(result)) {
        return false;
    }
    
    if (!postorderHelper(root, result, 0)) {
        freeResult(result);
        return false;
    }
    
    return true;
}

// Print traversal result with bounds checking
void printResult(const TraversalResult* result) {
    if (result == NULL || result->data == NULL) {
        printf("[]\\n");
        return;
    }
    
    printf("[");
    for (size_t i = 0; i < result->size; i++) {
        printf("%d", result->data[i]);
        if (i < result->size - 1) {
            printf(", ");
        }
    }
    printf("]\\n");
}

int main(void) {
    TraversalResult result;
    
    // Test case 1: Simple tree with 3 nodes
    printf("Test Case 1: Simple tree (1, 2, 3)\\n");
    TreeNode* root1 = createNode(1);
    if (root1 == NULL) {
        return 1;
    }
    root1->left = createNode(2);
    root1->right = createNode(3);
    
    if (root1->left == NULL || root1->right == NULL) {
        freeTree(root1, 0);
        return 1;
    }
    
    if (preorder(root1, &result)) {
        printf("Preorder:  ");
        printResult(&result);
        freeResult(&result);
    }
    
    if (inorder(root1, &result)) {
        printf("Inorder:   ");
        printResult(&result);
        freeResult(&result);
    }
    
    if (postorder(root1, &result)) {
        printf("Postorder: ");
        printResult(&result);
        freeResult(&result);
    }
    printf("\\n");
    freeTree(root1, 0);
    
    // Test case 2: Balanced tree
    printf("Test Case 2: Balanced tree\\n");
    TreeNode* root2 = createNode(4);
    if (root2 == NULL) {
        return 1;
    }
    root2->left = createNode(2);
    root2->right = createNode(6);
    
    if (root2->left == NULL || root2->right == NULL) {
        freeTree(root2, 0);
        return 1;
    }
    
    root2->left->left = createNode(1);
    root2->left->right = createNode(3);
    root2->right->left = createNode(5);
    root2->right->right = createNode(7);
    
    if (root2->left->left == NULL || root2->left->right == NULL ||
        root2->right->left == NULL || root2->right->right == NULL) {
        freeTree(root2, 0);
        return 1;
    }
    
    if (preorder(root2, &result)) {
        printf("Preorder:  ");
        printResult(&result);
        freeResult(&result);
    }
    
    if (inorder(root2, &result)) {
        printf("Inorder:   ");
        printResult(&result);
        freeResult(&result);
    }
    
    if (postorder(root2, &result)) {
        printf("Postorder: ");
        printResult(&result);
        freeResult(&result);
    }
    printf("\\n");
    freeTree(root2, 0);
    
    // Test case 3: Left-skewed tree
    printf("Test Case 3: Left-skewed tree\\n");
    TreeNode* root3 = createNode(5);
    if (root3 == NULL) {
        return 1;
    }
    root3->left = createNode(4);
    if (root3->left == NULL) {
        freeTree(root3, 0);
        return 1;
    }
    root3->left->left = createNode(3);
    if (root3->left->left == NULL) {
        freeTree(root3, 0);
        return 1;
    }
    
    if (preorder(root3, &result)) {
        printf("Preorder:  ");
        printResult(&result);
        freeResult(&result);
    }
    
    if (inorder(root3, &result)) {
        printf("Inorder:   ");
        printResult(&result);
        freeResult(&result);
    }
    
    if (postorder(root3, &result)) {
        printf("Postorder: ");
        printResult(&result);
        freeResult(&result);
    }
    printf("\\n");
    freeTree(root3, 0);
    
    // Test case 4: Right-skewed tree
    printf("Test Case 4: Right-skewed tree\\n");
    TreeNode* root4 = createNode(1);
    if (root4 == NULL) {
        return 1;
    }
    root4->right = createNode(2);
    if (root4->right == NULL) {
        freeTree(root4, 0);
        return 1;
    }
    root4->right->right = createNode(3);
    if (root4->right->right == NULL) {
        freeTree(root4, 0);
        return 1;
    }
    
    if (preorder(root4, &result)) {
        printf("Preorder:  ");
        printResult(&result);
        freeResult(&result);
    }
    
    if (inorder(root4, &result)) {
        printf("Inorder:   ");
        printResult(&result);
        freeResult(&result);
    }
    
    if (postorder(root4, &result)) {
        printf("Postorder: ");
        printResult(&result);
        freeResult(&result);
    }
    printf("\\n");
    freeTree(root4, 0);
    
    // Test case 5: Single node tree
    printf("Test Case 5: Single node tree\\n");
    TreeNode* root5 = createNode(42);
    if (root5 == NULL) {
        return 1;
    }
    
    if (preorder(root5, &result)) {
        printf("Preorder:  ");
        printResult(&result);
        freeResult(&result);
    }
    
    if (inorder(root5, &result)) {
        printf("Inorder:   ");
        printResult(&result);
        freeResult(&result);
    }
    
    if (postorder(root5, &result)) {
        printf("Postorder: ");
        printResult(&result);
        freeResult(&result);
    }
    freeTree(root5, 0);
    
    return 0;
}
