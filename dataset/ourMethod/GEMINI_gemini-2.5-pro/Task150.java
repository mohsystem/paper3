public class Task150 {

    static class Node {
        int data;
        Node next;

        Node(int d) {
            this.data = d;
            this.next = null;
        }
    }

    static class SinglyLinkedList {
        Node head;

        public SinglyLinkedList() {
            this.head = null;
        }

        /**
         * Inserts a new node at the front of the list.
         * @param newData The data for the new node.
         */
        public void insert(int newData) {
            Node newNode = new Node(newData);
            newNode.next = head;
            head = newNode;
        }

        /**
         * Deletes the first occurrence of a node with the given key.
         * @param key The data of the node to be deleted.
         */
        public void delete(int key) {
            Node temp = head, prev = null;

            // If the head node itself holds the key to be deleted
            if (temp != null && temp.data == key) {
                head = temp.next; // Change head
                return;
            }

            // Search for the key to be deleted, keeping track of the previous node
            while (temp != null && temp.data != key) {
                prev = temp;
                temp = temp.next;
            }

            // If key was not present in the linked list
            if (temp == null) {
                return;
            }

            // Unlink the node from the linked list
            if (prev != null) {
                prev.next = temp.next;
            }
        }

        /**
         * Searches for a key in the linked list.
         * @param key The data to search for.
         * @return true if the key is found, false otherwise.
         */
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

        /**
         * Utility function to print the linked list.
         */
        public void printList() {
            Node tnode = head;
            while (tnode != null) {
                System.out.print(tnode.data + " -> ");
                tnode = tnode.next;
            }
            System.out.println("NULL");
        }
    }

    public static void main(String[] args) {
        SinglyLinkedList list = new SinglyLinkedList();

        // Test Case 1: Insertion
        System.out.println("--- Test Case 1: Insertion ---");
        list.insert(3);
        list.insert(2);
        list.insert(1);
        System.out.print("Created list: ");
        list.printList();
        System.out.println();

        // Test Case 2: Search for an existing element
        System.out.println("--- Test Case 2: Search (existing) ---");
        int keyToSearch1 = 2;
        System.out.println("Searching for " + keyToSearch1 + ": " + (list.search(keyToSearch1) ? "Found" : "Not Found"));
        System.out.println();

        // Test Case 3: Search for a non-existing element
        System.out.println("--- Test Case 3: Search (non-existing) ---");
        int keyToSearch2 = 4;
        System.out.println("Searching for " + keyToSearch2 + ": " + (list.search(keyToSearch2) ? "Found" : "Not Found"));
        System.out.println();

        // Test Case 4: Delete an element from the middle
        System.out.println("--- Test Case 4: Deletion (middle) ---");
        int keyToDelete1 = 2;
        System.out.println("Deleting " + keyToDelete1);
        list.delete(keyToDelete1);
        System.out.print("List after deletion: ");
        list.printList();
        System.out.println();
        
        // Test Case 5: Delete the head element
        System.out.println("--- Test Case 5: Deletion (head) ---");
        int keyToDelete2 = 1;
        System.out.println("Deleting " + keyToDelete2);
        list.delete(keyToDelete2);
        System.out.print("List after deletion: ");
        list.printList();
    }
}