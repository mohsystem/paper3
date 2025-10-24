#include <bits/stdc++.h>
using namespace std;

struct ListNode {
    int val;
    ListNode* next;
    explicit ListNode(int v) : val(v), next(nullptr) {}
};

struct Cmp {
    bool operator()(const ListNode* a, const ListNode* b) const {
        return a->val > b->val;
    }
};

ListNode* mergeKLists(const vector<ListNode*>& lists) {
    priority_queue<ListNode*, vector<ListNode*>, Cmp> pq;
    for (auto node : lists) if (node) pq.push(node);
    ListNode dummy(0);
    ListNode* tail = &dummy;
    while (!pq.empty()) {
        ListNode* node = pq.top(); pq.pop();
        tail->next = node;
        tail = node;
        if (node->next) pq.push(node->next);
    }
    return dummy.next;
}

// Helpers
ListNode* buildList(const vector<int>& arr) {
    ListNode dummy(0);
    ListNode* cur = &dummy;
    for (int v : arr) {
        cur->next = new (nothrow) ListNode(v);
        if (!cur->next) {
            // Allocation failed; clean up and return partial result
            ListNode* t = dummy.next;
            while (t) { ListNode* n = t->next; delete t; t = n; }
            return nullptr;
        }
        cur = cur->next;
    }
    return dummy.next;
}

vector<int> toVector(ListNode* head) {
    vector<int> res;
    while (head) {
        res.push_back(head->val);
        head = head->next;
    }
    return res;
}

void freeList(ListNode* head) {
    while (head) {
        ListNode* n = head->next;
        delete head;
        head = n;
    }
}

int main() {
    // Test 1
    {
        vector<ListNode*> lists = {
            buildList({1,4,5}),
            buildList({1,3,4}),
            buildList({2,6})
        };
        ListNode* merged = mergeKLists(lists);
        vector<int> out = toVector(merged);
        cout << "[";
        for (size_t i = 0; i < out.size(); ++i) {
            if (i) cout << ",";
            cout << out[i];
        }
        cout << "]\n";
        freeList(merged);
    }
    // Test 2
    {
        vector<ListNode*> lists = { };
        ListNode* merged = mergeKLists(lists);
        vector<int> out = toVector(merged);
        cout << "[";
        for (size_t i = 0; i < out.size(); ++i) {
            if (i) cout << ",";
            cout << out[i];
        }
        cout << "]\n";
        freeList(merged);
    }
    // Test 3
    {
        vector<ListNode*> lists = { buildList({}) };
        ListNode* merged = mergeKLists(lists);
        vector<int> out = toVector(merged);
        cout << "[";
        for (size_t i = 0; i < out.size(); ++i) {
            if (i) cout << ",";
            cout << out[i];
        }
        cout << "]\n";
        freeList(merged);
    }
    // Test 4
    {
        vector<ListNode*> lists = {
            buildList({-10,-5,0,5}),
            buildList({-6,-3,2,2,7}),
            buildList({})
        };
        ListNode* merged = mergeKLists(lists);
        vector<int> out = toVector(merged);
        cout << "[";
        for (size_t i = 0; i < out.size(); ++i) {
            if (i) cout << ",";
            cout << out[i];
        }
        cout << "]\n";
        freeList(merged);
    }
    // Test 5
    {
        vector<ListNode*> lists = {
            buildList({1}),
            buildList({}),
            buildList({}),
            buildList({0}),
            buildList({1,1,1}),
            buildList({-1,2})
        };
        ListNode* merged = mergeKLists(lists);
        vector<int> out = toVector(merged);
        cout << "[";
        for (size_t i = 0; i < out.size(); ++i) {
            if (i) cout << ",";
            cout << out[i];
        }
        cout << "]\n";
        freeList(merged);
    }
    return 0;
}