#include <iostream>
#include <vector>
#include <queue>

// Definition for singly-linked list.
struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

class Solution {
public:
    ListNode* mergeKLists(std::vector<ListNode*>& lists) {
        // Custom comparator for the min-heap
        struct CompareNodes {
            bool operator()(const ListNode* a, const ListNode* b) {
                return a->val > b->val;
            }
        };
        
        std::priority_queue<ListNode*, std::vector<ListNode*>, CompareNodes> pq;

        for (ListNode* node : lists) {
            if (node) {
                pq.push(node);
            }
        }

        ListNode dummy_node(0);
        ListNode* tail = &dummy_node;

        while (!pq.empty()) {
            ListNode* node = pq.top();
            pq.pop();
            tail->next = node;
            tail = tail->next;
            if (node->next) {
                pq.push(node->next);
            }
        }
        
        return dummy_node.next;
    }
};

// Helper function to create a linked list from a vector
ListNode* createLinkedList(const std::vector<int>& arr) {
    if (arr.empty()) {
        return nullptr;
    }
    ListNode* dummy = new ListNode(-1);
    ListNode* current = dummy;
    for (int val : arr) {
        current->next = new ListNode(val);
        current = current->next;
    }
    ListNode* head = dummy->next;
    delete dummy;
    return head;
}

// Helper function to print a linked list
void printLinkedList(ListNode* head) {
    if (!head) {
        std::cout << "[]" << std::endl;
        return;
    }
    std::cout << "[";
    ListNode* current = head;
    while (current) {
        std::cout << current->val;
        if (current->next) {
            std::cout << ", ";
        }
        current = current->next;
    }
    std::cout << "]" << std::endl;
}

// Helper function to delete a linked list and free memory
void deleteLinkedList(ListNode* head) {
    ListNode* current = head;
    while (current) {
        ListNode* next = current->next;
        delete current;
        current = next;
    }
}

int main() {
    Solution solution;

    // Test Case 1: Example 1
    std::cout << "Test Case 1:" << std::endl;
    std::vector<ListNode*> lists1 = {
        createLinkedList({1, 4, 5}),
        createLinkedList({1, 3, 4}),
        createLinkedList({2, 6})
    };
    ListNode* result1 = solution.mergeKLists(lists1);
    printLinkedList(result1);
    deleteLinkedList(result1);

    // Test Case 2: Example 2
    std::cout << "\nTest Case 2:" << std::endl;
    std::vector<ListNode*> lists2 = {};
    ListNode* result2 = solution.mergeKLists(lists2);
    printLinkedList(result2);
    deleteLinkedList(result2);

    // Test Case 3: Example 3
    std::cout << "\nTest Case 3:" << std::endl;
    std::vector<ListNode*> lists3 = {createLinkedList({})};
    ListNode* result3 = solution.mergeKLists(lists3);
    printLinkedList(result3);
    deleteLinkedList(result3);

    // Test Case 4: lists with null and single elements
    std::cout << "\nTest Case 4:" << std::endl;
    std::vector<ListNode*> lists4 = {
        createLinkedList({1}),
        nullptr,
        createLinkedList({0})
    };
    ListNode* result4 = solution.mergeKLists(lists4);
    printLinkedList(result4);
    deleteLinkedList(result4);

    // Test Case 5: lists with negative numbers
    std::cout << "\nTest Case 5:" << std::endl;
    std::vector<ListNode*> lists5 = {
        createLinkedList({-2, -1, 0}),
        createLinkedList({-3, 1, 2}),
        createLinkedList({-5, 5, 10})
    };
    ListNode* result5 = solution.mergeKLists(lists5);
    printLinkedList(result5);
    deleteLinkedList(result5);

    return 0;
}