#include <iostream>
#include <vector>
#include <stdexcept>
#include <new>

struct Node {
    int key;
    int value;
    Node* next;

    Node(int k, int v) : key(k), value(v), next(nullptr) {}
};

class HashTable {
private:
    std::vector<Node*> table;
    int capacity;

    int hash(int key) {
        // Ensure the hash is non-negative
        return std::abs(key) % capacity;
    }

public:
    HashTable(int cap) {
        if (cap <= 0) {
            throw std::invalid_argument("Capacity must be positive.");
        }
        this->capacity = cap;
        table.resize(capacity, nullptr);
    }

    ~HashTable() {
        for (int i = 0; i < capacity; ++i) {
            Node* current = table[i];
            while (current != nullptr) {
                Node* to_delete = current;
                current = current->next;
                delete to_delete;
            }
        }
    }
    
    // Disable copy constructor and assignment operator
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void insert(int key, int value) {
        int index = hash(key);
        Node* head = table[index];

        // Search for key. If it exists, update value.
        Node* current = head;
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }

        // If key not found, insert a new node at the beginning of the chain.
        try {
            Node* newNode = new Node(key, value);
            newNode->next = head;
            table[index] = newNode;
        } catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        }
    }

    bool search(int key, int& value) {
        int index = hash(key);
        Node* current = table[index];

        while (current != nullptr) {
            if (current->key == key) {
                value = current->value;
                return true;
            }
            current = current->next;
        }
        
        return false; // Key not found
    }

    void remove(int key) {
        int index = hash(key);
        Node* current = table[index];
        Node* prev = nullptr;

        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    // Node to be deleted is the head
                    table[index] = current->next;
                } else {
                    // Node is in the middle or at the end
                    prev->next = current->next;
                }
                delete current;
                return; // Key found and deleted
            }
            prev = current;
            current = current->next;
        }
    }

    void printTable() {
        std::cout << "---- Hash Table ----" << std::endl;
        for (int i = 0; i < capacity; ++i) {
            std::cout << "Bucket " << i << ": ";
            Node* current = table[i];
            if (current == nullptr) {
                std::cout << "[]" << std::endl;
                continue;
            }
            while (current != nullptr) {
                std::cout << "[K:" << current->key << ", V:" << current->value << "] -> ";
                current = current->next;
            }
            std::cout << "nullptr" << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }
};

int main() {
    try {
        HashTable ht(10);

        // Test Case 1: Insert new key-value pairs
        std::cout << "Test Case 1: Inserting new elements" << std::endl;
        ht.insert(1, 10);
        ht.insert(11, 110); // Should collide with key 1 if capacity is 10
        ht.insert(2, 20);
        ht.insert(12, 120);
        ht.printTable();

        // Test Case 2: Search for an existing key
        std::cout << "\nTest Case 2: Searching for key 11" << std::endl;
        int value;
        if (ht.search(11, value)) {
            std::cout << "Value for key 11: " << value << std::endl;
        } else {
            std::cout << "Value for key 11: Not Found" << std::endl;
        }

        // Test Case 3: Search for a non-existing key
        std::cout << "\nTest Case 3: Searching for key 99" << std::endl;
        if (ht.search(99, value)) {
            std::cout << "Value for key 99: " << value << std::endl;
        } else {
            std::cout << "Value for key 99: Not Found" << std::endl;
        }

        // Test Case 4: Update an existing key
        std::cout << "\nTest Case 4: Updating key 1 to value 100" << std::endl;
        if (ht.search(1, value)) {
            std::cout << "Value for key 1 before update: " << value << std::endl;
        }
        ht.insert(1, 100);
        if (ht.search(1, value)) {
            std::cout << "Value for key 1 after update: " << value << std::endl;
        }
        ht.printTable();

        // Test Case 5: Delete a key and verify
        std::cout << "\nTest Case 5: Deleting key 11" << std::endl;
        ht.remove(11);
        std::cout << "Searching for key 11 after deletion..." << std::endl;
        if (ht.search(11, value)) {
            std::cout << "Value for key 11: " << value << std::endl;
        } else {
            std::cout << "Value for key 11: Not Found" << std::endl;
        }
        ht.printTable();

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}