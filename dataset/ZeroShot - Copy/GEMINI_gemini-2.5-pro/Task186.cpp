#include <iostream>
#include <vector>
#include <queue>
#include <functional>

// Definition for singly-linked list.
struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

// Custom comparator for the min-heap.
// std::priority_queue is a max-heap by default, so we need to use `>`
// to simulate a min-heap (the element with the "greatest" priority is smallest).
struct CompareNode {
    bool operator()(const ListNode* a, const ListNode* b) {
        return a->val > b->val;
    }
};

class Solution {
public:
    /**
     * @brief Merges k sorted linked lists into one single sorted linked list.
     * This implementation uses a min-heap to efficiently find the minimum
     * node among the heads of all lists.
     * @param lists A vector of pointers to the heads of sorted linked lists.
     * @return A pointer to the head of the merged sorted linked list.
     */
    ListNode* mergeKLists(std::vector<ListNode*>& lists) {
        if (lists.empty()) {
            return nullptr;
        }

        std::priority_queue<ListNode*, std::vector<ListNode*>, CompareNode> pq;

        // Add the head of each non-empty list to the priority queue.
        for (ListNode* head : lists) {
            if (head) {
                pq.push(head);
            }
        }

        // A dummy node to simplify the construction of the result list.
        ListNode dummy(0);
        ListNode* tail = &dummy;

        // Process nodes from the priority queue until it's empty.
        while (!pq.empty()) {
            // Get the node with the smallest value.
            ListNode* minNode = pq.top();
            pq.pop();

            // Append it to the result list.
            tail->next = minNode;
            tail = tail->next;

            // If the extracted node has a next element, add it to the queue.
            if (minNode->next) {
                pq.push(minNode->next);
            }
        }

        return dummy.next;
    }
};

// --- Helper functions for testing ---
ListNode* createList(const std::vector<int>& arr) {
    if (arr.empty()) return nullptr;
    ListNode dummy(0);
    ListNode* current = &dummy;
    for (int val : arr) {
        current->next = new ListNode(val);
        current = current->next;
    }
    return dummy.next;
}

void printList(ListNode* head) {
    ListNode* current = head;
    while (current) {
        std::cout << current->val << " -> ";
        current = current->next;
    }
    std::cout << "NULL" << std::endl;
}

void freeList(ListNode* head) {
    ListNode* current = head;
    while (current) {
        ListNode* temp = current;
        current = current->next;
        delete temp;
    }
}

int main() {
    Solution solution;
    std::cout << "--- C++ Tests ---" << std::endl;

    // Test Case 1: Example 1
    std::vector<ListNode*> lists1 = {
        createList({1, 4, 5}),
        createList({1, 3, 4}),
        createList({2, 6})
    };
    std::cout << "Test Case 1: ";
    ListNode* result1 = solution.mergeKLists(lists1);
    printList(result1);
    freeList(result1);

    // Test Case 2: Example 2 (empty vector)
    std::vector<ListNode*> lists2 = {};
    std::cout << "Test Case 2: ";
    ListNode* result2 = solution.mergeKLists(lists2);
    printList(result2);
    // No need to free, result is nullptr

    // Test Case 3: Example 3 (vector with one empty list)
    std::vector<ListNode*> lists3 = {createList({})};
    std::cout << "Test Case 3: ";
    ListNode* result3 = solution.mergeKLists(lists3);
    printList(result3);
    // No need to free, result is nullptr

    // Test Case 4: Mix of empty and non-empty lists
    std::vector<ListNode*> lists4 = {
        createList({1, 2, 3}),
        createList({}),
        createList({4, 5})
    };
    std::cout << "Test Case 4: ";
    ListNode* result4 = solution.mergeKLists(lists4);
    printList(result4);
    freeList(result4);

    // Test Case 5: Edge case with small lists
    std::vector<ListNode*> lists5 = {
        createList({1}),
        createList({0})
    };
    std::cout << "Test Case 5: ";
    ListNode* result5 = solution.mergeKLists(lists5);
    printList(result5);
    freeList(result5);
    
    return 0;
}