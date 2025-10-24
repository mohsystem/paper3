
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <functional>
#include <memory>

template<typename K, typename V>
class HashNode {
public:
    K key;
    V value;
    
    HashNode(K key, V value) : key(key), value(value) {}
};

template<typename K, typename V>
class HashTable {
private:
    std::vector<std::list<HashNode<K, V>>> buckets;
    int capacity;
    int size;
    
    int getBucketIndex(const K& key) {
        std::hash<K> hashFunc;
        size_t hashCode = hashFunc(key);
        return hashCode % capacity;
    }
    
public:
    HashTable(int capacity = 10) : capacity(capacity), size(0) {
        buckets.resize(capacity);
    }
    
    void insert(const K& key, const V& value) {
        int bucketIndex = getBucketIndex(key);
        auto& bucket = buckets[bucketIndex];
        
        for (auto& node : bucket) {
            if (node.key == key) {
                node.value = value;
                return;
            }
        }
        
        bucket.push_back(HashNode<K, V>(key, value));
        size++;
    }
    
    V* search(const K& key) {
        int bucketIndex = getBucketIndex(key);
        auto& bucket = buckets[bucketIndex];
        
        for (auto& node : bucket) {
            if (node.key == key) {
                return &node.value;
            }
        }
        
        return nullptr;
    }
    
    bool deleteKey(const K& key) {
        int bucketIndex = getBucketIndex(key);
        auto& bucket = buckets[bucketIndex];
        
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->key == key) {
                bucket.erase(it);
                size--;
                return true;
            }
        }
        
        return false;
    }
    
    int getSize() const {
        return size;
    }
    
    bool isEmpty() const {
        return size == 0;
    }
};

int main() {
    std::cout << "Test Case 1: Basic Insert and Search" << std::endl;
    HashTable<std::string, int> ht1(10);
    ht1.insert("apple", 5);
    ht1.insert("banana", 10);
    ht1.insert("orange", 15);
    int* result1 = ht1.search("apple");
    std::cout << "Search 'apple': " << (result1 ? *result1 : -1) << std::endl;
    int* result2 = ht1.search("banana");
    std::cout << "Search 'banana': " << (result2 ? *result2 : -1) << std::endl;
    std::cout << "Size: " << ht1.getSize() << std::endl;
    
    std::cout << "\\nTest Case 2: Delete Operation" << std::endl;
    HashTable<std::string, int> ht2(10);
    ht2.insert("key1", 100);
    ht2.insert("key2", 200);
    std::cout << "Before delete - Size: " << ht2.getSize() << std::endl;
    std::cout << "Delete 'key1': " << (ht2.deleteKey("key1") ? "true" : "false") << std::endl;
    std::cout << "After delete - Size: " << ht2.getSize() << std::endl;
    int* result3 = ht2.search("key1");
    std::cout << "Search 'key1': " << (result3 ? "found" : "not found") << std::endl;
    
    std::cout << "\\nTest Case 3: Update Existing Key" << std::endl;
    HashTable<std::string, int> ht3(10);
    ht3.insert("counter", 1);
    int* result4 = ht3.search("counter");
    std::cout << "Initial value: " << (result4 ? *result4 : -1) << std::endl;
    ht3.insert("counter", 2);
    int* result5 = ht3.search("counter");
    std::cout << "Updated value: " << (result5 ? *result5 : -1) << std::endl;
    std::cout << "Size: " << ht3.getSize() << std::endl;
    
    std::cout << "\\nTest Case 4: Search Non-existent Key" << std::endl;
    HashTable<std::string, int> ht4(10);
    ht4.insert("exists", 99);
    int* result6 = ht4.search("exists");
    std::cout << "Search 'exists': " << (result6 ? *result6 : -1) << std::endl;
    int* result7 = ht4.search("notexists");
    std::cout << "Search 'notexists': " << (result7 ? "found" : "not found") << std::endl;
    
    std::cout << "\\nTest Case 5: Multiple Operations" << std::endl;
    HashTable<int, std::string> ht5(10);
    ht5.insert(1, "one");
    ht5.insert(2, "two");
    ht5.insert(3, "three");
    std::string* result8 = ht5.search(2);
    std::cout << "Search 2: " << (result8 ? *result8 : "not found") << std::endl;
    ht5.deleteKey(2);
    std::string* result9 = ht5.search(2);
    std::cout << "After delete 2: " << (result9 ? *result9 : "not found") << std::endl;
    ht5.insert(4, "four");
    std::string* result10 = ht5.search(4);
    std::cout << "Search 4: " << (result10 ? *result10 : "not found") << std::endl;
    std::cout << "Final size: " << ht5.getSize() << std::endl;
    
    return 0;
}
