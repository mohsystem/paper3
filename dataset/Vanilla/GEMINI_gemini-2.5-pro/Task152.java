public class Task152 {

    // Node class for the linked list (chaining)
    static class HashNode {
        int key;
        int value;
        HashNode next;

        public HashNode(int key, int value) {
            this.key = key;
            this.value = value;
            this.next = null;
        }
    }

    private HashNode[] table;
    private int capacity;

    // Constructor
    public Task152(int capacity) {
        this.capacity = capacity;
        table = new HashNode[capacity];
        // Initialize chains
        for (int i = 0; i < capacity; i++) {
            table[i] = null;
        }
    }

    // Hash function
    private int hashFunction(int key) {
        // Simple modulo hash. Handle negative keys.
        return Math.abs(key % capacity);
    }

    // Insert a key-value pair
    public void insert(int key, int value) {
        int index = hashFunction(key);
        HashNode head = table[index];
        
        // Check if key already exists, if so, update value
        HashNode current = head;
        while (current != null) {
            if (current.key == key) {
                current.value = value; // Update value
                return;
            }
            current = current.next;
        }

        // Key does not exist, insert new node at the beginning of the chain
        HashNode newNode = new HashNode(key, value);
        newNode.next = head;
        table[index] = newNode;
    }

    // Search for a key and return its value
    public Integer search(int key) {
        int index = hashFunction(key);
        HashNode head = table[index];

        // Traverse the chain
        HashNode current = head;
        while (current != null) {
            if (current.key == key) {
                return current.value;
            }
            current = current.next;
        }

        // Key not found
        return null;
    }

    // Delete a key
    public void delete(int key) {
        int index = hashFunction(key);
        HashNode head = table[index];
        HashNode prev = null;
        HashNode current = head;

        // Find the key in the chain
        while (current != null) {
            if (current.key == key) {
                break;
            }
            prev = current;
            current = current.next;
        }

        // If key was not present in chain
        if (current == null) {
            return;
        }

        // Unlink the node from the linked list
        if (prev != null) {
            prev.next = current.next;
        } else {
            // It's the head of the chain
            table[index] = current.next;
        }
    }
    
    // Display method for testing
    public void display() {
        System.out.println("---- HASH TABLE ----");
        for (int i = 0; i < capacity; i++) {
            System.out.print("Index " + i + ":");
            HashNode current = table[i];
            while (current != null) {
                System.out.print(" -> (" + current.key + ", " + current.value + ")");
                current = current.next;
            }
            System.out.println();
        }
        System.out.println("--------------------");
    }

    // Main method with test cases
    public static void main(String[] args) {
        Task152 hashTable = new Task152(10);
        
        System.out.println("----- Testing Insert and Search -----");
        hashTable.insert(1, 10);
        hashTable.insert(2, 20);
        hashTable.insert(12, 120); // Collision with key 2
        hashTable.display();
        
        System.out.println("\nSearch for key 2: " + hashTable.search(2)); // Expected: 20
        System.out.println("Search for key 12: " + hashTable.search(12)); // Expected: 120
        System.out.println("Search for key 5 (non-existent): " + hashTable.search(5)); // Expected: null

        System.out.println("\n----- Testing Update -----");
        hashTable.insert(2, 22); // Update value for key 2
        System.out.println("Search for key 2 after update: " + hashTable.search(2)); // Expected: 22
        hashTable.display();

        System.out.println("\n----- Testing Delete -----");
        hashTable.delete(12);
        System.out.println("Search for key 12 after delete: " + hashTable.search(12)); // Expected: null
        System.out.println("Search for key 2 after deleting 12: " + hashTable.search(2)); // Expected: 22
        hashTable.display();

        System.out.println("\n----- More Test Cases -----");
        hashTable.insert(22, 220); // Another collision at index 2
        hashTable.insert(32, 320); // Another collision at index 2
        hashTable.display();
        System.out.println("Deleting head of a chain (key 32)...");
        hashTable.delete(32);
        hashTable.display();
        System.out.println("Search for 32: " + hashTable.search(32)); // Expected: null
        System.out.println("Search for 22: " + hashTable.search(22)); // Expected: 220
    }
}