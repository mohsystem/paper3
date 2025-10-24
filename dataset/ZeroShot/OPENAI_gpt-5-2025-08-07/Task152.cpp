#include <bits/stdc++.h>
using namespace std;

class HashTable {
    struct Entry {
        string key;
        int value;
    };

    vector<list<Entry>> buckets;
    size_t sz;
    size_t threshold;
    double loadFactor;
    uint64_t salt;

    static constexpr uint64_t FNV64_OFFSET = 0xcbf29ce484222325ULL;
    static constexpr uint64_t FNV64_PRIME  = 0x100000001b3ULL;

    static size_t nextPowerOfTwo(size_t n) {
        size_t c = 1;
        while (c < n) c <<= 1;
        return c;
    }

    uint64_t fnv1a64(const string& s) const {
        uint64_t h = FNV64_OFFSET;
        for (unsigned char c : s) {
            h ^= c;
            h *= FNV64_PRIME;
        }
        return h;
    }

    size_t indexFor(const string& key) const {
        uint64_t h = fnv1a64(key) ^ salt;
        return static_cast<size_t>(h & (buckets.size() - 1));
    }

    void resizeIfNeeded() {
        if (sz <= threshold) return;
        size_t newCap = buckets.size() << 1;
        vector<list<Entry>> newBuckets(newCap);
        for (auto& bucket : buckets) {
            for (auto& e : bucket) {
                uint64_t h = (fnv1a64(e.key) ^ salt) & (newCap - 1);
                newBuckets[h].push_front({e.key, e.value});
            }
        }
        buckets.swap(newBuckets);
        threshold = static_cast<size_t>(newCap * loadFactor);
    }

public:
    HashTable(size_t capacity = 16, double lf = 0.75) : sz(0), loadFactor(lf) {
        if (capacity == 0) capacity = 16;
        capacity = nextPowerOfTwo(capacity);
        buckets.assign(capacity, {});
        threshold = static_cast<size_t>(capacity * loadFactor);
        std::random_device rd;
        uint64_t a = (static_cast<uint64_t>(rd()) << 32) ^ rd();
        uint64_t b = (static_cast<uint64_t>(rd()) << 32) ^ rd();
        salt = a ^ b;
    }

    // Inserts or updates. Returns true if new insert, false if updated.
    bool put(const string& key, int value) {
        if (key.empty()) {
            // Allow empty string as a valid key, but if you wish to forbid, handle here.
        }
        size_t idx = indexFor(key);
        for (auto& e : buckets[idx]) {
            if (e.key == key) {
                e.value = value;
                return false;
            }
        }
        buckets[idx].push_front({key, value});
        ++sz;
        resizeIfNeeded();
        return true;
    }

    // Deletes key. Returns true if deleted, false if not found.
    bool erase(const string& key) {
        size_t idx = indexFor(key);
        auto& lst = buckets[idx];
        for (auto it = lst.begin(); it != lst.end(); ++it) {
            if (it->key == key) {
                lst.erase(it);
                --sz;
                return true;
            }
        }
        return false;
    }

    // Searches key. Returns true and sets outValue if found; false otherwise.
    bool get(const string& key, int& outValue) const {
        size_t idx = indexFor(key);
        const auto& lst = buckets[idx];
        for (const auto& e : lst) {
            if (e.key == key) {
                outValue = e.value;
                return true;
            }
        }
        return false;
    }

    size_t size() const { return sz; }
};

int main() {
    HashTable ht;

    // Test case 1: insert and search
    ht.put("apple", 1);
    ht.put("banana", 2);
    ht.put("cherry", 3);
    int v = 0;
    bool found = ht.get("banana", v);
    cout << "T1 banana=" << (found ? to_string(v) : string("None")) << "\n"; // expect 2

    // Test case 2: update existing
    ht.put("banana", 20);
    found = ht.get("banana", v);
    cout << "T2 banana=" << (found ? to_string(v) : string("None")) << "\n"; // expect 20

    // Test case 3: delete and search
    bool deleted = ht.erase("apple");
    found = ht.get("apple", v);
    cout << "T3 apple_deleted=" << (deleted ? "true" : "false") << " val=" << (found ? to_string(v) : string("None")) << "\n"; // expect true, None

    // Test case 4: search non-existent
    found = ht.get("durian", v);
    cout << "T4 durian=" << (found ? to_string(v) : string("None")) << "\n"; // expect None

    // Test case 5: insert more and verify
    ht.put("elderberry", 5);
    ht.put("fig", 6);
    found = ht.get("elderberry", v);
    cout << "T5 elderberry=" << (found ? to_string(v) : string("None")) << " size=" << ht.size() << "\n"; // expect 5 and size

    return 0;
}