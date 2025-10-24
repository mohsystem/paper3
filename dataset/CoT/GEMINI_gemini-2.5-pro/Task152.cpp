#include <iostream>
#include <vector>
#include <cmath>

class HashTable {
private:
    struct Node {
        int key;
        int value;
        Node* next;

        Node(int k, int v) : key(k), value(v), next(nullptr) {}
    };

    std::vector<Node*> buckets;
    int capacity;

    int getBucketIndex(int key) {
        return std::abs(key) % capacity;
    }

public:
    HashTable(int cap) {
        if (cap <= 0) {
            // In a real application, you might throw an exception.
            // For simplicity, we'll set a default capacity.
            cap = 10;
        }
        this->capacity = cap;
        buckets.resize(capacity, nullptr);
    }

    ~HashTable() {
        for (int i = 0; i < capacity; ++i) {
            Node* current = buckets[i];
            while (current != nullptr) {
                Node* toDelete = current;
                current = current->next;
                delete toDelete;
            }
        }
    }

    void insert(int key, int value) {
        int index = getBucketIndex(key);
        Node* current = buckets[index];

        // Check if key exists and update value
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }

        // Key not found, insert new node at the head of the chain
        Node* newNode = new Node(key, value);
        newNode->next = buckets[index];
        buckets[index] = newNode;
    }

    int search(int key) {
        int index = getBucketIndex(key);
        Node* current = buckets[index];

        while (current != nullptr) {
            if (current->key == key) {
                return current->value;
            }
            current = current->next;
        }
        return -1; // Sentinel value for not found
    }

    void remove(int key) {
        int index = getBucketIndex(key);
        Node* current = buckets[index];
        Node* prev = nullptr;

        // Find the node with the given key
        while (current != nullptr && current->key != key) {
            prev = current;
            current = current->next;
        }

        // If key was not found
        if (current == nullptr) {
            return;
        }

        // Unlink the node
        if (prev == nullptr) {
            // The node to be deleted is the head
            buckets[index] = current->next;
        } else {
            prev->next = current->next;
        }
        delete current;
    }
};

// Main class to encapsulate the test logic
class Task152 {
public:
    static void runTests() {
        HashTable hashTable(10);
        std::cout << "C++ Hash Table Implementation Test" << std::endl;

        // Test Case 1: Insert key-value pairs. (5 and 15 will collide)
        hashTable.insert(5, 50);
        hashTable.insert(15, 150);
        hashTable.insert(2, 20);
        std::cout << "Test 1: Insert (5, 50), (15, 150), (2, 20)" << std::endl;
        std::cout << "Value for key 15: " << hashTable.search(15) << std::endl;
        std::cout << "Value for key 5: " << hashTable.search(5) << std::endl;
        std::cout << "Value for key 2: " << hashTable.search(2) << std::endl;
        std::cout << "--------------------" << std::endl;

        // Test Case 2: Search for an existing key.
        std::cout << "Test 2: Search for existing key 15" << std::endl;
        std::cout << "Value: " << hashTable.search(15) << std::endl;
        std::cout << "--------------------" << std::endl;

        // Test Case 3: Search for a non-existing key.
        std::cout << "Test 3: Search for non-existing key 25" << std::endl;
        std::cout << "Value: " << hashTable.search(25) << std::endl;
        std::cout << "--------------------" << std::endl;
        
        // Test Case 4: Update an existing key's value.
        std::cout << "Test 4: Update value for key 5 to 55" << std::endl;
        hashTable.insert(5, 55);
        std::cout << "New value for key 5: " << hashTable.search(5) << std::endl;
        std::cout << "--------------------" << std::endl;

        // Test Case 5: Delete a key and verify deletion.
        std::cout << "Test 5: Delete key 15" << std::endl;
        hashTable.remove(15);
        std::cout << "Searching for deleted key 15: " << hashTable.search(15) << std::endl;
        std::cout << "Searching for key 5 to ensure it's still there: " << hashTable.search(5) << std::endl;
        std::cout << "--------------------" << std::endl;
    }
};

int main() {
    Task152::runTests();
    return 0;
}