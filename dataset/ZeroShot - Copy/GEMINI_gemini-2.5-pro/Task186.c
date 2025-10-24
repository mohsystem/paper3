#include <stdio.h>
#include <stdlib.h>

// Definition for singly-linked list.
struct ListNode {
    int val;
    struct ListNode *next;
};

/**
 * @brief Merges two sorted linked lists into one sorted linked list.
 * @param l1 Pointer to the head of the first sorted list.
 * @param l2 Pointer to the head of the second sorted list.
 * @return Pointer to the head of the merged sorted list.
 */
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
    tail->next = l1 ? l1 : l2;
    return dummy.next;
}

/**
 * @brief Merges k sorted linked lists using a divide and conquer approach.
 * It repeatedly merges pairs of lists until only one list remains.
 * @param lists An array of pointers to the heads of sorted linked lists.
 * @param listsSize The number of linked lists in the array.
 * @return Pointer to the head of the final merged sorted list.
 */
struct ListNode* mergeKLists(struct ListNode** lists, int listsSize) {
    if (listsSize == 0) {
        return NULL;
    }
    if (listsSize == 1) {
        return lists[0];
    }
    
    int last = listsSize - 1;
    while (last > 0) {
        int i = 0, j = last;
        while (i < j) {
            // Merge list i and list j, store the result in list i.
            lists[i] = mergeTwoLists(lists[i], lists[j]);
            i++;
            j--;
        }
        last = j; // Update `last` to the last merged list's index.
    }
    return lists[0];
}


// --- Helper functions for testing ---
struct ListNode* createList(const int* arr, int size) {
    if (size == 0) return NULL;
    struct ListNode dummy;
    dummy.next = NULL;
    struct ListNode* current = &dummy;
    for (int i = 0; i < size; ++i) {
        current->next = (struct ListNode*)malloc(sizeof(struct ListNode));
        current->next->val = arr[i];
        current->next->next = NULL;
        current = current->next;
    }
    return dummy.next;
}

void printList(struct ListNode* head) {
    struct ListNode* current = head;
    while (current) {
        printf("%d -> ", current->val);
        current = current->next;
    }
    printf("NULL\n");
}

void freeList(struct ListNode* head) {
    struct ListNode* current = head;
    while (current) {
        struct ListNode* temp = current;
        current = current->next;
        free(temp);
    }
}

int main() {
    printf("--- C Tests ---\n");

    // Test Case 1: Example 1
    int arr1_1[] = {1, 4, 5};
    int arr1_2[] = {1, 3, 4};
    int arr1_3[] = {2, 6};
    struct ListNode* lists1[] = {
        createList(arr1_1, 3),
        createList(arr1_2, 3),
        createList(arr1_3, 2)
    };
    printf("Test Case 1: ");
    struct ListNode* result1 = mergeKLists(lists1, 3);
    printList(result1);
    freeList(result1);

    // Test Case 2: Example 2 (empty array)
    printf("Test Case 2: ");
    struct ListNode* result2 = mergeKLists(NULL, 0);
    printList(result2);
    // No need to free

    // Test Case 3: Example 3 (array with one empty list)
    struct ListNode* lists3[] = {createList(NULL, 0)};
    printf("Test Case 3: ");
    struct ListNode* result3 = mergeKLists(lists3, 1);
    printList(result3);
    // No need to free

    // Test Case 4: Mix of empty and non-empty lists
    int arr4_1[] = {1, 2, 3};
    int arr4_3[] = {4, 5};
    struct ListNode* lists4[] = {
        createList(arr4_1, 3),
        createList(NULL, 0),
        createList(arr4_3, 2)
    };
    printf("Test Case 4: ");
    struct ListNode* result4 = mergeKLists(lists4, 3);
    printList(result4);
    freeList(result4);

    // Test Case 5: Edge case with small lists
    int arr5_1[] = {1};
    int arr5_2[] = {0};
    struct ListNode* lists5[] = {
        createList(arr5_1, 1),
        createList(arr5_2, 1)
    };
    printf("Test Case 5: ");
    struct ListNode* result5 = mergeKLists(lists5, 2);
    printList(result5);
    freeList(result5);

    return 0;
}