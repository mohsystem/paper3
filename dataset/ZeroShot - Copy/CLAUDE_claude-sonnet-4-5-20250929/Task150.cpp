
#include <iostream>
using namespace std;

class Node {
public:
    int data;
    Node* next;
    
    Node(int data) {
        this->data = data;
        this->next = nullptr;
    }
};

class LinkedList {
private:
    Node* head;
    
public:
    LinkedList() {
        head = nullptr;
    }
    
    ~LinkedList() {
        Node* current = head;
        while (current != nullptr) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
    }
    
    void insert(int data) {
        Node* newNode = new Node(data);
        if (head == nullptr) {
            head = newNode;
            return;
        }
        
        Node* current = head;
        while (current->next != nullptr) {
            current = current->next;
        }
        current->next = newNode;
    }
    
    bool deleteNode(int data) {
        if (head == nullptr) {
            return false;
        }
        
        if (head->data == data) {
            Node* temp = head;
            head = head->next;
            delete temp;
            return true;
        }
        
        Node* current = head;
        while (current->next != nullptr) {
            if (current->next->data == data) {
                Node* temp = current->next;
                current->next = current->next->next;
                delete temp;
                return true;
            }
            current = current->next;
        }
        return false;
    }
    
    bool search(int data) {
        Node* current = head;
        while (current != nullptr) {
            if (current->data == data) {
                return true;
            }
            current = current->next;
        }
        return false;
    }
    
    void display() {
        Node* current = head;
        while (current != nullptr) {
            cout << current->data << " -> ";
            current = current->next;
        }
        cout << "null" << endl;
    }
};

int main() {
    // Test Case 1: Insert and display
    cout << "Test Case 1: Insert elements" << endl;
    LinkedList list1;
    list1.insert(10);
    list1.insert(20);
    list1.insert(30);
    list1.display();
    
    // Test Case 2: Search operations
    cout << "\\nTest Case 2: Search operations" << endl;
    LinkedList list2;
    list2.insert(5);
    list2.insert(15);
    list2.insert(25);
    cout << "Search 15: " << (list2.search(15) ? "true" : "false") << endl;
    cout << "Search 100: " << (list2.search(100) ? "true" : "false") << endl;
    
    // Test Case 3: Delete operations
    cout << "\\nTest Case 3: Delete operations" << endl;
    LinkedList list3;
    list3.insert(1);
    list3.insert(2);
    list3.insert(3);
    list3.insert(4);
    cout << "Before delete: ";
    list3.display();
    list3.deleteNode(3);
    cout << "After deleting 3: ";
    list3.display();
    
    // Test Case 4: Delete head element
    cout << "\\nTest Case 4: Delete head element" << endl;
    LinkedList list4;
    list4.insert(100);
    list4.insert(200);
    list4.insert(300);
    cout << "Before delete: ";
    list4.display();
    list4.deleteNode(100);
    cout << "After deleting head: ";
    list4.display();
    
    // Test Case 5: Operations on empty list
    cout << "\\nTest Case 5: Operations on empty list" << endl;
    LinkedList list5;
    cout << "Delete from empty list: " << (list5.deleteNode(10) ? "true" : "false") << endl;
    cout << "Search in empty list: " << (list5.search(10) ? "true" : "false") << endl;
    list5.insert(50);
    cout << "After inserting 50: ";
    list5.display();
    
    return 0;
}
