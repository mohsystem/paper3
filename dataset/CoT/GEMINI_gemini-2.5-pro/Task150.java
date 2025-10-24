class Node {
    int data;
    Node next;

    // Constructor to create a new node
    public Node(int data) {
        this.data = data;
        this.next = null;
    }
}

public class Task150 {

    /**
     * Inserts a new node at the beginning of the list.
     * @param head The current head of the list.
     * @param data The data for the new node.
     * @return The new head of the list.
     */
    public static Node insert(Node head, int data) {
        Node newNode = new Node(data);
        newNode.next = head;
        return newNode;
    }

    /**
     * Deletes the first occurrence of a node with the given key.
     * @param head The current head of the list.
     * @param key The data of the node to be deleted.
     * @return The new head of the list.
     */
    public static Node delete(Node head, int key) {
        // Security: Handle case where list is empty.
        if (head == null) {
            return null;
        }

        // Case 1: The head node itself holds the key to be deleted.
        if (head.data == key) {
            return head.next; // The new head is the next node.
        }

        Node current = head;
        // Case 2: The key is somewhere other than the head.
        // Traverse the list to find the node *before* the one to be deleted.
        // Security: Check for current.next != null to prevent NullPointerException.
        while (current.next != null && current.next.data != key) {
            current = current.next;
        }

        // If the key was found (current.next is the node to delete).
        if (current.next != null) {
            current.next = current.next.next; // Unlink the node.
        }

        return head;
    }

    /**
     * Searches for a node with the given key.
     * @param head The head of the list.
     * @param key The data to search for.
     * @return true if the key is found, false otherwise.
     */
    public static boolean search(Node head, int key) {
        Node current = head;
        // Security: Loop condition prevents dereferencing a null pointer.
        while (current != null) {
            if (current.data == key) {
                return true;
            }
            current = current.next;
        }
        return false;
    }

    /**
     * Utility function to print the linked list.
     * @param head The head of the list.
     */
    public static void printList(Node head) {
        Node current = head;
        while (current != null) {
            System.out.print(current.data + " -> ");
            current = current.next;
        }
        System.out.println("null");
    }

    public static void main(String[] args) {
        Node head = null;

        // Test Case 1: Insertion
        System.out.println("Test Case 1: Insertion");
        head = insert(head, 30);
        head = insert(head, 20);
        head = insert(head, 10);
        System.out.print("List after insertions: ");
        printList(head); // Expected: 10 -> 20 -> 30 -> null
        System.out.println("--------------------");

        // Test Case 2: Search for an existing element
        System.out.println("Test Case 2: Search for existing element (20)");
        System.out.println("Found: " + search(head, 20)); // Expected: true
        System.out.println("--------------------");
        
        // Test Case 3: Search for a non-existent element
        System.out.println("Test Case 3: Search for non-existent element (50)");
        System.out.println("Found: " + search(head, 50)); // Expected: false
        System.out.println("--------------------");
        
        // Test Case 4: Delete an element from the middle
        System.out.println("Test Case 4: Delete middle element (20)");
        head = delete(head, 20);
        System.out.print("List after deleting 20: ");
        printList(head); // Expected: 10 -> 30 -> null
        System.out.println("--------------------");
        
        // Test Case 5: Delete head, non-existent, and from empty list
        System.out.println("Test Case 5: Complex Deletions");
        head = delete(head, 10); // Delete head
        System.out.print("List after deleting head (10): ");
        printList(head); // Expected: 30 -> null
        head = delete(head, 100); // Delete non-existent
        System.out.print("List after attempting to delete 100: ");
        printList(head); // Expected: 30 -> null
        head = delete(head, 30); // Delete last element
        System.out.print("List after deleting 30: ");
        printList(head); // Expected: null
        head = delete(head, 5); // Delete from empty list
        System.out.print("List after deleting from empty list: ");
        printList(head); // Expected: null
        System.out.println("--------------------");
    }
}