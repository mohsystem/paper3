#include <iostream>
#include <vector>
#include <new>

class SinglyLinkedList {
private:
    struct Node {
        int val;
        Node* next;
        explicit Node(int v) : val(v), next(nullptr) {}
    };
    Node* head;
    Node* tail;
    std::size_t sz;

public:
    SinglyLinkedList() : head(nullptr), tail(nullptr), sz(0) {}

    bool insert(int value) {
        Node* n = new (std::nothrow) Node(value);
        if (n == nullptr) {
            return false;
        }
        if (head == nullptr) {
            head = n;
            tail = n;
        } else {
            tail->next = n;
            tail = n;
        }
        ++sz;
        return true;
    }

    bool deleteValue(int value) {
        Node* prev = nullptr;
        Node* curr = head;
        while (curr != nullptr) {
            if (curr->val == value) {
                if (prev == nullptr) {
                    head = curr->next;
                } else {
                    prev->next = curr->next;
                }
                if (curr == tail) {
                    tail = prev;
                }
                delete curr;
                --sz;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    bool search(int value) const {
        Node* curr = head;
        while (curr != nullptr) {
            if (curr->val == value) return true;
            curr = curr->next;
        }
        return false;
    }

    std::size_t size() const { return sz; }

    std::vector<int> toVector() const {
        std::vector<int> out;
        out.reserve(sz);
        Node* curr = head;
        while (curr != nullptr) {
            out.push_back(curr->val);
            curr = curr->next;
        }
        return out;
    }

    ~SinglyLinkedList() {
        Node* curr = head;
        while (curr != nullptr) {
            Node* next = curr->next;
            delete curr;
            curr = next;
        }
        head = nullptr;
        tail = nullptr;
        sz = 0;
    }
};

static void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

int main() {
    // Test Case 1
    SinglyLinkedList l1;
    l1.insert(1); l1.insert(2); l1.insert(3);
    std::cout << "TC1 search 2: " << (l1.search(2) ? "true" : "false") << "\n";
    std::cout << "TC1 delete 2: " << (l1.deleteValue(2) ? "true" : "false") << "\n";
    std::cout << "TC1 search 2: " << (l1.search(2) ? "true" : "false") << "\n";
    std::cout << "TC1 list: "; printVec(l1.toVector());

    // Test Case 2: delete head
    SinglyLinkedList l2;
    l2.insert(10); l2.insert(20);
    std::cout << "TC2 delete 10: " << (l2.deleteValue(10) ? "true" : "false") << "\n";
    std::cout << "TC2 list: "; printVec(l2.toVector());

    // Test Case 3: delete tail
    SinglyLinkedList l3;
    l3.insert(7); l3.insert(8); l3.insert(9);
    std::cout << "TC3 delete 9: " << (l3.deleteValue(9) ? "true" : "false") << "\n";
    std::cout << "TC3 list: "; printVec(l3.toVector());

    // Test Case 4: delete non-existent
    SinglyLinkedList l4;
    l4.insert(5);
    std::cout << "TC4 delete 6: " << (l4.deleteValue(6) ? "true" : "false") << "\n";
    std::cout << "TC4 list: "; printVec(l4.toVector());

    // Test Case 5: search empty
    SinglyLinkedList l5;
    std::cout << "TC5 search 42: " << (l5.search(42) ? "true" : "false") << "\n";
    std::cout << "TC5 list: "; printVec(l5.toVector());

    return 0;
}