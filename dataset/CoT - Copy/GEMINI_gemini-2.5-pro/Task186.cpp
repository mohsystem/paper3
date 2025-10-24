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

// Comparator for the priority queue to create a min-heap
struct CompareNodes {
    bool operator()(const ListNode* a, const ListNode* b) {
        return a->val > b->val;
    }
};

class Solution {
public:
    ListNode* mergeKLists(std::vector<ListNode*>& lists) {
        if (lists.empty()) {
            return nullptr;
        }

        std::priority_queue<ListNode*, std::vector<ListNode*>, CompareNodes> minHeap;

        for (ListNode* list : lists) {
            if (list) {
                minHeap.push(list);
            }
        }

        ListNode* dummy = new ListNode(0);
        ListNode* tail = dummy;

        while (!minHeap.empty()) {
            ListNode* node = minHeap.top();
            minHeap.pop();

            tail->next = node;
            tail = tail->next;

            if (node->next) {
                minHeap.push(node->next);
            }
        }

        ListNode* result = dummy->next;
        delete dummy;
        return result;
    }
};

// Helper function to create a linked list from a vector
ListNode* createList(const std::vector<int>& arr) {
    if (arr.empty()) {
        return nullptr;
    }
    ListNode* dummy = new ListNode(0);
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
void printList(ListNode* head) {
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
void deleteList(ListNode* head) {
    ListNode* current = head;
    while (current) {
        ListNode* next = current->next;
        delete current;
        current = next;
    }
}

int main() {
    Solution solution;

    // Test Case 1
    std::cout << "Test Case 1:" << std::endl;
    std::vector<ListNode*> lists1 = {
        createList({1, 4, 5}),
        createList({1, 3, 4}),
        createList({2, 6})
    };
    std::cout << "Input:" << std::endl;
    for (auto l : lists1) printList(l);
    ListNode* result1 = solution.mergeKLists(lists1);
    std::cout << "Output: ";
    printList(result1);
    deleteList(result1); // Clean up merged list
    std::cout << std::endl;


    // Test Case 2
    std::cout << "Test Case 2:" << std::endl;
    std::vector<ListNode*> lists2;
    std::cout << "Input: []" << std::endl;
    ListNode* result2 = solution.mergeKLists(lists2);
    std::cout << "Output: ";
    printList(result2);
    deleteList(result2);
    std::cout << std::endl;

    // Test Case 3
    std::cout << "Test Case 3:" << std::endl;
    std::vector<ListNode*> lists3 = {createList({})};
    std::cout << "Input: [[]]" << std::endl;
    ListNode* result3 = solution.mergeKLists(lists3);
    std::cout << "Output: ";
    printList(result3);
    deleteList(result3);
    std::cout << std::endl;

    // Test Case 4
    std::cout << "Test Case 4:" << std::endl;
    std::vector<ListNode*> lists4 = {createList({}), createList({1})};
    std::cout << "Input:" << std::endl;
    for (auto l : lists4) printList(l);
    ListNode* result4 = solution.mergeKLists(lists4);
    std::cout << "Output: ";
    printList(result4);
    deleteList(result4);
    std::cout << std::endl;

    // Test Case 5
    std::cout << "Test Case 5:" << std::endl;
    std::vector<ListNode*> lists5 = {
        createList({10, 20}),
        createList({5, 15}),
        createList({1, 2, 22})
    };
    std::cout << "Input:" << std::endl;
    for (auto l : lists5) printList(l);
    ListNode* result5 = solution.mergeKLists(lists5);
    std::cout << "Output: ";
    printList(result5);
    deleteList(result5);
    std::cout << std::endl;

    return 0;
}