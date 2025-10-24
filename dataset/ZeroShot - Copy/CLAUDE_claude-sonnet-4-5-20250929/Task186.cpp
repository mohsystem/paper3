
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
    static ListNode* mergeKLists(vector<ListNode*>& lists) {
        if (lists.empty()) {
            return nullptr;
        }
        
        return mergeKListsHelper(lists, 0, lists.size() - 1);
    }
    
private:
    static ListNode* mergeKListsHelper(vector<ListNode*>& lists, int left, int right) {
        if (left == right) {
            return lists[left];
        }
        
        if (left > right) {
            return nullptr;
        }
        
        int mid = left + (right - left) / 2;
        ListNode* l1 = mergeKListsHelper(lists, left, mid);
        ListNode* l2 = mergeKListsHelper(lists, mid + 1, right);
        
        return mergeTwoLists(l1, l2);
    }
    
    static ListNode* mergeTwoLists(ListNode* l1, ListNode* l2) {
        ListNode* dummy = new ListNode(0);
        ListNode* current = dummy;
        
        while (l1 != nullptr && l2 != nullptr) {
            if (l1->val <= l2->val) {
                current->next = l1;
                l1 = l1->next;
            } else {
                current->next = l2;
                l2 = l2->next;
            }
            current = current->next;
        }
        
        current->next = (l1 != nullptr) ? l1 : l2;
        
        ListNode* result = dummy->next;
        delete dummy;
        return result;
    }
    
    static ListNode* createList(vector<int> arr) {
        if (arr.empty()) {
            return nullptr;
        }
        ListNode* head = new ListNode(arr[0]);
        ListNode* current = head;
        for (size_t i = 1; i < arr.size(); i++) {
            current->next = new ListNode(arr[i]);
            current = current->next;
        }
        return head;
    }
    
    static void printList(ListNode* head) {
        cout << "[";
        while (head != nullptr) {
            cout << head->val;
            if (head->next != nullptr) {
                cout << ",";
            }
            head = head->next;
        }
        cout << "]" << endl;
    }
};

int main() {
    // Test case 1
    vector<ListNode*> lists1 = {
        Task186::createList({1, 4, 5}),
        Task186::createList({1, 3, 4}),
        Task186::createList({2, 6})
    };
    cout << "Test 1: ";
    Task186::printList(Task186::mergeKLists(lists1));
    
    // Test case 2
    vector<ListNode*> lists2 = {};
    cout << "Test 2: ";
    Task186::printList(Task186::mergeKLists(lists2));
    
    // Test case 3
    vector<ListNode*> lists3 = {nullptr};
    cout << "Test 3: ";
    Task186::printList(Task186::mergeKLists(lists3));
    
    // Test case 4
    vector<ListNode*> lists4 = {
        Task186::createList({1, 2, 3}),
        Task186::createList({4, 5, 6})
    };
    cout << "Test 4: ";
    Task186::printList(Task186::mergeKLists(lists4));
    
    // Test case 5
    vector<ListNode*> lists5 = {
        Task186::createList({-2, -1}),
        Task186::createList({-3, 1, 4}),
        Task186::createList({0, 2}),
        Task186::createList({-1, 3})
    };
    cout << "Test 5: ";
    Task186::printList(Task186::mergeKLists(lists5));
    
    return 0;
}
