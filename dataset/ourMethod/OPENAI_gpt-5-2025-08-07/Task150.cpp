#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <limits>

class SinglyLinkedList {
private:
    struct Node {
        int value;
        std::unique_ptr<Node> next;
        explicit Node(int v) : value(v), next(nullptr) {}
    };
    std::unique_ptr<Node> head;
    std::size_t sz;

public:
    SinglyLinkedList() : head(nullptr), sz(0) {}

    std::size_t size() const { return sz; }

    bool insertAtHead(int value) {
        auto n = std::make_unique<Node>(value);
        n->next = std::move(head);
        head = std::move(n);
        ++sz;
        return true;
    }

    bool insertAtTail(int value) {
        auto n = std::make_unique<Node>(value);
        if (!head) {
            head = std::move(n);
            sz = 1;
            return true;
        }
        Node* cur = head.get();
        while (cur->next) cur = cur->next.get();
        cur->next = std::move(n);
        ++sz;
        return true;
    }

    bool insertAtIndex(std::size_t index, int value) {
        if (index > sz) return false;
        if (index == 0) return insertAtHead(value);
        Node* prev = head.get();
        for (std::size_t i = 0; i < index - 1; ++i) {
            prev = prev->next.get();
        }
        auto n = std::make_unique<Node>(value);
        n->next = std::move(prev->next);
        prev->next = std::move(n);
        ++sz;
        return true;
    }

    bool deleteValue(int value) {
        std::unique_ptr<Node>* link = &head;
        while (link->get() && (*link)->value != value) {
            link = &((*link)->next);
        }
        if (!link->get()) return false;
        auto toDelete = std::move(*link);
        *link = std::move(toDelete->next);
        --sz;
        return true;
    }

    struct RemoveResult {
        bool ok;
        int value;
    };

    RemoveResult deleteAtIndex(std::size_t index) {
        if (index >= sz) return {false, 0};
        std::unique_ptr<Node>* link = &head;
        for (std::size_t i = 0; i < index; ++i) {
            link = &((*link)->next);
        }
        int val = (*link)->value;
        auto toDelete = std::move(*link);
        *link = std::move(toDelete->next);
        --sz;
        return {true, val};
    }

    int search(int value) const {
        const Node* cur = head.get();
        int idx = 0;
        while (cur) {
            if (cur->value == value) return idx;
            cur = cur->next.get();
            ++idx;
        }
        return -1;
    }

    std::vector<int> toVector() const {
        std::vector<int> out;
        out.reserve(sz);
        const Node* cur = head.get();
        while (cur) {
            out.push_back(cur->value);
            cur = cur->next.get();
        }
        return out;
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << "[";
        auto v = toVector();
        for (std::size_t i = 0; i < v.size(); ++i) {
            if (i) oss << ",";
            oss << v[i];
        }
        oss << "]";
        return oss.str();
    }
};

int main() {
    SinglyLinkedList list;

    // Test 1: Insert at tail and basic search/delete
    list.insertAtTail(1);
    list.insertAtTail(2);
    list.insertAtTail(3);
    std::cout << "Test1 list: " << list.toString() << "\n";
    std::cout << "Test1 search(2): " << list.search(2) << "\n";
    std::cout << "Test1 deleteValue(2): " << (list.deleteValue(2) ? "true" : "false")
              << " => " << list.toString() << " search(2): " << list.search(2) << "\n";

    // Test 2: Insert at head
    std::cout << "Test2 insertAtHead(10): " << (list.insertAtHead(10) ? "true" : "false")
              << " => " << list.toString() << "\n";

    // Test 3: Insert at index valid and invalid
    std::cout << "Test3 insertAtIndex(1,99): " << (list.insertAtIndex(1, 99) ? "true" : "false")
              << " => " << list.toString() << "\n";
    std::cout << "Test3 insertAtIndex(size+2,5): "
              << (list.insertAtIndex(list.size() + 2, 5) ? "true" : "false")
              << " => " << list.toString() << "\n";

    // Test 4: Delete at index valid and invalid
    auto rem0 = list.deleteAtIndex(0);
    std::cout << "Test4 deleteAtIndex(0): " << (rem0.ok ? std::to_string(rem0.value) : "null")
              << " => " << list.toString() << "\n";
    auto remBad = list.deleteAtIndex(list.size());
    std::cout << "Test4 deleteAtIndex(size): " << (remBad.ok ? std::to_string(remBad.value) : "null")
              << " => " << list.toString() << "\n";

    // Test 5: Search non-existing
    std::cout << "Test5 search(100): " << list.search(100) << "\n";

    return 0;
}