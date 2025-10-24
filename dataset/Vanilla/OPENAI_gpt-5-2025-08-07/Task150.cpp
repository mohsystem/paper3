#include <bits/stdc++.h>
using namespace std;

class SinglyLinkedList {
    struct Node {
        int val;
        Node* next;
        Node(int v): val(v), next(nullptr) {}
    };
    Node* head;

public:
    SinglyLinkedList(): head(nullptr) {}

    // Insert at the end
    void insert(int value) {
        if (!head) {
            head = new Node(value);
            return;
        }
        Node* cur = head;
        while (cur->next) cur = cur->next;
        cur->next = new Node(value);
    }

    // Delete first occurrence
    bool remove(int value) {
        Node* cur = head;
        Node* prev = nullptr;
        while (cur) {
            if (cur->val == value) {
                if (!prev) head = cur->next;
                else prev->next = cur->next;
                delete cur;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }

    // Search
    bool search(int value) const {
        Node* cur = head;
        while (cur) {
            if (cur->val == value) return true;
            cur = cur->next;
        }
        return false;
    }

    string toString() const {
        string s = "[";
        Node* cur = head;
        while (cur) {
            s += to_string(cur->val);
            if (cur->next) s += ", ";
            cur = cur->next;
        }
        s += "]";
        return s;
    }

    // For API similarity with the prompt
    bool deleteValue(int value) { return remove(value); }

    ~SinglyLinkedList() {
        Node* cur = head;
        while (cur) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
    }
};

int main() {
    // Test 1: Insert sequence
    SinglyLinkedList list;
    list.insert(10);
    list.insert(20);
    list.insert(30);
    cout << "Test1 list after inserts: " << list.toString() << "\n";

    // Test 2: Search existing
    cout << "Test2 search 20: " << (list.search(20) ? "1" : "0") << "\n";

    // Test 3: Delete middle
    cout << "Test3 delete 20: " << (list.deleteValue(20) ? "1" : "0") << ", list: " << list.toString() << "\n";

    // Test 4: Delete head and tail
    cout << "Test4 delete 10: " << (list.deleteValue(10) ? "1" : "0")
         << ", delete 30: " << (list.deleteValue(30) ? "1" : "0")
         << ", list: " << list.toString() << "\n";

    // Test 5: Delete on empty and search non-existing
    cout << "Test5 delete 99 on empty: " << (list.deleteValue(99) ? "1" : "0")
         << ", search 99: " << (list.search(99) ? "1" : "0") << "\n";

    return 0;
}