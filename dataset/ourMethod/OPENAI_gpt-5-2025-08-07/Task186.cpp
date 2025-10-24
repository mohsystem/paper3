#include <bits/stdc++.h>
using namespace std;

struct ListNode {
    int val;
    ListNode* next;
    explicit ListNode(int v) : val(v), next(nullptr) {}
};

struct CmpNode {
    bool operator()(const ListNode* a, const ListNode* b) const noexcept {
        // min-heap behavior via greater-than comparator
        return a->val > b->val;
    }
};

ListNode* mergeKLists(const vector<ListNode*>& lists) {
    priority_queue<ListNode*, vector<ListNode*>, CmpNode> pq;
    for (auto* head : lists) {
        if (head != nullptr) pq.push(head);
    }
    ListNode dummy(0);
    ListNode* tail = &dummy;
    while (!pq.empty()) {
        ListNode* node = pq.top(); pq.pop();
        tail->next = node;
        tail = node;
        if (node->next != nullptr) pq.push(node->next);
    }
    return dummy.next;
}

// Helpers
ListNode* buildList(const vector<int>& arr) {
    if (arr.empty()) return nullptr;
    ListNode dummy(0);
    ListNode* cur = &dummy;
    for (int v : arr) {
        cur->next = new (nothrow) ListNode(v);
        if (!cur->next) {
            // Allocation failed; clean up and return nullptr for safety.
            // Free already allocated nodes.
            ListNode* toFree = dummy.next;
            while (toFree) {
                ListNode* nxt = toFree->next;
                delete toFree;
                toFree = nxt;
            }
            return nullptr;
        }
        cur = cur->next;
    }
    return dummy.next;
}

vector<int> toVector(ListNode* head) {
    vector<int> out;
    out.reserve(1024);
    for (ListNode* cur = head; cur != nullptr; cur = cur->next) {
        out.push_back(cur->val);
    }
    return out;
}

void freeList(ListNode* head) {
    while (head) {
        ListNode* nxt = head->next;
        delete head;
        head = nxt;
    }
}

static void printVec(const vector<int>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ",";
        cout << v[i];
    }
    cout << "]\n";
}

int main() {
    // Test 1: Example 1
    {
        vector<ListNode*> lists;
        lists.push_back(buildList({1,4,5}));
        lists.push_back(buildList({1,3,4}));
        lists.push_back(buildList({2,6}));
        ListNode* merged = mergeKLists(lists);
        printVec(toVector(merged));
        freeList(merged);
    }
    // Test 2: Example 2 - empty list of lists
    {
        vector<ListNode*> lists;
        ListNode* merged = mergeKLists(lists);
        printVec(toVector(merged));
        freeList(merged);
    }
    // Test 3: Example 3 - lists = [[]]
    {
        vector<ListNode*> lists;
        lists.push_back(buildList({}));
        ListNode* merged = mergeKLists(lists);
        printVec(toVector(merged));
        freeList(merged);
    }
    // Test 4: negatives and duplicates
    {
        vector<ListNode*> lists;
        lists.push_back(buildList({-2,-1,3}));
        lists.push_back(buildList({1,1,1}));
        lists.push_back(buildList({2}));
        ListNode* merged = mergeKLists(lists);
        printVec(toVector(merged));
        freeList(merged);
    }
    // Test 5: single list
    {
        vector<ListNode*> lists;
        lists.push_back(buildList({0,5,10}));
        ListNode* merged = mergeKLists(lists);
        printVec(toVector(merged));
        freeList(merged);
    }
    return 0;
}