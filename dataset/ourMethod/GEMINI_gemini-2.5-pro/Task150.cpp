#include <iostream>

class SinglyLinkedList {
private:
    struct Node {
        int data;
        Node* next;
        Node(int d) : data(d), next(nullptr) {}
    };

    Node* head;

public:
    SinglyLinkedList() : head(nullptr) {}

    ~SinglyLinkedList() {
        Node* current = head;
        Node* nextNode = nullptr;
        while (current != nullptr) {
            nextNode = current->next;
            delete current;
            current = nextNode;
        }
        head = nullptr;
    }

    // Insert a new node at the front of the list
    void insert(int newData) {
        Node* newNode = new Node(newData);
        newNode->next = head;
        head = newNode;
    }

    // Delete the first occurrence of key in linked list
    void deleteNode(int key) {
        Node* temp = head;
        Node* prev = nullptr;

        // If head node itself holds the key to be deleted
        if (temp != nullptr && temp->data == key) {
            head = temp->next;
            delete temp;
            return;
        }

        // Search for the key to be deleted, keep track of the previous node
        while (temp != nullptr && temp->data != key) {
            prev = temp;
            temp = temp->next;
        }

        // If key was not present in linked list
        if (temp == nullptr) {
            return;
        }

        // Unlink the node from linked list and free memory
        if (prev != nullptr) {
            prev->next = temp->next;
        }
        delete temp;
    }

    // Search for a key in the linked list
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

    // Utility function to print the linked list
    void printList() {
        Node* tnode = head;
        while (tnode != nullptr) {
            std::cout << tnode->data << " -> ";
            tnode = tnode->next;
        }
        std::cout << "NULL" << std::endl;
    }
};

int main() {
    SinglyLinkedList list;

    // Test Case 1: Insertion
    std::cout << "--- Test Case 1: Insertion ---" << std::endl;
    list.insert(3);
    list.insert(2);
    list.insert(1);
    std::cout << "Created list: ";
    list.printList();
    std::cout << std::endl;

    // Test Case 2: Search for an existing element
    std::cout << "--- Test Case 2: Search (existing) ---" << std::endl;
    int keyToSearch1 = 2;
    std::cout << "Searching for " << keyToSearch1 << ": " << (list.search(keyToSearch1) ? "Found" : "Not Found") << std::endl;
    std::cout << std::endl;

    // Test Case 3: Search for a non-existing element
    std::cout << "--- Test Case 3: Search (non-existing) ---" << std::endl;
    int keyToSearch2 = 4;
    std::cout << "Searching for " << keyToSearch2 << ": " << (list.search(keyToSearch2) ? "Found" : "Not Found") << std::endl;
    std::cout << std::endl;

    // Test Case 4: Delete an element from the middle
    std::cout << "--- Test Case 4: Deletion (middle) ---" << std::endl;
    int keyToDelete1 = 2;
    std::cout << "Deleting " << keyToDelete1 << std::endl;
    list.deleteNode(keyToDelete1);
    std::cout << "List after deletion: ";
    list.printList();
    std::cout << std::endl;
    
    // Test Case 5: Delete the head element
    std::cout << "--- Test Case 5: Deletion (head) ---" << std::endl;
    int keyToDelete2 = 1;
    std::cout << "Deleting " << keyToDelete2 << std::endl;
    list.deleteNode(keyToDelete2);
    std::cout << "List after deletion: ";
    list.printList();

    return 0;
}