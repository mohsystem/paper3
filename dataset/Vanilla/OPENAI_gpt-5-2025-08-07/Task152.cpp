#include <bits/stdc++.h>
using namespace std;

class Task152 {
public:
    Task152(size_t capacity = 16) {
        cap = max<size_t>(4, capacity);
        table.assign(cap, {});
        sz = 0;
    }

    void insert(int key, int value) {
        size_t idx = index(key);
        for (auto &p : table[idx]) {
            if (p.first == key) {
                p.second = value;
                return;
            }
        }
        table[idx].emplace_back(key, value);
        ++sz;
    }

    pair<bool, int> search(int key) const {
        size_t idx = index(key);
        for (auto const &p : table[idx]) {
            if (p.first == key) return {true, p.second};
        }
        return {false, 0};
    }

    bool erase(int key) {
        size_t idx = index(key);
        auto &bucket = table[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                bucket.erase(it);
                --sz;
                return true;
            }
        }
        return false;
    }

    size_t size() const { return sz; }

private:
    vector<vector<pair<int,int>>> table;
    size_t cap;
    size_t sz;

    size_t index(int key) const {
        size_t h = std::hash<int>{}(key);
        h ^= (h >> 16);
        return h % cap;
    }
};

int main() {
    Task152 ht(16);

    // Test 1: Insert and search
    ht.insert(1, 10);
    auto r1 = ht.search(1);
    cout << "Test1 search(1): " << (r1.first ? to_string(r1.second) : string("None")) << "\n";

    // Test 2: Collision insert and search
    ht.insert(17, 20); // likely collides with 1 if capacity 16
    auto r2 = ht.search(17);
    cout << "Test2 search(17): " << (r2.first ? to_string(r2.second) : string("None")) << "\n";

    // Test 3: Update existing key
    ht.insert(1, 15);
    auto r3 = ht.search(1);
    cout << "Test3 search(1): " << (r3.first ? to_string(r3.second) : string("None")) << "\n";

    // Test 4: Delete existing key and verify
    cout << "Test4 erase(17): " << (ht.erase(17) ? "true" : "false") << "\n";
    auto r4 = ht.search(17);
    cout << "Test4 search(17): " << (r4.first ? to_string(r4.second) : string("None")) << "\n";

    // Test 5: Delete non-existing key and size check
    cout << "Test5 erase(99): " << (ht.erase(99) ? "true" : "false") << "\n";
    cout << "Test5 size(): " << ht.size() << "\n";

    return 0;
}