
#include <iostream>
#include <vector>
#include <queue>
#include <functional>

// Security: Define ListNode structure with proper initialization
struct ListNode {
    int val;
    ListNode *next;
    // Security: Constructor ensures proper initialization of all members
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

// Security: Custom comparator for priority queue to ensure deterministic ordering
struct CompareNode {
    bool operator()(ListNode* a, ListNode* b) {
        // Security: Null pointer checks before dereferencing
        if (!a) return true;
        if (!b) return false;
        return a->val > b->val;
    }
};

class Solution {
public:
    ListNode* mergeKLists(std::vector<ListNode*>& lists) {
        // Security: Validate input - check for null or empty input
        if (lists.empty()) {
            return nullptr;
        }
        
        // Security: Use priority queue (min-heap) to avoid buffer operations
        // This avoids manual array manipulation and associated overflow risks
        std::priority_queue<ListNode*, std::vector<ListNode*>, CompareNode> pq;
        
        // Security: Validate and add non-null list heads to priority queue
        // Bounds check: iterate only within vector size
        for (size_t i = 0; i < lists.size(); ++i) {
            if (lists[i] != nullptr) {
                pq.push(lists[i]);
            }
        }
        
        // Security: Use dummy head to simplify list building and avoid null checks
        ListNode dummy(0);
        ListNode* tail = &dummy;
        
        // Security: Process nodes while queue is not empty
        while (!pq.empty()) {
            // Security: Get minimum node safely
            ListNode* minNode = pq.top();
            pq.pop();
            
            // Security: Null check before dereferencing
            if (!minNode) {
                continue;
            }
            
            // Security: Append node to result list
            tail->next = minNode;
            tail = tail->next;
            
            // Security: If current list has more nodes, add next node to queue
            if (minNode->next != nullptr) {
                pq.push(minNode->next);
            }
        }
        
        // Security: Return the merged list (skip dummy head)
        return dummy.next;
    }
};

// Security: Helper function to create linked list from vector with bounds checking
ListNode* createList(const std::vector<int>& values) {
    if (values.empty()) {
        return nullptr;
    }
    
    // Security: Allocate head node and check for allocation failure
    ListNode* head = new(std::nothrow) ListNode(values[0]);
    if (!head) {
        return nullptr;
    }
    
    ListNode* current = head;
    // Security: Bounds-checked iteration
    for (size_t i = 1; i < values.size(); ++i) {
        current->next = new(std::nothrow) ListNode(values[i]);
        if (!current->next) {
            // Security: Clean up on allocation failure
            while (head) {
                ListNode* temp = head;
                head = head->next;
                delete temp;
            }
            return nullptr;
        }
        current = current->next;
    }
    return head;
}

// Security: Helper function to free linked list memory
void freeList(ListNode* head) {
    while (head) {
        ListNode* temp = head;
        head = head->next;
        delete temp;
    }
}

// Security: Helper function to print list with bounds checking
void printList(ListNode* head) {
    std::cout << "[";
    bool first = true;
    // Security: Track iteration count to prevent infinite loops
    int count = 0;
    const int MAX_NODES = 10000; // Per problem constraints
    
    while (head && count < MAX_NODES) {
        if (!first) std::cout << ",";
        std::cout << head->val;
        first = false;
        head = head->next;
        count++;
    }
    std::cout << "]" << std::endl;
}

int main() {
    Solution solution;
    
    // Test case 1
    std::vector<ListNode*> lists1 = {
        createList({1, 4, 5}),
        createList({1, 3, 4}),
        createList({2, 6})
    };
    ListNode* result1 = solution.mergeKLists(lists1);
    std::cout << "Test 1: ";
    printList(result1);
    freeList(result1);
    
    // Test case 2: empty input
    std::vector<ListNode*> lists2;
    ListNode* result2 = solution.mergeKLists(lists2);
    std::cout << "Test 2: ";
    printList(result2);
    
    // Test case 3: single empty list
    std::vector<ListNode*> lists3 = {nullptr};
    ListNode* result3 = solution.mergeKLists(lists3);
    std::cout << "Test 3: ";
    printList(result3);
    
    // Test case 4: single list
    std::vector<ListNode*> lists4 = {createList({1, 2, 3})};
    ListNode* result4 = solution.mergeKLists(lists4);
    std::cout << "Test 4: ";
    printList(result4);
    freeList(result4);
    
    // Test case 5: negative numbers
    std::vector<ListNode*> lists5 = {
        createList({-2, -1, 0}),
        createList({-3, 1, 2})
    };
    ListNode* result5 = solution.mergeKLists(lists5);
    std::cout << "Test 5: ";
    printList(result5);
    freeList(result5);
    
    return 0;
}
