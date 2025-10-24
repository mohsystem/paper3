// Chain-of-Through secure implementation
// 1) Problem: Merge k sorted linked lists.
// 2) Security: Handle null pointers, avoid UB, limit resource usage.
// 3) Secure coding: Use safe comparator; check inputs.
// 4) Review: Bound operations per constraints; no raw array overruns.
// 5) Final secure output with tests.

#include <iostream>
#include <vector>
#include <queue>

struct ListNode {
    int val;
    ListNode* next;
    explicit ListNode(int v) : val(v), next(nullptr) {}
};

struct Cmp {
    bool operator()(const ListNode* a, const ListNode* b) const {
        return a->val > b->val; // min-heap
    }
};

ListNode* mergeKLists(const std::vector<ListNode*>& lists) {
    if (lists.empty()) return nullptr;
    std::priority_queue<ListNode*, std::vector<ListNode*>, Cmp> pq;
    for (auto node : lists) if (node) pq.push(node);
    ListNode dummy(0);
    ListNode* tail = &dummy;
    while (!pq.empty()) {
        ListNode* cur = pq.top(); pq.pop();
        tail->next = cur;
        tail = cur;
        if (cur->next) pq.push(cur->next);
    }
    return dummy.next;
}

// Utilities
ListNode* buildList(const std::vector<int>& arr) {
    if (arr.empty()) return nullptr;
    ListNode dummy(0);
    ListNode* t = &dummy;
    for (int v : arr) {
        t->next = new ListNode(v);
        t = t->next;
    }
    return dummy.next;
}

std::vector<ListNode*> buildLists(const std::vector<std::vector<int>>& arrs) {
    std::vector<ListNode*> res;
    res.reserve(arrs.size());
    for (const auto& a : arrs) res.push_back(buildList(a));
    return res;
}

std::vector<int> toArray(ListNode* head) {
    std::vector<int> out;
    for (ListNode* cur = head; cur; cur = cur->next) out.push_back(cur->val);
    return out;
}

void printList(ListNode* head) {
    std::vector<int> a = toArray(head);
    std::cout << "[";
    for (size_t i = 0; i < a.size(); ++i) {
        if (i) std::cout << ",";
        std::cout << a[i];
    }
    std::cout << "]\n";
}

void freeList(ListNode* head) {
    while (head) {
        ListNode* n = head->next;
        delete head;
        head = n;
    }
}

int main() {
    // Test case 1
    {
        std::vector<std::vector<int>> t1{{1,4,5},{1,3,4},{2,6}};
        auto lists = buildLists(t1);
        ListNode* merged = mergeKLists(lists);
        printList(merged);
        // Free all nodes (merged contains all nodes)
        freeList(merged);
    }
    // Test case 2
    {
        std::vector<std::vector<int>> t2{};
        auto lists = buildLists(t2);
        ListNode* merged = mergeKLists(lists);
        printList(merged);
        freeList(merged);
    }
    // Test case 3
    {
        std::vector<std::vector<int>> t3{{}};
        auto lists = buildLists(t3);
        ListNode* merged = mergeKLists(lists);
        printList(merged);
        freeList(merged);
    }
    // Test case 4
    {
        std::vector<std::vector<int>> t4{{-10,-5,0,3},{1,2,2},{}};
        auto lists = buildLists(t4);
        ListNode* merged = mergeKLists(lists);
        printList(merged);
        freeList(merged);
    }
    // Test case 5
    {
        std::vector<std::vector<int>> t5{{1},{0},{5},{3},{3},{4}};
        auto lists = buildLists(t5);
        ListNode* merged = mergeKLists(lists);
        printList(merged);
        freeList(merged);
    }
    return 0;
}