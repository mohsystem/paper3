
#include <iostream>
#include <vector>
using namespace std;

struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

class Task186 {
public:
    ListNode* mergeKLists(vector<ListNode*>& lists) {
        if (lists.empty()) {
            return nullptr;
        }
        
        return mergeKListsHelper(lists, 0, lists.size() - 1);
    }
    
private:
    ListNode* mergeKListsHelper(vector<ListNode*>& lists, int left, int right) {
        if (left == right) {
            return lists[left];
        }
        
        if (left < right) {
            int mid = left + (right - left) / 2;
            ListNode* l1 = mergeKListsHelper(lists, left, mid);
            ListNode* l2 = mergeKListsHelper(lists, mid + 1, right);
            return mergeTwoLists(l1, l2);
        }
        
        return nullptr;
    }
    
    ListNode* mergeTwoLists(ListNode* l1, ListNode* l2) {
        ListNode dummy(0);
        ListNode* current = &dummy;
        
        while (l1 && l2) {
            if (l1->val <= l2->val) {
                current->next = l1;
                l1 = l1->next;
            } else {
                current->next = l2;
                l2 = l2->next;
            }
            current = current->next;
        }
        
        if (l1) {
            current->next = l1;
        }
        if (l2) {
            current->next = l2;
        }
        
        return dummy.next;
    }
};

ListNode* createList(vector<int> arr) {
    if (arr.empty()) {
        return nullptr;
    }
    ListNode dummy(0);
    ListNode* current = &dummy;
    for (int val : arr) {
        current->next = new ListNode(val);
        current = current->next;
    }
    return dummy.next;
}

void printList(ListNode* head) {
    cout << "[";
    while (head) {
        cout << head->val;
        if (head->next) {
            cout << ",";
        }
        head = head->next;
    }
    cout << "]" << endl;
}

int main() {
    Task186 solution;
    
    // Test case 1
    vector<ListNode*> lists1 = {
        createList({1, 4, 5}),
        createList({1, 3, 4}),
        createList({2, 6})
    };
    cout << "Test 1: ";
    printList(solution.mergeKLists(lists1));
    
    // Test case 2
    vector<ListNode*> lists2 = {};
    cout << "Test 2: ";
    printList(solution.mergeKLists(lists2));
    
    // Test case 3
    vector<ListNode*> lists3 = {nullptr};
    cout << "Test 3: ";
    printList(solution.mergeKLists(lists3));
    
    // Test case 4
    vector<ListNode*> lists4 = {
        createList({1, 2, 3}),
        createList({4, 5, 6})
    };
    cout << "Test 4: ";
    printList(solution.mergeKLists(lists4));
    
    // Test case 5
    vector<ListNode*> lists5 = {
        createList({-2, -1, 0}),
        createList({-3}),
        createList({1, 2})
    };
    cout << "Test 5: ";
    printList(solution.mergeKLists(lists5));
    
    return 0;
}
