
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <limits>

// Node structure using smart pointers for automatic memory management
template<typename T>
struct Node {
    T data;
    std::unique_ptr<Node<T>> next;
    
    explicit Node(const T& value) : data(value), next(nullptr) {}
};

// Singly linked list class with security measures
template<typename T>
class SinglyLinkedList {
private:
    std::unique_ptr<Node<T>> head;
    size_t size;
    static constexpr size_t MAX_SIZE = 1000000; // Prevent excessive memory usage
    
public:
    SinglyLinkedList() : head(nullptr), size(0) {}
    
    // Insert at the beginning - O(1)
    // Security: Checks size limit to prevent memory exhaustion
    bool insert(const T& value) {
        try {
            // Prevent excessive memory usage
            if (size >= MAX_SIZE) {
                std::cerr << "Error: List size limit reached" << std::endl;
                return false;
            }
            
            auto newNode = std::make_unique<Node<T>>(value);
            if (!newNode) {
                std::cerr << "Error: Memory allocation failed" << std::endl;
                return false;
            }
            
            newNode->next = std::move(head);
            head = std::move(newNode);
            size++;
            return true;
        } catch (const std::bad_alloc& e) {
            std::cerr << "Error: Memory allocation failed - " << e.what() << std::endl;
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Error during insert: " << e.what() << std::endl;
            return false;
        }
    }
    
    // Search for a value - O(n)
    // Security: Bounds checking through size validation
    bool search(const T& value) const {
        if (size == 0) {
            return false;
        }
        
        try {
            Node<T>* current = head.get();
            size_t count = 0;
            
            // Iterate with bounds checking to prevent infinite loops
            while (current != nullptr && count < size) {
                if (current->data == value) {
                    return true;
                }
                current = current->next.get();
                count++;
            }
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Error during search: " << e.what() << std::endl;
            return false;
        }
    }
    
    // Delete first occurrence of value - O(n)
    // Security: Safe pointer manipulation with validation
    bool deleteValue(const T& value) {
        if (size == 0) {
            return false;
        }
        
        try {
            // Check if head contains the value
            if (head && head->data == value) {
                head = std::move(head->next);
                size--;
                return true;
            }
            
            Node<T>* current = head.get();
            size_t count = 0;
            
            // Traverse with bounds checking
            while (current != nullptr && current->next != nullptr && count < size) {
                if (current->next->data == value) {
                    // Safe deletion using unique_ptr
                    current->next = std::move(current->next->next);
                    size--;
                    return true;
                }
                current = current->next.get();
                count++;
            }
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Error during delete: " << e.what() << std::endl;
            return false;
        }
    }
    
    size_t getSize() const { return size; }
    
    void display() const {
        Node<T>* current = head.get();
        size_t count = 0;
        
        std::cout << "List: ";
        while (current != nullptr && count < size) {
            std::cout << current->data << " -> ";
            current = current->next.get();
            count++;
        }
        std::cout << "NULL" << std::endl;
    }
};

int main() {
    // Test case 1: Basic insert and search operations
    std::cout << "Test Case 1: Basic operations with integers" << std::endl;
    SinglyLinkedList<int> list1;
    list1.insert(10);
    list1.insert(20);
    list1.insert(30);
    list1.display();
    std::cout << "Search 20: " << (list1.search(20) ? "Found" : "Not Found") << std::endl;
    std::cout << "Search 40: " << (list1.search(40) ? "Found" : "Not Found") << std::endl;
    std::cout << std::endl;
    
    // Test case 2: Delete operations
    std::cout << "Test Case 2: Delete operations" << std::endl;
    SinglyLinkedList<int> list2;
    list2.insert(5);
    list2.insert(15);
    list2.insert(25);
    list2.insert(35);
    list2.display();
    std::cout << "Delete 15: " << (list2.deleteValue(15) ? "Success" : "Failed") << std::endl;
    list2.display();
    std::cout << "Delete 35: " << (list2.deleteValue(35) ? "Success" : "Failed") << std::endl;
    list2.display();
    std::cout << std::endl;
    
    // Test case 3: String data type
    std::cout << "Test Case 3: String operations" << std::endl;
    SinglyLinkedList<std::string> list3;
    list3.insert("apple");
    list3.insert("banana");
    list3.insert("cherry");
    list3.display();
    std::cout << "Search 'banana': " << (list3.search("banana") ? "Found" : "Not Found") << std::endl;
    list3.deleteValue("banana");
    list3.display();
    std::cout << std::endl;
    
    // Test case 4: Empty list operations
    std::cout << "Test Case 4: Empty list operations" << std::endl;
    SinglyLinkedList<int> list4;
    std::cout << "Search in empty list: " << (list4.search(10) ? "Found" : "Not Found") << std::endl;
    std::cout << "Delete from empty list: " << (list4.deleteValue(10) ? "Success" : "Failed") << std::endl;
    list4.insert(100);
    list4.display();
    std::cout << std::endl;
    
    // Test case 5: Multiple operations
    std::cout << "Test Case 5: Multiple sequential operations" << std::endl;
    SinglyLinkedList<int> list5;
    for (int i = 1; i <= 10; i++) {
        list5.insert(i * 10);
    }
    list5.display();
    std::cout << "Size: " << list5.getSize() << std::endl;
    list5.deleteValue(50);
    list5.deleteValue(10);
    list5.deleteValue(100);
    list5.display();
    std::cout << "Size after deletions: " << list5.getSize() << std::endl;
    
    return 0;
}
