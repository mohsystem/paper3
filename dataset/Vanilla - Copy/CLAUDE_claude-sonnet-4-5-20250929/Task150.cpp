
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
    
    void insert(int data) {
        Node* newNode = new Node(data);
        if (head == nullptr) {
            head = newNode;
        } else {
            Node* current = head;
            while (current->next != nullptr) {
                current = current->next;
            }
            current->next = newNode;
        }
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
        cout << "[";
        while (current != nullptr) {
            cout << current->data;
            if (current->next != nullptr) {
                cout << ", ";
            }
            current = current->next;
        }
        cout << "]" << endl;
    }
    
    ~LinkedList() {
        Node* current = head;
        while (current != nullptr) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
    }
};

int main() {
    // Test Case 1: Basic insert and search
    cout << "Test Case 1: Basic insert and search" << endl;
    LinkedList list1;
    list1.insert(10);
    list1.insert(20);
    list1.insert(30);
    list1.display();
    cout << "Search 20: " << (list1.search(20) ? "true" : "false") << endl;
    cout << "Search 40: " << (list1.search(40) ? "true" : "false") << endl;
    cout << endl;
    
    // Test Case 2: Delete from middle
    cout << "Test Case 2: Delete from middle" << endl;
    LinkedList list2;
    list2.insert(5);
    list2.insert(15);
    list2.insert(25);
    list2.insert(35);
    cout << "Before delete: ";
    list2.display();
    list2.deleteNode(15);
    cout << "After delete 15: ";
    list2.display();
    cout << endl;
    
    // Test Case 3: Delete head
    cout << "Test Case 3: Delete head" << endl;
    LinkedList list3;
    list3.insert(100);
    list3.insert(200);
    list3.insert(300);
    cout << "Before delete: ";
    list3.display();
    list3.deleteNode(100);
    cout << "After delete head: ";
    list3.display();
    cout << endl;
    
    // Test Case 4: Delete non-existent element
    cout << "Test Case 4: Delete non-existent element" << endl;
    LinkedList list4;
    list4.insert(1);
    list4.insert(2);
    list4.insert(3);
    cout << "List: ";
    list4.display();
    cout << "Delete 99: " << (list4.deleteNode(99) ? "true" : "false") << endl;
    cout << endl;
    
    // Test Case 5: Operations on empty list
    cout << "Test Case 5: Operations on empty list" << endl;
    LinkedList list5;
    cout << "Search in empty list: " << (list5.search(10) ? "true" : "false") << endl;
    cout << "Delete from empty list: " << (list5.deleteNode(10) ? "true" : "false") << endl;
    list5.insert(50);
    cout << "After insert: ";
    list5.display();
    
    return 0;
}
