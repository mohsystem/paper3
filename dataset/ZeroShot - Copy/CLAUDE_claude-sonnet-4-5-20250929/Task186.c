
#include <stdio.h>
#include <stdlib.h>

struct ListNode {
    int val;
    struct ListNode *next;
};

struct ListNode* createNode(int val) {
    struct ListNode* node = (struct ListNode*)malloc(sizeof(struct ListNode));
    node->val = val;
    node->next = NULL;
    return node;
}

struct ListNode* mergeTwoLists(struct ListNode* l1, struct ListNode* l2) {
    struct ListNode* dummy = createNode(0);
    struct ListNode* current = dummy;
    
    while (l1 != NULL && l2 != NULL) {
        if (l1->val <= l2->val) {
            current->next = l1;
            l1 = l1->next;
        } else {
            current->next = l2;
            l2 = l2->next;
        }
        current = current->next;
    }
    
    current->next = (l1 != NULL) ? l1 : l2;
    
    struct ListNode* result = dummy->next;
    free(dummy);
    return result;
}

struct ListNode* mergeKListsHelper(struct ListNode** lists, int left, int right) {
    if (left == right) {
        return lists[left];
    }
    
    if (left > right) {
        return NULL;
    }
    
    int mid = left + (right - left) / 2;
    struct ListNode* l1 = mergeKListsHelper(lists, left, mid);
    struct ListNode* l2 = mergeKListsHelper(lists, mid + 1, right);
    
    return mergeTwoLists(l1, l2);
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
    struct ListNode* head = createNode(arr[0]);
    struct ListNode* current = head;
    for (int i = 1; i < size; i++) {
        current->next = createNode(arr[i]);
        current = current->next;
    }
    return head;
}

void printList(struct ListNode* head) {
    printf("[");
    while (head != NULL) {
        printf("%d", head->val);
        if (head->next != NULL) {
            printf(",");
        }
        head = head->next;
    }
    printf("]\\n");
}

int main() {
    // Test case 1
    int arr1_1[] = {1, 4, 5};
    int arr1_2[] = {1, 3, 4};
    int arr1_3[] = {2, 6};
    struct ListNode* lists1[3];
    lists1[0] = createList(arr1_1, 3);
    lists1[1] = createList(arr1_2, 3);
    lists1[2] = createList(arr1_3, 2);
    printf("Test 1: ");
    printList(mergeKLists(lists1, 3));
    
    // Test case 2
    printf("Test 2: ");
    printList(mergeKLists(NULL, 0));
    
    // Test case 3
    struct ListNode* lists3[1];
    lists3[0] = NULL;
    printf("Test 3: ");
    printList(mergeKLists(lists3, 1));
    
    // Test case 4
    int arr4_1[] = {1, 2, 3};
    int arr4_2[] = {4, 5, 6};
    struct ListNode* lists4[2];
    lists4[0] = createList(arr4_1, 3);
    lists4[1] = createList(arr4_2, 3);
    printf("Test 4: ");
    printList(mergeKLists(lists4, 2));
    
    // Test case 5
    int arr5_1[] = {-2, -1};
    int arr5_2[] = {-3, 1, 4};
    int arr5_3[] = {0, 2};
    int arr5_4[] = {-1, 3};
    struct ListNode* lists5[4];
    lists5[0] = createList(arr5_1, 2);
    lists5[1] = createList(arr5_2, 3);
    lists5[2] = createList(arr5_3, 2);
    lists5[3] = createList(arr5_4, 2);
    printf("Test 5: ");
    printList(mergeKLists(lists5, 4));
    
    return 0;
}
