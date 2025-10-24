#include <iostream>
#include <vector>
#include <utility>
#include <cstdint>

struct SearchResult {
    bool found;
    int value;
};

class HashTable {
public:
    explicit HashTable(std::size_t capacity = 17) : cap_(capacity), buckets_(capacity) {
        if (cap_ == 0) cap_ = 17;
        buckets_.assign(cap_, {});
    }

    // Inserts key->value. Returns true if inserted new, false if updated existing.
    bool insert(int key, int value) {
        std::size_t idx = index(key);
        auto &bucket = buckets_[idx];
        for (auto &kv : bucket) {
            if (kv.first == key) {
                kv.second = value;
                return false;
            }
        }
        bucket.emplace_back(key, value);
        return true;
    }

    // Deletes key. Returns true if existed and removed, false otherwise.
    bool erase(int key) {
        std::size_t idx = index(key);
        auto &bucket = buckets_[idx];
        for (std::size_t i = 0; i < bucket.size(); ++i) {
            if (bucket[i].first == key) {
                bucket.erase(bucket.begin() + static_cast<long>(i));
                return true;
            }
        }
        return false;
    }

    // Searches key. Returns SearchResult {found, value}
    SearchResult search(int key) const {
        std::size_t idx = index(key);
        const auto &bucket = buckets_[idx];
        for (const auto &kv : bucket) {
            if (kv.first == key) {
                return {true, kv.second};
            }
        }
        return {false, 0};
    }

private:
    std::size_t cap_;
    std::vector<std::vector<std::pair<int,int>>> buckets_;

    std::size_t index(int key) const {
        std::uint32_t h = static_cast<std::uint32_t>(key) * 0x9E3779B1u;
        h ^= (h >> 16);
        return static_cast<std::size_t>(h) % cap_;
    }
};

int main() {
    HashTable ht(17);

    // Test 1: Insert and search basic
    bool t1_insert = ht.insert(1, 100) && ht.insert(2, 200) && ht.insert(3, 300);
    SearchResult s1 = ht.search(1), s2 = ht.search(2), s3 = ht.search(3);
    bool t1_search = (s1.found && s1.value == 100) && (s2.found && s2.value == 200) && (s3.found && s3.value == 300);
    std::cout << "Test1 Insert new keys: " << (t1_insert ? "true" : "false") << "\n";
    std::cout << "Test1 Search keys: " << (t1_search ? "true" : "false") << "\n";

    // Test 2: Update existing key
    bool updated = !ht.insert(2, 250); // update
    SearchResult s2b = ht.search(2);
    std::cout << "Test2 Update existing: " << ((updated && s2b.found && s2b.value == 250) ? "true" : "false") << "\n";

    // Test 3: Delete existing key
    bool del3 = ht.erase(3);
    SearchResult s3b = ht.search(3);
    std::cout << "Test3 Delete existing: " << ((del3 && !s3b.found) ? "true" : "false") << "\n";

    // Test 4: Delete non-existing key
    bool del99 = ht.erase(99);
    std::cout << "Test4 Delete non-existing: " << ((!del99) ? "true" : "false") << "\n";

    // Test 5: Search non-existing key after sequence
    SearchResult sNF = ht.search(42);
    std::cout << "Test5 Search non-existing: " << ((!sNF.found) ? "true" : "false") << "\n";

    return 0;
}