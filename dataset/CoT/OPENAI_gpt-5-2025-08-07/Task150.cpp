// Chain-of-Through process in code generation:
// 1) Problem understanding: Singly linked list with insert, delete, and search operations.
// 2) Security requirements: Validate indices, handle empty list and null pointers.
// 3) Secure coding generation: Use RAII for memory cleanup; check all operations.
// 4) Code review: Verified pointer safety and size consistency.
// 5) Secure code output: Final safe implementation with 5 tests.

#include <iostream>
#include <string>
#include <sstream>

class SinglyLinkedList {
private:
    struct Node {
        int val;
        Node* next;
        explicit Node(int v) : val(v), next(nullptr) {}
    };
    Node* head;
    int sz;

public:
    SinglyLinkedList() : head(nullptr), sz(0) {}
    ~SinglyLinkedList() {
        Node* cur = head;
        while (cur) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        head = nullptr;
        sz = 0;
    }

    int size() const { return sz; }

    void insertAtHead(int value) {
        Node* n = new Node(value);
        n->next = head;
        head = n;
        ++sz;
    }

    void insertAtTail(int value) {
        Node* n = new Node(value);
        if (!head) {
            head = n;
        } else {
            Node* cur = head;
            while (cur->next) cur = cur->next;
            cur->next = n;
        }
        ++sz;
    }

    bool insertAtIndex(int index, int value) {
        if (index < 0 || index > sz) return false;
        if (index == 0) {
            insertAtHead(value);
            return true;
        }
        Node* prev = head;
        for (int i = 0; i < index - 1; ++i) {
            if (!prev) return false;
            prev = prev->next;
        }
        Node* n = new Node(value);
        n->next = prev->next;
        prev->next = n;
        ++sz;
        return true;
    }

    bool deleteByValue(int value) {
        Node* prev = nullptr;
        Node* cur = head;
        while (cur && cur->val != value) {
            prev = cur;
            cur = cur->next;
        }
        if (!cur) return false;
        if (!prev) head = cur->next;
        else prev->next = cur->next;
        delete cur;
        --sz;
        return true;
    }

    bool deleteAtIndex(int index) {
        if (index < 0 || index >= sz) return false;
        if (index == 0) {
            if (head) {
                Node* tmp = head;
                head = head->next;
                delete tmp;
                --sz;
                return true;
            }
            return false;
        }
        Node* prev = head;
        for (int i = 0; i < index - 1; ++i) {
            if (!prev) return false;
            prev = prev->next;
        }
        if (!prev || !prev->next) return false;
        Node* toDel = prev->next;
        prev->next = toDel->next;
        delete toDel;
        --sz;
        return true;
    }

    int indexOf(int value) const {
        int idx = 0;
        Node* cur = head;
        while (cur) {
            if (cur->val == value) return idx;
            cur = cur->next;
            ++idx;
        }
        return -1;
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << "[";
        Node* cur = head;
        while (cur) {
            oss << cur->val;
            if (cur->next) oss << " -> ";
            cur = cur->next;
        }
        oss << "]";
        return oss.str();
    }
};

int main() {
    // 5 Test cases
    SinglyLinkedList list;

    // Test 1: Insert at head and tail
    list.insertAtHead(3);
    list.insertAtTail(5);
    list.insertAtTail(7);
    std::cout << "Test1: " << list.toString() << " | size=" << list.size() << "\n";

    // Test 2: Insert at index
    bool insIdx = list.insertAtIndex(1, 4); // [3,4,5,7]
    std::cout << "Test2: insertAtIndex(1,4)=" << (insIdx ? "true" : "false") << " -> " << list.toString() << " | size=" << list.size() << "\n";

    // Test 3: Search
    std::cout << "Test3: indexOf(5)=" << list.indexOf(5) << ", indexOf(9)=" << list.indexOf(9) << "\n";

    // Test 4: Delete by value
    bool delVal1 = list.deleteByValue(4);
    bool delVal2 = list.deleteByValue(42);
    std::cout << "Test4: deleteByValue(4)=" << (delVal1 ? "true" : "false") << ", deleteByValue(42)=" << (delVal2 ? "true" : "false") << " -> " << list.toString() << " | size=" << list.size() << "\n";

    // Test 5: Delete at index
    bool delIdx = list.deleteAtIndex(1);
    std::cout << "Test5: deleteAtIndex(1)=" << (delIdx ? "true" : "false") << " -> " << list.toString() << " | size=" << list.size() << "\n";

    return 0;
}