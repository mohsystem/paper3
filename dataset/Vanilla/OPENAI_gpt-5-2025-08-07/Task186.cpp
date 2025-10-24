#include <bits/stdc++.h>
using namespace std;

struct ListNode {
    int val;
    ListNode* next;
    ListNode(int v) : val(v), next(nullptr) {}
};

ListNode* mergeKLists(vector<ListNode*>& lists) {
    struct Cmp {
        bool operator()(ListNode* a, ListNode* b) const { return a->val > b->val; }
    };
    priority_queue<ListNode*, vector<ListNode*>, Cmp> pq;
    for (auto n : lists) if (n) pq.push(n);
    ListNode dummy(0), *tail = &dummy;
    while (!pq.empty()) {
        ListNode* n = pq.top(); pq.pop();
        tail->next = n; tail = n;
        if (n->next) pq.push(n->next);
    }
    tail->next = nullptr;
    return dummy.next;
}

// Helpers
ListNode* buildList(const vector<int>& arr) {
    ListNode dummy(0), *t = &dummy;
    for (int v : arr) { t->next = new ListNode(v); t = t->next; }
    return dummy.next;
}

vector<ListNode*> buildLists(const vector<vector<int>>& arrs) {
    vector<ListNode*> res;
    res.reserve(arrs.size());
    for (auto& a : arrs) res.push_back(buildList(a));
    return res;
}

vector<int> toArray(ListNode* head) {
    vector<int> out;
    while (head) { out.push_back(head->val); head = head->next; }
    return out;
}

void freeList(ListNode* head) {
    while (head) { ListNode* n = head->next; delete head; head = n; }
}

int main() {
    vector<vector<vector<int>>> tests = {
        {{1,4,5},{1,3,4},{2,6}},
        { /* empty -> no lists */ },
        { {} },
        { {-10,-5,0,5}, {1,2,3}, {-10,-10,100} },
        { {1}, {0}, {-1,2,2}, {}, {3,3,3} }
    };
    for (auto& tcase : tests) {
        auto lists = buildLists(tcase);
        ListNode* merged = mergeKLists(lists);
        vector<int> arr = toArray(merged);
        // print
        cout << "[";
        for (size_t i = 0; i < arr.size(); ++i) {
            if (i) cout << ",";
            cout << arr[i];
        }
        cout << "]" << "\n";
        // cleanup
        for (auto l : lists) freeList(l);
        // merged nodes are already freed if they were in lists (shared), skip double free
    }
    return 0;
}