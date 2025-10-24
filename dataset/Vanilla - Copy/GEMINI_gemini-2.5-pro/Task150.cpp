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

        prev->next = temp->next;
        delete temp;
    }

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

    void printList() {
        Node* current = head;
        while (current != nullptr) {
            std::cout << current->data << " -> ";
            current = current->next;
        }
        std::cout << "NULL" << std::endl;
    }
};

int main() {
    LinkedList list;

    // Test Case 1: Insertion
    std::cout << "--- Test Case 1: Insertion ---" << std::endl;
    list.insert(10);
    list.insert(20);
    list.insert(30);
    list.insert(40);
    list.insert(50);
    std::cout << "Created Linked list: ";
    list.printList();
    std::cout << std::endl;

    // Test Case 2: Search for an existing element
    std::cout << "--- Test Case 2: Search (Existing) ---" << std::endl;
    std::cout << "Searching for 30..." << std::endl;
    if (list.search(30)) {
        std::cout << "Element 30 found in the list." << std::endl;
    } else {
        std::cout << "Element 30 not found in the list." << std::endl;
    }
    std::cout << std::endl;

    // Test Case 3: Search for a non-existing element
    std::cout << "--- Test Case 3: Search (Not Existing) ---" << std::endl;
    std::cout << "Searching for 100..." << std::endl;
    if (list.search(100)) {
        std::cout << "Element 100 found in the list." << std::endl;
    } else {
        std::cout << "Element 100 not found in the list." << std::endl;
    }
    std::cout << std::endl;

    // Test Case 4: Deletion of a middle element
    std::cout << "--- Test Case 4: Deletion (Middle) ---" << std::endl;
    std::cout << "Deleting element 30..." << std::endl;
    list.deleteNode(30);
    std::cout << "List after deleting 30: ";
    list.printList();
    std::cout << std::endl;

    // Test Case 5: Deletion of head and tail elements
    std::cout << "--- Test Case 5: Deletion (Head and Tail) ---" << std::endl;
    std::cout << "Deleting element 10 (head)..." << std::endl;
    list.deleteNode(10);
    std::cout << "List after deleting 10: ";
    list.printList();
    
    std::cout << "Deleting element 50 (tail)..." << std::endl;
    list.deleteNode(50);
    std::cout << "List after deleting 50: ";
    list.printList();

    return 0;
}