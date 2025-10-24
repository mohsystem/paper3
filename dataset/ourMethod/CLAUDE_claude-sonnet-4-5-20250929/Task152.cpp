
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <limits>

// Hash table implementation using separate chaining for collision resolution
// Security: All inputs validated, bounds checked, memory managed via RAII

class HashTable {
private:
    struct Node {
        std::string key;
        int value;
        std::unique_ptr<Node> next;
        
        Node(const std::string& k, int v) : key(k), value(v), next(nullptr) {}
    };
    
    std::vector<std::unique_ptr<Node>> table;
    size_t capacity;
    size_t count;
    static constexpr size_t MAX_CAPACITY = 1000000; // Prevent excessive memory
    static constexpr size_t MIN_CAPACITY = 16;
    
    // Simple hash function with bounds checking
    size_t hash(const std::string& key) const {
        if (key.empty() || capacity == 0) {
            return 0;
        }
        
        size_t hash_value = 0;
        // Validate key length to prevent overflow in hash computation
        if (key.length() > 10000) {
            return 0; // Reject excessively long keys
        }
        
        for (unsigned char c : key) {
            // Use modulo to prevent overflow
            hash_value = (hash_value * 31 + c) % capacity;
        }
        return hash_value;
    }
    
    // Input validation for keys
    bool isValidKey(const std::string& key) const {
        // Reject empty keys or excessively long keys
        return !key.empty() && key.length() <= 10000;
    }
    
public:
    explicit HashTable(size_t cap = 16) : capacity(cap), count(0) {
        // Validate capacity within safe bounds
        if (cap < MIN_CAPACITY) {
            capacity = MIN_CAPACITY;
        }
        if (cap > MAX_CAPACITY) {
            capacity = MAX_CAPACITY;
        }
        
        table.resize(capacity);
        // RAII: vector automatically manages memory
    }
    
    // Insert or update key-value pair
    bool insert(const std::string& key, int value) {
        // Validate input key
        if (!isValidKey(key)) {
            return false;
        }
        
        size_t index = hash(key);
        // Bounds check (defensive, vector handles this but explicit check is safer)
        if (index >= capacity) {
            return false;
        }
        
        Node* current = table[index].get();
        
        // Check if key already exists and update
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return true;
            }
            current = current->next.get();
        }
        
        // Insert new node at head of chain
        auto newNode = std::make_unique<Node>(key, value);
        if (!newNode) {
            return false; // Memory allocation failed
        }
        
        newNode->next = std::move(table[index]);
        table[index] = std::move(newNode);
        count++;
        
        return true;
    }
    
    // Search for key and return value via output parameter
    bool search(const std::string& key, int& value) const {
        // Validate input key
        if (!isValidKey(key)) {
            return false;
        }
        
        size_t index = hash(key);
        // Bounds check
        if (index >= capacity) {
            return false;
        }
        
        Node* current = table[index].get();
        
        while (current != nullptr) {
            if (current->key == key) {
                value = current->value;
                return true;
            }
            current = current->next.get();
        }
        
        return false; // Key not found
    }
    
    // Delete key from hash table
    bool remove(const std::string& key) {
        // Validate input key
        if (!isValidKey(key)) {
            return false;
        }
        
        size_t index = hash(key);
        // Bounds check
        if (index >= capacity) {
            return false;
        }
        
        Node* current = table[index].get();
        Node* prev = nullptr;
        
        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    // Remove head of chain
                    table[index] = std::move(current->next);
                } else {
                    // Remove middle/end of chain
                    prev->next = std::move(current->next);
                }
                count--;
                return true;
            }
            prev = current;
            current = current->next.get();
        }
        
        return false; // Key not found
    }
    
    size_t size() const {
        return count;
    }
};

int main() {
    HashTable ht(16);
    
    // Test case 1: Basic insert and search
    std::cout << "Test 1: Insert and search\\n";
    ht.insert("apple", 100);
    int value = 0;
    if (ht.search("apple", value)) {
        std::cout << "Found: apple = " << value << "\\n";
    } else {
        std::cout << "Not found: apple\\n";
    }
    
    // Test case 2: Multiple inserts
    std::cout << "\\nTest 2: Multiple inserts\\n";
    ht.insert("banana", 200);
    ht.insert("cherry", 300);
    ht.insert("date", 400);
    if (ht.search("cherry", value)) {
        std::cout << "Found: cherry = " << value << "\\n";
    }
    
    // Test case 3: Update existing key
    std::cout << "\\nTest 3: Update existing key\\n";
    ht.insert("apple", 150);
    if (ht.search("apple", value)) {
        std::cout << "Updated: apple = " << value << "\\n";
    }
    
    // Test case 4: Delete operation
    std::cout << "\\nTest 4: Delete operation\\n";
    ht.remove("banana");
    if (!ht.search("banana", value)) {
        std::cout << "Successfully deleted: banana\\n";
    }
    
    // Test case 5: Search non-existent key and invalid input
    std::cout << "\\nTest 5: Edge cases\\n";
    if (!ht.search("grape", value)) {
        std::cout << "Not found: grape (expected)\\n";
    }
    if (!ht.search("", value)) {
        std::cout << "Empty key rejected (expected)\\n";
    }
    
    std::cout << "\\nTotal items in hash table: " << ht.size() << "\\n";
    
    return 0;
}
