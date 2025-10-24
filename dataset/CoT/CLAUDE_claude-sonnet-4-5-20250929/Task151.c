
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct Task151 {
    Node* root;
} Task151;

Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

Node* insertRec(Node* root, int data) {
    if (root == NULL) {
        return createNode(data);
    }
    
    if (data < root->data) {
        root->left = insertRec(root->left, data);
    } else if (data > root->data) {
        root->right = insertRec(root->right, data);
    }
    
    return root;
}

bool searchRec(Node* root, int data) {
    if (root == NULL) {
        return false;
    }
    
    if (root->data == data) {
        return true;
    }
    
    if (data < root->data) {
        return searchRec(root->left, data);
    } else {
        return searchRec(root->right, data);
    }
}

int minValue(Node* root) {
    int minVal = root->data;
    while (root->left != NULL) {
        minVal = root->left->data;
        root = root->left;
    }
    return minVal;
}

Node* deleteRec(Node* root, int data) {
    if (root == NULL) {
        return NULL;
    }
    
    if (data < root->data) {
        root->left = deleteRec(root->left, data);
    } else if (data > root->data) {
        root->right = deleteRec(root->right, data);
    } else {
        if (root->left == NULL) {
            Node* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Node* temp = root->left;
            free(root);
            return temp;
        }
        
        root->data = minValue(root->right);
        root->right = deleteRec(root->right, root->data);
    }
    
    return root;
}

void inorderRec(Node* root) {
    if (root != NULL) {
        inorderRec(root->left);
        printf("%d ", root->data);
        inorderRec(root->right);
    }
}

void destroyTree(Node* root) {
    if (root != NULL) {
        destroyTree(root->left);
        destroyTree(root->right);
        free(root);
    }
}

Task151* createBST() {
    Task151* bst = (Task151*)malloc(sizeof(Task151));
    if (bst == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        exit(EXIT_FAILURE);
    }
    bst->root = NULL;
    return bst;
}

void insert(Task151* bst, int data) {
    if (bst == NULL) return;
    bst->root = insertRec(bst->root, data);
}

bool search(Task151* bst, int data) {
    if (bst == NULL) return false;
    return searchRec(bst->root, data);
}

void delete(Task151* bst, int data) {
    if (bst == NULL) return;
    bst->root = deleteRec(bst->root, data);
}

void inorder(Task151* bst) {
    if (bst == NULL) return;
    inorderRec(bst->root);
    printf("\\n");
}

void destroyBST(Task151* bst) {
    if (bst == NULL) return;
    destroyTree(bst->root);
    free(bst);
}

int main() {
    // Test Case 1: Basic insertion and search
    printf("Test Case 1: Basic insertion and search\\n");
    Task151* bst1 = createBST();
    insert(bst1, 50);
    insert(bst1, 30);
    insert(bst1, 70);
    insert(bst1, 20);
    insert(bst1, 40);
    printf("Inorder traversal: ");
    inorder(bst1);
    printf("Search 40: %s\\n", search(bst1, 40) ? "true" : "false");
    printf("Search 60: %s\\n", search(bst1, 60) ? "true" : "false");
    printf("\\n");
    destroyBST(bst1);
    
    // Test Case 2: Delete leaf node
    printf("Test Case 2: Delete leaf node\\n");
    Task151* bst2 = createBST();
    insert(bst2, 50);
    insert(bst2, 30);
    insert(bst2, 70);
    insert(bst2, 20);
    printf("Before delete: ");
    inorder(bst2);
    delete(bst2, 20);
    printf("After delete 20: ");
    inorder(bst2);
    printf("\\n");
    destroyBST(bst2);
    
    // Test Case 3: Delete node with one child
    printf("Test Case 3: Delete node with one child\\n");
    Task151* bst3 = createBST();
    insert(bst3, 50);
    insert(bst3, 30);
    insert(bst3, 70);
    insert(bst3, 60);
    printf("Before delete: ");
    inorder(bst3);
    delete(bst3, 70);
    printf("After delete 70: ");
    inorder(bst3);
    printf("\\n");
    destroyBST(bst3);
    
    // Test Case 4: Delete node with two children
    printf("Test Case 4: Delete node with two children\\n");
    Task151* bst4 = createBST();
    insert(bst4, 50);
    insert(bst4, 30);
    insert(bst4, 70);
    insert(bst4, 20);
    insert(bst4, 40);
    insert(bst4, 60);
    insert(bst4, 80);
    printf("Before delete: ");
    inorder(bst4);
    delete(bst4, 50);
    printf("After delete 50: ");
    inorder(bst4);
    printf("\\n");
    destroyBST(bst4);
    
    // Test Case 5: Empty tree operations
    printf("Test Case 5: Empty tree operations\\n");
    Task151* bst5 = createBST();
    printf("Search in empty tree: %s\\n", search(bst5, 10) ? "true" : "false");
    delete(bst5, 10);
    printf("Inorder of empty tree: ");
    inorder(bst5);
    destroyBST(bst5);
    
    return 0;
}
