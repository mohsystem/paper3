#include <iostream>
#include <vector>
#include <queue>
#include <functional>

struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

ListNode* mergeKLists(std::vector<ListNode*>& lists) {
    // Custom comparator for the min-heap to compare nodes by value.
    auto compare = [](ListNode* a, ListNode* b) {
        return a->val > b->val;
    };
    
    std::priority_queue<ListNode*, std::vector<ListNode*>, decltype(compare)> minHeap(compare);

    // Add the head of each non-empty list to the heap.
    for (ListNode* head : lists) {
        if (head) {
            minHeap.push(head);
        }
    }

    // A dummy node to simplify list construction.
    ListNode dummy;
    ListNode* current = &dummy;

    // Process nodes from the heap until it's empty.
    while (!minHeap.empty()) {
        // Get the node with the smallest value.
        ListNode* minNode = minHeap.top();
        minHeap.pop();
        
        // Append it to the result list.
        current->next = minNode;
        current = current->next;

        // If the extracted node has a next element, add it to the heap.
        if (minNode->next) {
            minHeap.push(minNode->next);
        }
    }

    return dummy.next;
}

// Helper function to create a linked list from a vector for testing.
ListNode* createLinkedList(const std::vector<int>& vals) {
    if (vals.empty()) {
        return nullptr;
    }
    ListNode* head = new ListNode(vals[0]);
    ListNode* current = head;
    for (size_t i = 1; i < vals.size(); ++i) {
        current->next = new ListNode(vals[i]);
        current = current->next;
    }
    return head;
}

// Helper function to print a linked list for testing.
void printLinkedList(ListNode* head) {
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

// Helper function to free a linked list's memory.
void freeLinkedList(ListNode* head) {
    while (head) {
        ListNode* temp = head;
        head = head->next;
        delete temp;
    }
}

int main() {
    // Test Case 1
    std::vector<ListNode*> lists1 = {
        createLinkedList({1, 4, 5}),
        createLinkedList({1, 3, 4}),
        createLinkedList({2, 6})
    };
    std::cout << "Test Case 1:" << std::endl;
    ListNode* result1 = mergeKLists(lists1);
    printLinkedList(result1);
    freeLinkedList(result1);

    // Test Case 2
    std::vector<ListNode*> lists2;
    std::cout << "Test Case 2:" << std::endl;
    ListNode* result2 = mergeKLists(lists2);
    printLinkedList(result2);
    freeLinkedList(result2);

    // Test Case 3
    std::vector<ListNode*> lists3 = {createLinkedList({})};
    std::cout << "Test Case 3:" << std::endl;
    ListNode* result3 = mergeKLists(lists3);
    printLinkedList(result3);
    freeLinkedList(result3);

    // Test Case 4
    std::vector<ListNode*> lists4 = {
        createLinkedList({}),
        createLinkedList({1})
    };
    std::cout << "Test Case 4:" << std::endl;
    ListNode* result4 = mergeKLists(lists4);
    printLinkedList(result4);
    freeLinkedList(result4);

    // Test Case 5
    std::vector<ListNode*> lists5 = {
        createLinkedList({9}),
        createLinkedList({1, 5, 10}),
        createLinkedList({2, 3, 7, 12})
    };
    std::cout << "Test Case 5:" << std::endl;
    ListNode* result5 = mergeKLists(lists5);
    printLinkedList(result5);
    freeLinkedList(result5);

    return 0;
}