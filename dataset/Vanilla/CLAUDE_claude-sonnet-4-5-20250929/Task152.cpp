
#include <iostream>
#include <string>
#include <list>
#include <vector>

using namespace std;

class HashNode {
public:
    string key;
    int value;
    
    HashNode(string k, int v) : key(k), value(v) {}
};

class Task152 {
private:
    vector<list<HashNode>> table;
    int capacity;
    int size;
    
    int hash(const string& key) {
        int hashValue = 0;
        for (char c : key) {
            hashValue = (hashValue * 31 + c) % capacity;
        }
        return abs(hashValue);
    }
    
public:
    Task152(int cap = 10) : capacity(cap), size(0) {
        table.resize(capacity);
    }
    
    void insert(const string& key, int value) {
        int index = hash(key);
        list<HashNode>& bucket = table[index];
        
        for (auto& node : bucket) {
            if (node.key == key) {
                node.value = value;
                return;
            }
        }
        
        bucket.push_back(HashNode(key, value));
        size++;
    }
    
    int* search(const string& key) {
        int index = hash(key);
        list<HashNode>& bucket = table[index];
        
        for (auto& node : bucket) {
            if (node.key == key) {
                return new int(node.value);
            }
        }
        
        return nullptr;
    }
    
    bool deleteKey(const string& key) {
        int index = hash(key);
        list<HashNode>& bucket = table[index];
        
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->key == key) {
                bucket.erase(it);
                size--;
                return true;
            }
        }
        
        return false;
    }
    
    int getSize() {
        return size;
    }
};

int main() {
    cout << "Test Case 1: Basic insert and search" << endl;
    Task152 ht1(10);
    ht1.insert("apple", 100);
    ht1.insert("banana", 200);
    ht1.insert("orange", 300);
    int* result1 = ht1.search("apple");
    cout << "Search 'apple': " << (result1 ? *result1 : -1) << endl;
    delete result1;
    int* result2 = ht1.search("banana");
    cout << "Search 'banana': " << (result2 ? *result2 : -1) << endl;
    delete result2;
    int* result3 = ht1.search("orange");
    cout << "Search 'orange': " << (result3 ? *result3 : -1) << endl;
    delete result3;
    
    cout << "\\nTest Case 2: Delete operation" << endl;
    Task152 ht2(10);
    ht2.insert("cat", 50);
    ht2.insert("dog", 60);
    int* result4 = ht2.search("cat");
    cout << "Before delete - Search 'cat': " << (result4 ? *result4 : -1) << endl;
    delete result4;
    ht2.deleteKey("cat");
    int* result5 = ht2.search("cat");
    cout << "After delete - Search 'cat': " << (result5 ? *result5 : -1) << endl;
    delete result5;
    int* result6 = ht2.search("dog");
    cout << "Search 'dog': " << (result6 ? *result6 : -1) << endl;
    delete result6;
    
    cout << "\\nTest Case 3: Update existing key" << endl;
    Task152 ht3(10);
    ht3.insert("key1", 10);
    int* result7 = ht3.search("key1");
    cout << "Initial value: " << (result7 ? *result7 : -1) << endl;
    delete result7;
    ht3.insert("key1", 20);
    int* result8 = ht3.search("key1");
    cout << "Updated value: " << (result8 ? *result8 : -1) << endl;
    delete result8;
    
    cout << "\\nTest Case 4: Search non-existent key" << endl;
    Task152 ht4(10);
    ht4.insert("exist", 100);
    int* result9 = ht4.search("exist");
    cout << "Search 'exist': " << (result9 ? *result9 : -1) << endl;
    delete result9;
    int* result10 = ht4.search("notexist");
    cout << "Search 'notexist': " << (result10 ? *result10 : -1) << endl;
    delete result10;
    
    cout << "\\nTest Case 5: Multiple operations" << endl;
    Task152 ht5(10);
    ht5.insert("a", 1);
    ht5.insert("b", 2);
    ht5.insert("c", 3);
    cout << "Size: " << ht5.getSize() << endl;
    ht5.deleteKey("b");
    cout << "After delete, Size: " << ht5.getSize() << endl;
    int* result11 = ht5.search("a");
    cout << "Search 'a': " << (result11 ? *result11 : -1) << endl;
    delete result11;
    int* result12 = ht5.search("b");
    cout << "Search 'b': " << (result12 ? *result12 : -1) << endl;
    delete result12;
    int* result13 = ht5.search("c");
    cout << "Search 'c': " << (result13 ? *result13 : -1) << endl;
    delete result13;
    
    return 0;
}
