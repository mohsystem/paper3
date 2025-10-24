public class Task150 {

    Node head;

    static class Node {
        int data;
        Node next;

        Node(int d) {
            data = d;
            next = null;
        }
    }

    // Method to insert a new node at the end
    public void insert(int data) {
        Node newNode = new Node(data);
        if (head == null) {
            head = newNode;
            return;
        }
        Node last = head;
        while (last.next != null) {
            last = last.next;
        }
        last.next = newNode;
    }

    // Method to delete a node with a given key
    public void delete(int key) {
        Node temp = head, prev = null;

        // If head node itself holds the key to be deleted
        if (temp != null && temp.data == key) {
            head = temp.next; // Changed head
            return;
        }

        // Search for the key to be deleted, keep track of the
        // previous node as we need to change prev.next
        while (temp != null && temp.data != key) {
            prev = temp;
            temp = temp.next;
        }

        // If key was not present in linked list
        if (temp == null) {
            return;
        }

        // Unlink the node from linked list
        prev.next = temp.next;
    }

    // Method to search for a node with a given key
    public boolean search(int key) {
        Node current = head;
        while (current != null) {
            if (current.data == key) {
                return true; // data found
            }
            current = current.next;
        }
        return false; // data not found
    }

    // Method to print the linked list
    public void printList() {
        Node current = head;
        while (current != null) {
            System.out.print(current.data + " -> ");
            current = current.next;
        }
        System.out.println("NULL");
    }

    public static void main(String[] args) {
        Task150 list = new Task150();

        // Test Case 1: Insertion
        System.out.println("--- Test Case 1: Insertion ---");
        list.insert(10);
        list.insert(20);
        list.insert(30);
        list.insert(40);
        list.insert(50);
        System.out.print("Created Linked list: ");
        list.printList();
        System.out.println();

        // Test Case 2: Search for an existing element
        System.out.println("--- Test Case 2: Search (Existing) ---");
        System.out.println("Searching for 30...");
        if (list.search(30)) {
            System.out.println("Element 30 found in the list.");
        } else {
            System.out.println("Element 30 not found in the list.");
        }
        System.out.println();

        // Test Case 3: Search for a non-existing element
        System.out.println("--- Test Case 3: Search (Not Existing) ---");
        System.out.println("Searching for 100...");
        if (list.search(100)) {
            System.out.println("Element 100 found in the list.");
        } else {
            System.out.println("Element 100 not found in the list.");
        }
        System.out.println();

        // Test Case 4: Deletion of a middle element
        System.out.println("--- Test Case 4: Deletion (Middle) ---");
        System.out.println("Deleting element 30...");
        list.delete(30);
        System.out.print("List after deleting 30: ");
        list.printList();
        System.out.println();
        
        // Test Case 5: Deletion of head and tail elements
        System.out.println("--- Test Case 5: Deletion (Head and Tail) ---");
        System.out.println("Deleting element 10 (head)...");
        list.delete(10);
        System.out.print("List after deleting 10: ");
        list.printList();
        
        System.out.println("Deleting element 50 (tail)...");
        list.delete(50);
        System.out.print("List after deleting 50: ");
        list.printList();
    }
}