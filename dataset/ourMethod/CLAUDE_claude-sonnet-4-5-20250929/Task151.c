
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

// Maximum nodes to prevent excessive memory allocation
#define MAX_NODES 1000000

typedef struct Node {
    int value;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct BST {
    Node* root;
    size_t node_count;
} BST;

// Securely clear sensitive memory before freeing
static void secure_clear(void* ptr, size_t size) {
    if (ptr == NULL || size == 0) {
        return;
    }
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (size--) {
        *p++ = 0;
    }
}

// Create a new node with bounds checking
static Node* createNode(int value) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    // Initialize all fields to prevent uninitialized memory access
    node->value = value;
    node->left = NULL;
    node->right = NULL;
    
    return node;
}

// Initialize BST
BST* bst_create(void) {
    BST* bst = (BST*)malloc(sizeof(BST));
    if (bst == NULL) {
        fprintf(stderr, "BST allocation failed\\n");
        return NULL;
    }
    
    bst->root = NULL;
    bst->node_count = 0;
    
    return bst;
}

// Helper function to insert recursively
static Node* insertHelper(BST* bst, Node* node, int value, bool* success) {
    if (bst == NULL || success == NULL) {
        return NULL;
    }
    
    if (node == NULL) {
        // Check node count limit to prevent resource exhaustion
        if (bst->node_count >= MAX_NODES) {
            fprintf(stderr, "Maximum tree size exceeded\\n");
            *success = false;
            return NULL;
        }
        
        Node* newNode = createNode(value);
        if (newNode != NULL) {
            bst->node_count++;
            *success = true;
        } else {
            *success = false;
        }
        return newNode;
    }
    
    if (value < node->value) {
        node->left = insertHelper(bst, node->left, value, success);
    } else if (value > node->value) {
        node->right = insertHelper(bst, node->right, value, success);
    } else {
        // Value already exists
        *success = true;
    }
    
    return node;
}

// Insert value into BST
bool bst_insert(BST* bst, int value) {
    if (bst == NULL) {
        fprintf(stderr, "Invalid BST pointer\\n");
        return false;
    }
    
    bool success = false;
    bst->root = insertHelper(bst, bst->root, value, &success);
    return success;
}

// Helper function to search recursively
static bool searchHelper(const Node* node, int value) {
    if (node == NULL) {
        return false;
    }
    
    if (value == node->value) {
        return true;
    } else if (value < node->value) {
        return searchHelper(node->left, value);
    } else {
        return searchHelper(node->right, value);
    }
}

// Search for value in BST
bool bst_search(const BST* bst, int value) {
    if (bst == NULL) {
        return false;
    }
    
    return searchHelper(bst->root, value);
}

// Find minimum value node in subtree
static Node* findMin(Node* node) {
    if (node == NULL) {
        return NULL;
    }
    
    while (node->left != NULL) {
        node = node->left;
    }
    
    return node;
}

// Helper function to delete recursively
static Node* deleteHelper(BST* bst, Node* node, int value, bool* deleted) {
    if (bst == NULL || deleted == NULL) {
        return NULL;
    }
    
    if (node == NULL) {
        *deleted = false;
        return NULL;
    }
    
    if (value < node->value) {
        node->left = deleteHelper(bst, node->left, value, deleted);
    } else if (value > node->value) {
        node->right = deleteHelper(bst, node->right, value, deleted);
    } else {
        // Node found, perform deletion
        *deleted = true;
        if (bst->node_count > 0) {
            bst->node_count--;
        }
        
        // Case 1: No left child
        if (node->left == NULL) {
            Node* temp = node->right;
            secure_clear(node, sizeof(Node));
            free(node);
            return temp;
        }
        // Case 2: No right child
        if (node->right == NULL) {
            Node* temp = node->left;
            secure_clear(node, sizeof(Node));
            free(node);
            return temp;
        }
        
        // Case 3: Two children
        Node* successor = findMin(node->right);
        if (successor != NULL) {
            node->value = successor->value;
            bool temp_deleted = false;
            node->right = deleteHelper(bst, node->right, successor->value, &temp_deleted);
        }
    }
    
    return node;
}

// Delete value from BST
bool bst_remove(BST* bst, int value) {
    if (bst == NULL) {
        fprintf(stderr, "Invalid BST pointer\\n");
        return false;
    }
    
    bool deleted = false;
    bst->root = deleteHelper(bst, bst->root, value, &deleted);
    return deleted;
}

// Free all nodes recursively
static void freeNodes(Node* node) {
    if (node == NULL) {
        return;
    }
    
    freeNodes(node->left);
    freeNodes(node->right);
    secure_clear(node, sizeof(Node));
    free(node);
}

// Destroy BST and free all memory
void bst_destroy(BST* bst) {
    if (bst == NULL) {
        return;
    }
    
    freeNodes(bst->root);
    secure_clear(bst, sizeof(BST));
    free(bst);
}

// Get size of BST
size_t bst_size(const BST* bst) {
    if (bst == NULL) {
        return 0;
    }
    return bst->node_count;
}

int main(void) {
    BST* bst = bst_create();
    if (bst == NULL) {
        return EXIT_FAILURE;
    }
    
    // Test case 1: Basic insertions and search
    printf("Test 1: Basic insertions\\n");
    bst_insert(bst, 50);
    bst_insert(bst, 30);
    bst_insert(bst, 70);
    bst_insert(bst, 20);
    bst_insert(bst, 40);
    printf("Search 30: %s\\n", bst_search(bst, 30) ? "Found" : "Not found");
    printf("Search 100: %s\\n", bst_search(bst, 100) ? "Found" : "Not found");
    
    // Test case 2: Delete leaf node
    printf("\\nTest 2: Delete leaf node (20)\\n");
    bst_remove(bst, 20);
    printf("Search 20 after delete: %s\\n", bst_search(bst, 20) ? "Found" : "Not found");
    
    // Test case 3: Delete node with one child
    printf("\\nTest 3: Delete node with one child (30)\\n");
    bst_remove(bst, 30);
    printf("Search 30 after delete: %s\\n", bst_search(bst, 30) ? "Found" : "Not found");
    printf("Search 40 (child of deleted): %s\\n", bst_search(bst, 40) ? "Found" : "Not found");
    
    // Test case 4: Delete node with two children
    printf("\\nTest 4: Delete root node with two children (50)\\n");
    bst_remove(bst, 50);
    printf("Search 50 after delete: %s\\n", bst_search(bst, 50) ? "Found" : "Not found");
    printf("Tree still maintains structure - search 70: %s\\n", bst_search(bst, 70) ? "Found" : "Not found");
    
    // Test case 5: Insert duplicate and edge values
    printf("\\nTest 5: Duplicate insertion and edge values\\n");
    bst_insert(bst, 70); // Duplicate
    bst_insert(bst, INT_MAX);
    bst_insert(bst, INT_MIN);
    printf("Search max int: %s\\n", bst_search(bst, INT_MAX) ? "Found" : "Not found");
    printf("Search min int: %s\\n", bst_search(bst, INT_MIN) ? "Found" : "Not found");
    printf("Tree size: %zu\\n", bst_size(bst));
    
    // Clean up all allocated memory
    bst_destroy(bst);
    
    return EXIT_SUCCESS;
}
