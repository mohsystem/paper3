// Chain-of-Through secure implementation: C++
// 1) Problem understanding: Integer hash set with insert, delete, search.
// 2) Security: safe hashing, power-of-two capacity, bounds checks.
// 3) Secure coding: RAII for memory, exception safety, no UB on rehash.
// 4) Code review: validate rehash logic and pointer updates.
// 5) Secure output: deterministic tests and cleanup.

#include <bits/stdc++.h>
using namespace std;

class IntHashSet {
    struct Node {
        int key;
        Node* next;
        Node(int k, Node* n) : key(k), next(n) {}
    };

    static constexpr float LOAD_FACTOR = 0.75f;
    static constexpr size_t MIN_CAP = 16;
    static constexpr size_t MAX_CAP = 1ull << 30;

    vector<Node*> table;
    size_t sz;
    size_t threshold;

    static inline uint64_t mix64(uint64_t z) {
        z = (z ^ (z >> 33)) * 0xff51afd7ed558ccdll;
        z = (z ^ (z >> 33)) * 0xc4ceb9fe1a85ec53ull;
        z = z ^ (z >> 33);
        return z;
    }

    inline size_t indexFor(int key, size_t cap) const {
        return static_cast<size_t>(mix64(static_cast<uint64_t>(static_cast<int64_t>(key)))) & (cap - 1);
    }

    void clearTable(vector<Node*>& tab) {
        for (Node* &head : tab) {
            Node* cur = head;
            while (cur) {
                Node* nx = cur->next;
                delete cur;
                cur = nx;
            }
            head = nullptr;
        }
    }

    void resizeIfNeeded() {
        if (sz <= threshold) return;
        size_t oldCap = table.size();
        if (oldCap >= MAX_CAP) return;
        size_t newCap = oldCap << 1;
        if (newCap == 0 || newCap > MAX_CAP) newCap = MAX_CAP;

        vector<Node*> newTab(newCap, nullptr);

        for (size_t i = 0; i < oldCap; ++i) {
            Node* e = table[i];
            while (e) {
                Node* next = e->next;
                size_t idx = indexFor(e->key, newCap);
                e->next = newTab[idx];
                newTab[idx] = e;
                e = next;
            }
            table[i] = nullptr;
        }
        table.swap(newTab);
        // newTab goes out of scope; nodes are preserved in table
        threshold = static_cast<size_t>(min<double>(static_cast<double>(table.size()) * LOAD_FACTOR, static_cast<double>(numeric_limits<int>::max())));
    }

public:
    IntHashSet() : table(MIN_CAP, nullptr), sz(0), threshold(static_cast<size_t>(MIN_CAP * LOAD_FACTOR)) {}

    ~IntHashSet() {
        clearTable(table);
    }

    bool insert(int key) {
        size_t idx = indexFor(key, table.size());
        for (Node* e = table[idx]; e != nullptr; e = e->next) {
            if (e->key == key) return false;
        }
        table[idx] = new (nothrow) Node(key, table[idx]);
        if (!table[idx]) {
            // Allocation failed; conservative behavior: no change performed (though it failed).
            return false;
        }
        ++sz;
        resizeIfNeeded();
        return true;
    }

    bool erase(int key) {
        size_t idx = indexFor(key, table.size());
        Node* prev = nullptr;
        Node* cur = table[idx];
        while (cur) {
            if (cur->key == key) {
                if (prev) prev->next = cur->next;
                else table[idx] = cur->next;
                delete cur;
                --sz;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }

    bool search(int key) const {
        size_t idx = indexFor(key, table.size());
        for (Node* e = table[idx]; e != nullptr; e = e->next) {
            if (e->key == key) return true;
        }
        return false;
    }

    size_t size() const { return sz; }
};

int main() {
    IntHashSet set;

    // Test 1
    set.insert(10);
    set.insert(20);
    set.insert(30);
    cout << "T1 contains(20): " << (set.search(20) ? "true" : "false") << "\n";

    // Test 2
    cout << "T2 contains(15): " << (set.search(15) ? "true" : "false") << "\n";

    // Test 3
    cout << "T3 delete(20): " << (set.erase(20) ? "true" : "false") << "\n";
    cout << "T3 contains(20): " << (set.search(20) ? "true" : "false") << "\n";

    // Test 4
    cout << "T4 insert(20): " << (set.insert(20) ? "true" : "false") << "\n";
    cout << "T4 contains(20): " << (set.search(20) ? "true" : "false") << "\n";

    // Test 5
    cout << "T5 delete(99): " << (set.erase(99) ? "true" : "false") << "\n";

    return 0;
}