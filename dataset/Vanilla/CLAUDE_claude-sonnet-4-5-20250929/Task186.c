
#include <stdio.h>
#include <stdlib.h>

struct ListNode {
    int val;
    struct ListNode *next;
};

struct ListNode* mergeTwoLists(struct ListNode* l1, struct ListNode* l2) {
    struct ListNode dummy;
    dummy.val = 0;
    dummy.next = NULL;
    struct ListNode* current = &dummy;
    
    while (l1 && l2) {
        if (l1->val <= l2->val) {
            current->next = l1;
            l1 = l1->next;
        } else {
            current->next = l2;
            l2 = l2->next;
        }
        current = current->next;
    }
    
    if (l1) {
        current->next = l1;
    }
    if (l2) {
        current->next = l2;
    }
    
    return dummy.next;
}

struct ListNode* mergeKListsHelper(struct ListNode** lists, int left, int right) {
    if (left == right) {
        return lists[left];
    }
    
    if (left < right) {
        int mid = left + (right - left) / 2;
        struct ListNode* l1 = mergeKListsHelper(lists, left, mid);
        struct ListNode* l2 = mergeKListsHelper(lists, mid + 1, right);
        return mergeTwoLists(l1, l2);
    }
    
    return NULL;
}

struct ListNode* mergeKLists(struct ListNode** lists, int listsSize) {
    if (lists == NULL || listsSize == 0) {
        return NULL;
    }
    
    return mergeKListsHelper(lists, 0, listsSize - 1);
}

struct ListNode* createList(int* arr, int size) {
    if (arr == NULL || size == 0) {
        return NULL;
    }
    struct ListNode dummy;
    dummy.val = 0;
    dummy.next = NULL;
    struct ListNode* current = &dummy;
    for (int i = 0; i < size; i++) {
        struct ListNode* newNode = (struct ListNode*)malloc(sizeof(struct ListNode));
        newNode->val = arr[i];
        newNode->next = NULL;
        current->next = newNode;
        current = current->next;
    }
    return dummy.next;
}

void printList(struct ListNode* head) {
    printf("[");
    while (head) {
        printf("%d", head->val);
        if (head->next) {
            printf(",");
        }
        head = head->next;
    }
    printf("]\\n");
}

int main() {
    // Test case 1
    int arr1[] = {1, 4, 5};
    int arr2[] = {1, 3, 4};
    int arr3[] = {2, 6};
    struct ListNode* lists1[3] = {
        createList(arr1, 3),
        createList(arr2, 3),
        createList(arr3, 2)
    };
    printf("Test 1: ");
    printList(mergeKLists(lists1, 3));
    
    // Test case 2
    printf("Test 2: ");
    printList(mergeKLists(NULL, 0));
    
    // Test case 3
    struct ListNode* lists3[1] = {NULL};
    printf("Test 3: ");
    printList(mergeKLists(lists3, 1));
    
    // Test case 4
    int arr4[] = {1, 2, 3};
    int arr5[] = {4, 5, 6};
    struct ListNode* lists4[2] = {
        createList(arr4, 3),
        createList(arr5, 3)
    };
    printf("Test 4: ");
    printList(mergeKLists(lists4, 2));
    
    // Test case 5
    int arr6[] = {-2, -1, 0};
    int arr7[] = {-3};
    int arr8[] = {1, 2};
    struct ListNode* lists5[3] = {
        createList(arr6, 3),
        createList(arr7, 1),
        createList(arr8, 2)
    };
    printf("Test 5: ");
    printList(mergeKLists(lists5, 3));
    
    return 0;
}
