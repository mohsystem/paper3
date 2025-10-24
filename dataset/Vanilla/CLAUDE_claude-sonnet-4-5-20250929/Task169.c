
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int val;
    int count;
    struct Node* left;
    struct Node* right;
} Node;

Node* createNode(int val) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->val = val;
    node->count = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

Node* insert(Node* node, int val, int* result, int index, int preSum) {
    if (node == NULL) {
        node = createNode(val);
        result[index] = preSum;
    } else if (node->val > val) {
        node->count++;
        node->left = insert(node->left, val, result, index, preSum);
    } else {
        node->right = insert(node->right, val, result, index, preSum + node->count + (node->val < val ? 1 : 0));
    }
    return node;
}

int* countSmaller(int* nums, int numsSize, int* returnSize) {
    *returnSize = numsSize;
    int* result = (int*)calloc(numsSize, sizeof(int));
    if (numsSize == 0) return result;
    
    Node* root = NULL;
    for (int i = numsSize - 1; i >= 0; i--) {
        root = insert(root, nums[i], result, i, 0);
    }
    
    return result;
}

int main() {
    // Test case 1
    int nums1[] = {5, 2, 6, 1};
    int size1;
    int* res1 = countSmaller(nums1, 4, &size1);
    printf("Test 1: ");
    for (int i = 0; i < size1; i++) printf("%d ", res1[i]);
    printf("\\n");
    free(res1);
    
    // Test case 2
    int nums2[] = {-1};
    int size2;
    int* res2 = countSmaller(nums2, 1, &size2);
    printf("Test 2: ");
    for (int i = 0; i < size2; i++) printf("%d ", res2[i]);
    printf("\\n");
    free(res2);
    
    // Test case 3
    int nums3[] = {-1, -1};
    int size3;
    int* res3 = countSmaller(nums3, 2, &size3);
    printf("Test 3: ");
    for (int i = 0; i < size3; i++) printf("%d ", res3[i]);
    printf("\\n");
    free(res3);
    
    // Test case 4
    int nums4[] = {1, 2, 3, 4, 5};
    int size4;
    int* res4 = countSmaller(nums4, 5, &size4);
    printf("Test 4: ");
    for (int i = 0; i < size4; i++) printf("%d ", res4[i]);
    printf("\\n");
    free(res4);
    
    // Test case 5
    int nums5[] = {5, 4, 3, 2, 1};
    int size5;
    int* res5 = countSmaller(nums5, 5, &size5);
    printf("Test 5: ");
    for (int i = 0; i < size5; i++) printf("%d ", res5[i]);
    printf("\\n");
    free(res5);
    
    return 0;
}
