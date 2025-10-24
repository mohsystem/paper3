
#include <iostream>
#include <vector>
#include <queue>
using namespace std;

struct ListNode {
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(nullptr) {}
};

class Task186 {
public:
    struct Compare {
        bool operator()(ListNode* a, ListNode* b) {
            return a->val > b->val;
        }
    };
    
    static ListNode* mergeKLists(vector<ListNode*>& lists) {
        if (lists.empty()) {
            return nullptr;
        }
        
        priority_queue<ListNode*, vector<ListNode*>, Compare> pq;
        
        for (ListNode* node : lists) {
            if (node != nullptr) {
                pq.push(node);
            }
        }
        
        ListNode* dummy = new ListNode(0);
        ListNode* current = dummy;
        
        while (!pq.empty()) {
            ListNode* node = pq.top();
            pq.pop();
            current->next = node;
            current = current->next;
            
            if (node->next != nullptr) {
                pq.push(node->next);
            }
        }
        
        ListNode* result = dummy->next;
        delete dummy;
        return result;
    }
    
    static ListNode* createList(vector<int> arr) {
        if (arr.empty()) return nullptr;
        ListNode* dummy = new ListNode(0);
        ListNode* current = dummy;
        for (int val : arr) {
            current->next = new ListNode(val);
            current = current->next;
        }
        ListNode* result = dummy->next;
        delete dummy;
        return result;
    }
    
    static void printList(ListNode* head) {
        cout << "[";
        while (head != nullptr) {
            cout << head->val;
            if (head->next != nullptr) cout << ",";
            head = head->next;
        }
        cout << "]" << endl;
    }
};

int main() {
    // Test case 1
    vector<ListNode*> lists1 = {Task186::createList({1,4,5}), Task186::createList({1,3,4}), Task186::createList({2,6})};
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
    vector<ListNode*> lists4 = {Task186::createList({1,2,3}), Task186::createList({4,5,6})};
    cout << "Test 4: ";
    Task186::printList(Task186::mergeKLists(lists4));
    
    // Test case 5
    vector<ListNode*> lists5 = {Task186::createList({-2,-1,0}), Task186::createList({-3}), Task186::createList({1,2})};
    cout << "Test 5: ";
    Task186::printList(Task186::mergeKLists(lists5));
    
    return 0;
}
