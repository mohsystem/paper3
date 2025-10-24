#include <iostream>

struct Node {
    int data;
    Node* next;

    Node(int val) : data(val), next(nullptr) {}
};

/**
 * @brief Inserts a new node at the beginning of the list.
 * @param head The current head of the list.
 * @param data The data for the new node.
 * @return The new head of the list.
 */
Node* insert(Node* head, int data) {
    // Security: 'new' throws std::bad_alloc on failure, which is the standard C++ way.
    Node* newNode = new Node(data);
    newNode->next = head;
    return newNode;
}

/**
 * @brief Deletes the first occurrence of a node with the given key.
 * @param head The current head of the list.
 * @param key The data of the node to be deleted.
 * @return The new head of the list.
 */
Node* deleteNode(Node* head, int key) {
    // Security: Handle case where list is empty.
    if (head == nullptr) {
        return nullptr;
    }
    
    // Case 1: The head node itself holds the key.
    if (head->data == key) {
        Node* temp = head;
        head = head->next;
        delete temp; // Security: Free memory to prevent leaks.
        return head;
    }
    
    Node* current = head;
    // Case 2: The key is somewhere other than the head.
    // Traverse to find the node *before* the one to be deleted.
    // Security: Check current->next to prevent dereferencing a nullptr.
    while (current->next != nullptr && current->next->data != key) {
        current = current->next;
    }
    
    // If the key was found (current->next is the node to delete).
    if (current->next != nullptr) {
        Node* temp = current->next;
        current->next = current->next->next; // Unlink the node.
        delete temp; // Security: Free memory.
    }
    
    return head;
}

/**
 * @brief Searches for a node with the given key.
 * @param head The head of the list.
 * @param key The data to search for.
 * @return true if the key is found, false otherwise.
 */
bool search(Node* head, int key) {
    Node* current = head;
    // Security: The loop condition prevents dereferencing a null pointer.
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
 * @param head The head of the list.
 */
void printList(Node* head) {
    Node* current = head;
    while (current != nullptr) {
        std::cout << current->data << " -> ";
        current = current->next;
    }
    std::cout << "nullptr" << std::endl;
}

/**
 * @brief Security: Frees all nodes in the list to prevent memory leaks.
 * @param head The head of the list to be freed.
 */
void freeList(Node* head) {
    Node* current = head;
    while (current != nullptr) {
        Node* next = current->next;
        delete current;
        current = next;
    }
}

int main() {
    Node* head = nullptr;

    // Test Case 1: Insertion
    std::cout << "Test Case 1: Insertion" << std::endl;
    head = insert(head, 30);
    head = insert(head, 20);
    head = insert(head, 10);
    std::cout << "List after insertions: ";
    printList(head); // Expected: 10 -> 20 -> 30 -> nullptr
    std::cout << "--------------------" << std::endl;

    // Test Case 2: Search for an existing element
    std::cout << "Test Case 2: Search for existing element (20)" << std::endl;
    std::cout << "Found: " << (search(head, 20) ? "true" : "false") << std::endl; // Expected: true
    std::cout << "--------------------" << std::endl;
    
    // Test Case 3: Search for a non-existent element
    std::cout << "Test Case 3: Search for non-existent element (50)" << std::endl;
    std::cout << "Found: " << (search(head, 50) ? "true" : "false") << std::endl; // Expected: false
    std::cout << "--------------------" << std::endl;
    
    // Test Case 4: Delete an element from the middle
    std::cout << "Test Case 4: Delete middle element (20)" << std::endl;
    head = deleteNode(head, 20);
    std::cout << "List after deleting 20: ";
    printList(head); // Expected: 10 -> 30 -> nullptr
    std::cout << "--------------------" << std::endl;
    
    // Test Case 5: Delete head, non-existent, and from empty list
    std::cout << "Test Case 5: Complex Deletions" << std::endl;
    head = deleteNode(head, 10); // Delete head
    std::cout << "List after deleting head (10): ";
    printList(head); // Expected: 30 -> nullptr
    head = deleteNode(head, 100); // Delete non-existent
    std::cout << "List after attempting to delete 100: ";
    printList(head); // Expected: 30 -> nullptr
    head = deleteNode(head, 30); // Delete last element
    std::cout << "List after deleting 30: ";
    printList(head); // Expected: nullptr
    head = deleteNode(head, 5); // Delete from empty list
    std::cout << "List after deleting from empty list: ";
    printList(head); // Expected: nullptr
    std::cout << "--------------------" << std::endl;
    
    // Security: Clean up any remaining memory before exiting.
    freeList(head);

    return 0;
}