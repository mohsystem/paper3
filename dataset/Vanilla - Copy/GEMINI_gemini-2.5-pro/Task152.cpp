#include <iostream>
#include <vector>
#include <string>
#include <cmath>

// Node for the linked list in each hash table slot
class HashNode {
public:
    int key;
    int value;
    HashNode* next;

    HashNode(int key, int value) {
        this->key = key;
        this->value = value;
        this->next = nullptr;
    }
};

class HashTable {
private:
    std::vector<HashNode*> table;
    int capacity;

    // Hash function
    int hashFunction(int key) {
        return std::abs(key) % capacity;
    }

public:
    HashTable(int cap) {
        capacity = cap;
        table.resize(capacity, nullptr);
    }

    ~HashTable() {
        // Destructor to free all allocated memory
        for (int i = 0; i < capacity; ++i) {
            HashNode* entry = table[i];
            while (entry != nullptr) {
                HashNode* prev = entry;
                entry = entry->next;
                delete prev;
            }
        }
    }

    // Insert a key-value pair
    void insert(int key, int value) {
        int index = hashFunction(key);
        HashNode* head = table[index];
        HashNode* current = head;

        // Check if key exists and update value
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }

        // Insert new node at the beginning of the chain
        HashNode* newNode = new HashNode(key, value);
        newNode->next = head;
        table[index] = newNode;
    }

    // Search for a key and return its value
    // Returns -1 if key not found (assuming -1 is not a valid value)
    int search(int key) {
        int index = hashFunction(key);
        HashNode* head = table[index];
        HashNode* current = head;

        while (current != nullptr) {
            if (current->key == key) {
                return current->value;
            }
            current = current->next;
        }

        return -1; // Not found
    }

    // Delete a key
    void remove(int key) {
        int index = hashFunction(key);
        HashNode* head = table[index];
        HashNode* prev = nullptr;
        HashNode* current = head;
        
        while (current != nullptr && current->key != key) {
            prev = current;
            current = current->next;
        }

        if (current == nullptr) return;

        if (prev != nullptr) {
            prev->next = current->next;
        } else {
            table[index] = current->next;
        }
        
        delete current; // Free memory
    }
    
    void display() {
        std::cout << "---- HASH TABLE ----" << std::endl;
        for (int i = 0; i < capacity; ++i) {
            std::cout << "Index " << i << ":";
            HashNode* current = table[i];
            while (current != nullptr) {
                std::cout << " -> (" << current->key << ", " << current->value << ")";
                current = current->next;
            }
            std::cout << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }
};

int main() {
    HashTable hashTable(10);

    std::cout << "----- Testing Insert and Search -----" << std::endl;
    hashTable.insert(1, 10);
    hashTable.insert(2, 20);
    hashTable.insert(12, 120); // Collision with key 2
    hashTable.display();

    std::cout << "\nSearch for key 2: " << hashTable.search(2) << std::endl;
    std::cout << "Search for key 12: " << hashTable.search(12) << std::endl;
    std::cout << "Search for key 5 (non-existent): " << hashTable.search(5) << std::endl;

    std::cout << "\n----- Testing Update -----" << std::endl;
    hashTable.insert(2, 22); // Update value for key 2
    std::cout << "Search for key 2 after update: " << hashTable.search(2) << std::endl;
    hashTable.display();

    std::cout << "\n----- Testing Delete -----" << std::endl;
    hashTable.remove(12);
    std::cout << "Search for key 12 after delete: " << hashTable.search(12) << std::endl;
    std::cout << "Search for key 2 after deleting 12: " << hashTable.search(2) << std::endl;
    hashTable.display();
    
    std::cout << "\n----- More Test Cases -----" << std::endl;
    hashTable.insert(22, 220); // Another collision at index 2
    hashTable.insert(32, 320); // Another collision at index 2
    hashTable.display();
    std::cout << "Deleting head of a chain (key 32)..." << std::endl;
    hashTable.remove(32);
    hashTable.display();
    std::cout << "Search for 32: " << hashTable.search(32) << std::endl;
    std::cout << "Search for 22: " << hashTable.search(22) << std::endl;

    return 0;
}