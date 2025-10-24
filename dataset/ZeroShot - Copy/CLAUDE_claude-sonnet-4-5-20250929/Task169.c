
#include <stdio.h>
#include <stdlib.h>

typedef struct TreeNode {
    int val;
    int count;
    int leftSize;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

TreeNode* createNode(int val) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->val = val;
    node->count = 1;
    node->leftSize = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

TreeNode* insert(TreeNode* node, int val, int* result, int index, int preSum) {
    if (node == NULL) {
        node = createNode(val);
        result[index] = preSum;
    } else if (node->val == val) {
        node->count++;
        result[index] = preSum + node->leftSize;
    } else if (node->val > val) {
        node->leftSize++;
        node->left = insert(node->left, val, result, index, preSum);
    } else {
        node->right = insert(node->right, val, result, index, preSum + node->leftSize + node->count);
    }
    return node;
}

int* countSmaller(int* nums, int numsSize, int* returnSize) {
    if (nums == NULL || numsSize == 0) {
        *returnSize = 0;
        return NULL;
    }
    
    *returnSize = numsSize;
    int* result = (int*)malloc(numsSize * sizeof(int));
    TreeNode* root = NULL;
    
    for (int i = numsSize - 1; i >= 0; i--) {
        root = insert(root, nums[i], result, i, 0);
    }
    
    return result;
}

int main() {
    // Test case 1
    int nums1[] = {5, 2, 6, 1};
    int returnSize1;
    int* result1 = countSmaller(nums1, 4, &returnSize1);
    printf("Test 1: ");
    for (int i = 0; i < returnSize1; i++) printf("%d ", result1[i]);
    printf("\\n");
    free(result1);
    
    // Test case 2
    int nums2[] = {-1};
    int returnSize2;
    int* result2 = countSmaller(nums2, 1, &returnSize2);
    printf("Test 2: ");
    for (int i = 0; i < returnSize2; i++) printf("%d ", result2[i]);
    printf("\\n");
    free(result2);
    
    // Test case 3
    int nums3[] = {-1, -1};
    int returnSize3;
    int* result3 = countSmaller(nums3, 2, &returnSize3);
    printf("Test 3: ");
    for (int i = 0; i < returnSize3; i++) printf("%d ", result3[i]);
    printf("\\n");
    free(result3);
    
    // Test case 4
    int nums4[] = {1, 2, 3, 4, 5};
    int returnSize4;
    int* result4 = countSmaller(nums4, 5, &returnSize4);
    printf("Test 4: ");
    for (int i = 0; i < returnSize4; i++) printf("%d ", result4[i]);
    printf("\\n");
    free(result4);
    
    // Test case 5
    int nums5[] = {5, 4, 3, 2, 1};
    int returnSize5;
    int* result5 = countSmaller(nums5, 5, &returnSize5);
    printf("Test 5: ");
    for (int i = 0; i < returnSize5; i++) printf("%d ", result5[i]);
    printf("\\n");
    free(result5);
    
    return 0;
}
