#include <iostream>

class LinkedList {
private:
    struct Node {
        int data;
        Node* next;

        Node(int d) : data(d), next(nullptr) {}
    };

    Node* head;

public:
    LinkedList() : head(nullptr) {}

    // Destructor to free all nodes and prevent memory leaks
    ~LinkedList() {
        Node* current = head;
        Node* nextNode = nullptr;
        while (current != nullptr) {
            nextNode = current->next;
            delete current;
            current = nextNode;
        }
        head = nullptr;
    }

    /**
     * @brief Inserts a new node with the given data at the end of the list.
     * @param data The data for the new node.
     */
    void insert(int data) {
        Node* newNode = new Node(data);
        if (head == nullptr) {
            head = newNode;
            return;
        }
        Node* last = head;
        while (last->next != nullptr) {
            last = last->next;
        }
        last->next = newNode;
    }

    /**
     * @brief Deletes the first occurrence of a node with the given key.
     * @param key The key of the node to be deleted.
     */
    void deleteNode(int key) {
        Node* temp = head;
        Node* prev = nullptr;

        if (temp != nullptr && temp->data == key) {
            head = temp->next;
            delete temp;
            return;
        }

        while (temp != nullptr && temp->data != key) {
            prev = temp;
            temp = temp->next;
        }

        if (temp == nullptr) {
            return;
        }
        
        // prev is guaranteed to not be null here due to the head check
        prev->next = temp->next;
        delete temp;
    }

    /**
     * @brief Searches for a node with the given key in the list.
     * @param key The key to search for.
     * @return true if the key is found, false otherwise.
     */
    bool search(int key) {
        Node* current = head;
        while (current != nullptr) {
            if (current->data == key) {
                return true;
            }
            current = current->next;
        }
        return false;
    }

    /**
     * @brief Utility function to print the linked list.
     */
    void printList() {
        Node* current = head;
        if (current == nullptr) {
            std::cout << "List is empty." << std::endl;
            return;
        }
        while (current != nullptr) {
            std::cout << current->data << " -> ";
            current = current->next;
        }
        std::cout << "NULL" << std::endl;
    }
};

// Main function with test cases
int main() {
    LinkedList list;

    // Test Case 1: Insertion
    std::cout << "--- Test Case 1: Insertion ---" << std::endl;
    list.insert(10);
    list.insert(20);
    list.insert(30);
    list.insert(40);
    std::cout << "List after inserting 10, 20, 30, 40: ";
    list.printList();
    std::cout << std::endl;

    // Test Case 2: Search for an existing element
    std::cout << "--- Test Case 2: Search (Existing) ---" << std::endl;
    std::cout << "Searching for 30: " << (list.search(30) ? "Found" : "Not Found") << std::endl;
    std::cout << std::endl;

    // Test Case 3: Search for a non-existing element
    std::cout << "--- Test Case 3: Search (Non-Existing) ---" << std::endl;
    std::cout << "Searching for 99: " << (list.search(99) ? "Found" : "Not Found") << std::endl;
    std::cout << std::endl;

    // Test Case 4: Delete an element from the middle
    std::cout << "--- Test Case 4: Deletion (Middle) ---" << std::endl;
    std::cout << "List before deleting 20: ";
    list.printList();
    list.deleteNode(20);
    std::cout << "List after deleting 20: ";
    list.printList();
    std::cout << std::endl;

    // Test Case 5: Delete the head element
    std::cout << "--- Test Case 5: Deletion (Head) ---" << std::endl;
    std::cout << "List before deleting 10: ";
    list.printList();
    list.deleteNode(10);
    std::cout << "List after deleting 10: ";
    list.printList();

    return 0; // Destructor will be called here, freeing memory.
}