#include <iostream>
#include <limits>

class Task149 {
public:
    struct Node {
        int value;
        Node* next;
        explicit Node(int v) : value(v), next(nullptr) {}
    };

    class SafeQueue {
    public:
        SafeQueue() : head(nullptr), tail(nullptr), sz(0) {}
        ~SafeQueue() { clear(); }

        SafeQueue(const SafeQueue&) = delete;
        SafeQueue& operator=(const SafeQueue&) = delete;

        // Enqueue: returns true on success, false on allocation failure or size overflow
        bool enqueue(int value) {
            if (sz == std::numeric_limits<size_t>::max()) {
                return false; // prevent overflow
            }
            Node* n = new (std::nothrow) Node(value);
            if (!n) return false;
            if (!tail) {
                head = tail = n;
            } else {
                tail->next = n;
                tail = n;
            }
            ++sz;
            return true;
        }

        // Dequeue: returns true and sets out on success, false if empty
        bool dequeue(int& out) {
            if (!head) return false;
            Node* n = head;
            out = n->value;
            head = head->next;
            if (!head) tail = nullptr;
            delete n;
            --sz;
            return true;
        }

        // Peek: returns true and sets out on success, false if empty
        bool peek(int& out) const {
            if (!head) return false;
            out = head->value;
            return true;
        }

        bool isEmpty() const { return head == nullptr; }
        size_t size() const { return sz; }

        void clear() {
            while (head) {
                Node* n = head;
                head = head->next;
                delete n;
            }
            tail = nullptr;
            sz = 0;
        }

    private:
        Node* head;
        Node* tail;
        size_t sz;
    };
};

int main() {
    using std::cout;
    using std::endl;

    // Test 1
    Task149::SafeQueue q1;
    q1.enqueue(1);
    q1.enqueue(2);
    q1.enqueue(3);
    int out;
    if (q1.peek(out)) cout << "Test1 Peek (expect 1): " << out << endl; else cout << "Test1 Peek (expect 1): null" << endl;
    if (q1.dequeue(out)) cout << "Test1 Dequeue (expect 1): " << out << endl; else cout << "Test1 Dequeue (expect 1): null" << endl;
    if (q1.peek(out)) cout << "Test1 Peek (expect 2): " << out << endl; else cout << "Test1 Peek (expect 2): null" << endl;
    cout << "Test1 Size (expect 2): " << q1.size() << endl;

    // Test 2
    Task149::SafeQueue q2;
    if (q2.dequeue(out)) cout << "Test2 Dequeue empty (expect null): " << out << endl; else cout << "Test2 Dequeue empty (expect null): null" << endl;
    if (q2.peek(out)) cout << "Test2 Peek empty (expect null): " << out << endl; else cout << "Test2 Peek empty (expect null): null" << endl;
    cout << "Test2 Size (expect 0): " << q2.size() << endl;

    // Test 3
    Task149::SafeQueue q3;
    q3.enqueue(10);
    if (q3.dequeue(out)) cout << "Test3 Dequeue (expect 10): " << out << endl; else cout << "Test3 Dequeue (expect 10): null" << endl;
    q3.enqueue(20);
    q3.enqueue(30);
    if (q3.peek(out)) cout << "Test3 Peek (expect 20): " << out << endl; else cout << "Test3 Peek (expect 20): null" << endl;
    cout << "Test3 Size (expect 2): " << q3.size() << endl;

    // Test 4
    Task149::SafeQueue q4;
    q4.enqueue(-5);
    q4.enqueue(0);
    if (q4.dequeue(out)) cout << "Test4 Dequeue (expect -5): " << out << endl; else cout << "Test4 Dequeue (expect -5): null" << endl;
    if (q4.peek(out)) cout << "Test4 Peek (expect 0): " << out << endl; else cout << "Test4 Peek (expect 0): null" << endl;
    cout << "Test4 Size (expect 1): " << q4.size() << endl;

    // Test 5
    Task149::SafeQueue q5;
    for (int i = 100; i < 105; ++i) q5.enqueue(i);
    if (q5.dequeue(out)) cout << "Test5 Dequeue1 (expect 100): " << out << endl; else cout << "Test5 Dequeue1 (expect 100): null" << endl;
    if (q5.dequeue(out)) cout << "Test5 Dequeue2 (expect 101): " << out << endl; else cout << "Test5 Dequeue2 (expect 101): null" << endl;
    if (q5.dequeue(out)) cout << "Test5 Dequeue3 (expect 102): " << out << endl; else cout << "Test5 Dequeue3 (expect 102): null" << endl;
    if (q5.peek(out)) cout << "Test5 Peek (expect 103): " << out << endl; else cout << "Test5 Peek (expect 103): null" << endl;
    cout << "Test5 Size (expect 2): " << q5.size() << endl;

    return 0;
}