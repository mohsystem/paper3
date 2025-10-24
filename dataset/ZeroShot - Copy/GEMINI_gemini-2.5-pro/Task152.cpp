#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <utility>    // for std::pair
#include <functional> // for std::hash
#include <optional>   // for std::optional

template <typename K, typename V>
class HashTable {
private:
    using Node = std::pair<const K, V>;
    using Bucket = std::list<Node>;

    std::vector<Bucket> table;
    size_t current_size;
    const float max_load_factor = 0.75f;

    size_t hash(const K& key) const {
        return std::hash<K>{}(key) % table.size();
    }

    void rehash() {
        size_t new_capacity = table.empty() ? 16 : table.size() * 2;
        std::vector<Bucket> new_table(new_capacity);

        for (const auto& bucket : table) {
            for (const auto& node : bucket) {
                size_t index = std::hash<K>{}(node.first) % new_table.size();
                new_table[index].push_front(node);
            }
        }
        table = std::move(new_table);
    }

public:
    HashTable(size_t initial_capacity = 16) : current_size(0) {
        if (initial_capacity == 0) initial_capacity = 16;
        table.resize(initial_capacity);
    }

    void insert(const K& key, const V& value) {
        if (table.empty() || (static_cast<float>(current_size) / table.size()) >= max_load_factor) {
            rehash();
        }

        size_t index = hash(key);
        for (auto& node : table[index]) {
            if (node.first == key) {
                node.second = value; // Update existing key
                return;
            }
        }

        table[index].emplace_front(key, value);
        current_size++;
    }

    std::optional<V> search(const K& key) const {
        if (table.empty()) {
            return std::nullopt;
        }
        size_t index = hash(key);
        for (const auto& node : table[index]) {
            if (node.first == key) {
                return node.second;
            }
        }
        return std::nullopt;
    }

    void remove(const K& key) {
        if (table.empty()) {
            return;
        }
        size_t index = hash(key);
        auto& bucket = table[index];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                bucket.erase(it);
                current_size--;
                return;
            }
        }
    }
    
    size_t get_capacity() const {
        return table.size();
    }

    size_t get_size() const {
        return current_size;
    }
};

int main() {
    HashTable<std::string, int> ht;

    // Test Case 1: Basic insertion and search
    std::cout << "--- Test Case 1: Insert and Search ---" << std::endl;
    ht.insert("apple", 10);
    ht.insert("banana", 20);
    if (auto val = ht.search("apple")) std::cout << "Value for 'apple': " << *val << std::endl;
    if (auto val = ht.search("banana")) std::cout << "Value for 'banana': " << *val << std::endl;
    std::cout << std::endl;

    // Test Case 2: Search for a non-existent key
    std::cout << "--- Test Case 2: Search Non-existent Key ---" << std::endl;
    if (auto val = ht.search("cherry")) std::cout << "Value for 'cherry': " << *val << std::endl;
    else std::cout << "Value for 'cherry': Not found" << std::endl;
    std::cout << std::endl;

    // Test Case 3: Update an existing key
    std::cout << "--- Test Case 3: Update Key ---" << std::endl;
    if (auto val = ht.search("apple")) std::cout << "Value for 'apple' before update: " << *val << std::endl;
    ht.insert("apple", 15);
    if (auto val = ht.search("apple")) std::cout << "Value for 'apple' after update: " << *val << std::endl;
    std::cout << std::endl;

    // Test Case 4: Deletion
    std::cout << "--- Test Case 4: Deletion ---" << std::endl;
    if (auto val = ht.search("banana")) std::cout << "Value for 'banana' before deletion: " << *val << std::endl;
    ht.remove("banana");
    if (auto val = ht.search("banana")) std::cout << "Value for 'banana' after deletion: " << *val << std::endl;
    else std::cout << "Value for 'banana' after deletion: Not found" << std::endl;
    std::cout << std::endl;

    // Test Case 5: Trigger rehash
    std::cout << "--- Test Case 5: Trigger Rehash ---" << std::endl;
    std::cout << "Initial capacity: " << ht.get_capacity() << std::endl;
    // Initial capacity is 16, threshold is 0.75 * 16 = 12.
    // We have 1 element ('apple'). Let's add 11 more to trigger rehash.
    for (int i = 0; i < 12; ++i) {
        ht.insert("key" + std::to_string(i), i);
    }
    std::cout << "Size: " << ht.get_size() << std::endl;
    std::cout << "Capacity after rehash: " << ht.get_capacity() << std::endl; // Expected: 32
    if (auto val = ht.search("key5")) std::cout << "Value for 'key5': " << *val << std::endl;
    if (auto val = ht.search("apple")) std::cout << "Value for 'apple': " << *val << std::endl;

    return 0;
}