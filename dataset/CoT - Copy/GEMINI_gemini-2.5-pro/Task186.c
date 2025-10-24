#include <stdio.h>
#include <stdlib.h>

// Definition for singly-linked list.
struct ListNode {
    int val;
    struct ListNode *next;
};

// Helper function to merge two sorted linked lists
struct ListNode* mergeTwoLists(struct ListNode* l1, struct ListNode* l2) {
    if (!l1) return l2;
    if (!l2) return l1;

    struct ListNode dummy;
    struct ListNode* tail = &dummy;
    dummy.next = NULL;

    while (l1 && l2) {
        if (l1->val < l2->val) {
            tail->next = l1;
            l1 = l1->next;
        } else {
            tail->next = l2;
            l2 = l2->next;
        }
        tail = tail->next;
    }

    if (l1) {
        tail->next = l1;
    } else {
        tail->next = l2;
    }
    return dummy.next;
}

// Main function to merge k sorted lists using divide and conquer
struct ListNode* mergeKLists(struct ListNode** lists, int listsSize) {
    if (listsSize == 0) {
        return NULL;
    }
    if (listsSize == 1) {
        return lists[0];
    }
    
    int interval = 1;
    while (interval < listsSize) {
        for (int i = 0; i + interval < listsSize; i = i + interval * 2) {
            lists[i] = mergeTwoLists(lists[i], lists[i + interval]);
        }
        interval *= 2;
    }

    return lists[0];
}

// Helper function to create a linked list from an array
struct ListNode* createList(const int* arr, int size) {
    if (size == 0) {
        return NULL;
    }
    struct ListNode* dummy = (struct ListNode*)malloc(sizeof(struct ListNode));
    struct ListNode* current = dummy;
    for (int i = 0; i < size; ++i) {
        current->next = (struct ListNode*)malloc(sizeof(struct ListNode));
        current->next->val = arr[i];
        current->next->next = NULL;
        current = current->next;
    }
    struct ListNode* head = dummy->next;
    free(dummy);
    return head;
}

// Helper function to print a linked list
void printList(struct ListNode* head) {
    if (!head) {
        printf("[]\n");
        return;
    }
    printf("[");
    struct ListNode* current = head;
    while (current) {
        printf("%d", current->val);
        if (current->next) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]\n");
}

// Helper function to free a linked list
void freeList(struct ListNode* head) {
    struct ListNode* current = head;
    while (current) {
        struct ListNode* next = current->next;
        free(current);
        current = next;
    }
}

int main() {
    // Test Case 1
    printf("Test Case 1:\n");
    int arr1_1[] = {1, 4, 5};
    int arr1_2[] = {1, 3, 4};
    int arr1_3[] = {2, 6};
    struct ListNode** lists1 = (struct ListNode**)malloc(3 * sizeof(struct ListNode*));
    lists1[0] = createList(arr1_1, 3);
    lists1[1] = createList(arr1_2, 3);
    lists1[2] = createList(arr1_3, 2);
    printf("Input:\n");
    printList(lists1[0]);
    printList(lists1[1]);
    printList(lists1[2]);
    struct ListNode* result1 = mergeKLists(lists1, 3);
    printf("Output: ");
    printList(result1);
    freeList(result1); // The result is a combination of original nodes, freeing it frees all.
    free(lists1);
    printf("\n");

    // Test Case 2
    printf("Test Case 2:\n");
    struct ListNode** lists2 = NULL;
    printf("Input: []\n");
    struct ListNode* result2 = mergeKLists(lists2, 0);
    printf("Output: ");
    printList(result2);
    freeList(result2);
    printf("\n");

    // Test Case 3
    printf("Test Case 3:\n");
    struct ListNode** lists3 = (struct ListNode**)malloc(1 * sizeof(struct ListNode*));
    lists3[0] = createList(NULL, 0);
    printf("Input: [[]]\n");
    struct ListNode* result3 = mergeKLists(lists3, 1);
    printf("Output: ");
    printList(result3);
    freeList(result3);
    free(lists3);
    printf("\n");
    
    // Test Case 4
    printf("Test Case 4:\n");
    struct ListNode** lists4 = (struct ListNode**)malloc(2 * sizeof(struct ListNode*));
    int arr4_2[] = {1};
    lists4[0] = createList(NULL, 0);
    lists4[1] = createList(arr4_2, 1);
    printf("Input:\n");
    printList(lists4[0]);
    printList(lists4[1]);
    struct ListNode* result4 = mergeKLists(lists4, 2);
    printf("Output: ");
    printList(result4);
    freeList(result4);
    free(lists4);
    printf("\n");

    // Test Case 5
    printf("Test Case 5:\n");
    int arr5_1[] = {10, 20};
    int arr5_2[] = {5, 15};
    int arr5_3[] = {1, 2, 22};
    struct ListNode** lists5 = (struct ListNode**)malloc(3 * sizeof(struct ListNode*));
    lists5[0] = createList(arr5_1, 2);
    lists5[1] = createList(arr5_2, 2);
    lists5[2] = createList(arr5_3, 3);
    printf("Input:\n");
    printList(lists5[0]);
    printList(lists5[1]);
    printList(lists5[2]);
    struct ListNode* result5 = mergeKLists(lists5, 3);
    printf("Output: ");
    printList(result5);
    freeList(result5);
    free(lists5);
    printf("\n");

    return 0;
}