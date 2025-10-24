
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <cstdlib>

using namespace std;

class Task152 {
private:
    static const int TABLE_SIZE = 10;
    
    struct Entry {
        string key;
        int value;
        
        Entry(const string& k, int v) : key(k), value(v) {}
    };
    
    vector<list<Entry>> table;
    
    int hash(const string& key) {
        if (key.empty()) return 0;
        
        long long hash_value = 0;
        for (size_t i = 0; i < key.length(); i++) {
            hash_value = (hash_value * 31 + static_cast<unsigned char>(key[i])) % TABLE_SIZE;
        }
        return abs(static_cast<int>(hash_value));
    }
    
public:
    Task152() : table(TABLE_SIZE) {}
    
    void insert(const string& key, int value) {
        if (key.empty()) {
            cout << "Error: Key cannot be empty" << endl;
            return;
        }
        
        int index = hash(key);
        list<Entry>& bucket = table[index];
        
        // Update if key exists
        for (auto& entry : bucket) {
            if (entry.key == key) {
                entry.value = value;
                return;
            }
        }
        
        // Insert new entry
        bucket.push_back(Entry(key, value));
    }
    
    int* search(const string& key) {
        if (key.empty()) {
            cout << "Error: Key cannot be empty" << endl;
            return nullptr;
        }
        
        int index = hash(key);
        list<Entry>& bucket = table[index];
        
        for (auto& entry : bucket) {
            if (entry.key == key) {
                return &entry.value;
            }
        }
        
        return nullptr;
    }
    
    bool deleteKey(const string& key) {
        if (key.empty()) {
            cout << "Error: Key cannot be empty" << endl;
            return false;
        }
        
        int index = hash(key);
        list<Entry>& bucket = table[index];
        
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->key == key) {
                bucket.erase(it);
                return true;
            }
        }
        
        return false;
    }
};

int main() {
    Task152 hashTable;
    
    // Test Case 1: Insert and search
    cout << "Test 1: Insert and Search" << endl;
    hashTable.insert("apple", 100);
    hashTable.insert("banana", 200);
    int* result = hashTable.search("apple");
    cout << "Search 'apple': " << (result ? to_string(*result) : "Not found") << endl;
    result = hashTable.search("banana");
    cout << "Search 'banana': " << (result ? to_string(*result) : "Not found") << endl;
    
    // Test Case 2: Update existing key
    cout << "\\nTest 2: Update Existing Key" << endl;
    hashTable.insert("apple", 150);
    result = hashTable.search("apple");
    cout << "Search 'apple' after update: " << (result ? to_string(*result) : "Not found") << endl;
    
    // Test Case 3: Delete operation
    cout << "\\nTest 3: Delete Operation" << endl;
    cout << "Delete 'banana': " << (hashTable.deleteKey("banana") ? "true" : "false") << endl;
    result = hashTable.search("banana");
    cout << "Search 'banana' after delete: " << (result ? to_string(*result) : "Not found") << endl;
    
    // Test Case 4: Search non-existent key
    cout << "\\nTest 4: Search Non-existent Key" << endl;
    result = hashTable.search("orange");
    cout << "Search 'orange': " << (result ? to_string(*result) : "Not found") << endl;
    
    // Test Case 5: Empty key handling
    cout << "\\nTest 5: Empty Key Handling" << endl;
    hashTable.insert("", 300);
    result = hashTable.search("");
    cout << "Search empty: " << (result ? to_string(*result) : "Not found") << endl;
    cout << "Delete empty: " << (hashTable.deleteKey("") ? "true" : "false") << endl;
    
    return 0;
}
