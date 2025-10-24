#include <stdio.h>
#include <stdlib.h>

// Definition for singly-linked list.
struct ListNode {
    int val;
    struct ListNode *next;
};

// Helper function to merge two sorted lists
struct ListNode* mergeTwoLists(struct ListNode* l1, struct ListNode* l2) {
    if (!l1) return l2;
    if (!l2) return l1;

    struct ListNode dummy;
    dummy.val = -1;
    dummy.next = NULL;
    struct ListNode* tail = &dummy;

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

// Main function to merge k sorted lists using divide and conquer (bottom-up merge)
struct ListNode* mergeKLists(struct ListNode** lists, int listsSize) {
    if (listsSize == 0) {
        return NULL;
    }
    
    int interval = 1;
    while (interval < listsSize) {
        for (int i = 0; i + interval < listsSize; i += interval * 2) {
            lists[i] = mergeTwoLists(lists[i], lists[i + interval]);
        }
        interval *= 2;
    }
    
    return lists[0];
}


// Helper function to create a linked list from an array
struct ListNode* createLinkedList(const int* arr, int size) {
    if (size == 0) {
        return NULL;
    }
    struct ListNode* dummy = (struct ListNode*)malloc(sizeof(struct ListNode));
    dummy->next = NULL;
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
void printLinkedList(struct ListNode* head) {
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

// Helper function to delete a linked list and free memory
void deleteLinkedList(struct ListNode* head) {
    struct ListNode* current = head;
    while (current) {
        struct ListNode* next = current->next;
        free(current);
        current = next;
    }
}

int main() {
    // Test Case 1: Example 1
    printf("Test Case 1:\n");
    int arr1_1[] = {1, 4, 5};
    int arr1_2[] = {1, 3, 4};
    int arr1_3[] = {2, 6};
    struct ListNode* lists1[] = {
        createLinkedList(arr1_1, 3),
        createLinkedList(arr1_2, 3),
        createLinkedList(arr1_3, 2)
    };
    struct ListNode* result1 = mergeKLists(lists1, 3);
    printLinkedList(result1);
    deleteLinkedList(result1);

    // Test Case 2: Example 2
    printf("\nTest Case 2:\n");
    struct ListNode** lists2 = NULL;
    struct ListNode* result2 = mergeKLists(lists2, 0);
    printLinkedList(result2);

    // Test Case 3: Example 3
    printf("\nTest Case 3:\n");
    struct ListNode* lists3[] = {createLinkedList(NULL, 0)};
    struct ListNode* result3 = mergeKLists(lists3, 1);
    printLinkedList(result3);
    deleteLinkedList(result3);
    
    // Test Case 4: lists with null and single elements
    printf("\nTest Case 4:\n");
    int arr4_1[] = {1};
    int arr4_3[] = {0};
    struct ListNode* lists4[] = {
        createLinkedList(arr4_1, 1),
        NULL,
        createLinkedList(arr4_3, 1)
    };
    struct ListNode* result4 = mergeKLists(lists4, 3);
    printLinkedList(result4);
    deleteLinkedList(result4);

    // Test Case 5: lists with negative numbers
    printf("\nTest Case 5:\n");
    int arr5_1[] = {-2, -1, 0};
    int arr5_2[] = {-3, 1, 2};
    int arr5_3[] = {-5, 5, 10};
    struct ListNode* lists5[] = {
        createLinkedList(arr5_1, 3),
        createLinkedList(arr5_2, 3),
        createLinkedList(arr5_3, 3)
    };
    struct ListNode* result5 = mergeKLists(lists5, 3);
    printLinkedList(result5);
    deleteLinkedList(result5);

    return 0;
}