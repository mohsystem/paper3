public class Task150 {

    private Node head;

    // Node inner class for the linked list
    private static class Node {
        int data;
        Node next;

        Node(int d) {
            data = d;
            next = null;
        }
    }

    /**
     * Inserts a new node with the given data at the end of the list.
     * @param data The data for the new node.
     */
    public void insert(int data) {
        Node newNode = new Node(data);

        // If the list is empty, make the new node the head
        if (head == null) {
            head = newNode;
            return;
        }

        // Traverse to the last node
        Node last = head;
        while (last.next != null) {
            last = last.next;
        }

        // Append the new node at the end
        last.next = newNode;
    }

    /**
     * Deletes the first occurrence of a node with the given key.
     * @param key The key of the node to be deleted.
     */
    public void delete(int key) {
        Node temp = head;
        Node prev = null;

        // Case 1: If the head node itself holds the key to be deleted
        if (temp != null && temp.data == key) {
            head = temp.next; // Change head
            return;
        }

        // Case 2: Search for the key, keeping track of the previous node
        while (temp != null && temp.data != key) {
            prev = temp;
            temp = temp.next;
        }

        // If the key was not present in the list
        if (temp == null) {
            return;
        }

        // Unlink the node from the list. This check is a safe programming
        // practice, although prev will not be null here due to the head check.
        if (prev != null) {
            prev.next = temp.next;
        }
    }

    /**
     * Searches for a node with the given key in the list.
     * @param key The key to search for.
     * @return true if the key is found, false otherwise.
     */
    public boolean search(int key) {
        Node current = head;
        while (current != null) {
            if (current.data == key) {
                return true; // Key found
            }
            current = current.next;
        }
        return false; // Key not found
    }

    /**
     * Utility function to print the linked list.
     */
    public void printList() {
        Node current = head;
        if (current == null) {
            System.out.println("List is empty.");
            return;
        }
        while (current != null) {
            System.out.print(current.data + " -> ");
            current = current.next;
        }
        System.out.println("NULL");
    }

    /**
     * Main method with test cases.
     */
    public static void main(String[] args) {
        Task150 list = new Task150();

        // Test Case 1: Insertion
        System.out.println("--- Test Case 1: Insertion ---");
        list.insert(10);
        list.insert(20);
        list.insert(30);
        list.insert(40);
        System.out.print("List after inserting 10, 20, 30, 40: ");
        list.printList();
        System.out.println();

        // Test Case 2: Search for an existing element
        System.out.println("--- Test Case 2: Search (Existing) ---");
        System.out.println("Searching for 30: " + (list.search(30) ? "Found" : "Not Found"));
        System.out.println();

        // Test Case 3: Search for a non-existing element
        System.out.println("--- Test Case 3: Search (Non-Existing) ---");
        System.out.println("Searching for 99: " + (list.search(99) ? "Found" : "Not Found"));
        System.out.println();

        // Test Case 4: Delete an element from the middle
        System.out.println("--- Test Case 4: Deletion (Middle) ---");
        System.out.print("List before deleting 20: ");
        list.printList();
        list.delete(20);
        System.out.print("List after deleting 20: ");
        list.printList();
        System.out.println();

        // Test Case 5: Delete the head element
        System.out.println("--- Test Case 5: Deletion (Head) ---");
        System.out.print("List before deleting 10: ");
        list.printList();
        list.delete(10);
        System.out.print("List after deleting 10: ");
        list.printList();
    }
}