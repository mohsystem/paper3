import java.util.Objects;

public class Task152 {

    private static class Node {
        final int key;
        int value;
        Node next;

        Node(int key, int value) {
            this.key = key;
            this.value = value;
            this.next = null;
        }
    }

    private Node[] table;
    private int capacity;

    public Task152(int capacity) {
        if (capacity <= 0) {
            throw new IllegalArgumentException("Capacity must be positive.");
        }
        this.capacity = capacity;
        this.table = new Node[this.capacity];
    }

    private int hash(int key) {
        // Ensure the hash is non-negative
        return Math.abs(key % capacity);
    }

    public void insert(int key, int value) {
        int index = hash(key);
        Node head = table[index];

        // Search for the key. If it exists, update the value.
        Node current = head;
        while (current != null) {
            if (current.key == key) {
                current.value = value;
                return;
            }
            current = current.next;
        }

        // If key not found, insert a new node at the beginning of the chain.
        Node newNode = new Node(key, value);
        newNode.next = head;
        table[index] = newNode;
    }

    public Integer search(int key) {
        int index = hash(key);
        Node current = table[index];

        while (current != null) {
            if (current.key == key) {
                return current.value;
            }
            current = current.next;
        }

        return null; // Key not found
    }

    public void delete(int key) {
        int index = hash(key);
        Node current = table[index];
        Node prev = null;

        while (current != null) {
            if (current.key == key) {
                if (prev == null) {
                    // Node to be deleted is the head of the chain
                    table[index] = current.next;
                } else {
                    // Node is in the middle or at the end
                    prev.next = current.next;
                }
                return; // Key found and deleted
            }
            prev = current;
            current = current.next;
        }
    }
    
    public void printTable() {
        System.out.println("---- Hash Table ----");
        for (int i = 0; i < capacity; i++) {
            System.out.print("Bucket " + i + ": ");
            Node current = table[i];
            if (current == null) {
                System.out.println("[]");
                continue;
            }
            while(current != null) {
                System.out.print("[K:" + current.key + ", V:" + current.value + "] -> ");
                current = current.next;
            }
            System.out.println("null");
        }
        System.out.println("--------------------");
    }

    public static void main(String[] args) {
        Task152 ht = new Task152(10);

        // Test Case 1: Insert new key-value pairs
        System.out.println("Test Case 1: Inserting new elements");
        ht.insert(1, 10);
        ht.insert(11, 110); // Should collide with key 1 if capacity is 10
        ht.insert(2, 20);
        ht.insert(12, 120);
        ht.printTable();

        // Test Case 2: Search for an existing key
        System.out.println("\nTest Case 2: Searching for key 11");
        Integer value = ht.search(11);
        System.out.println("Value for key 11: " + (value != null ? value : "Not Found"));

        // Test Case 3: Search for a non-existing key
        System.out.println("\nTest Case 3: Searching for key 99");
        value = ht.search(99);
        System.out.println("Value for key 99: " + (value != null ? value : "Not Found"));

        // Test Case 4: Update an existing key
        System.out.println("\nTest Case 4: Updating key 1 to value 100");
        System.out.println("Value for key 1 before update: " + ht.search(1));
        ht.insert(1, 100);
        System.out.println("Value for key 1 after update: " + ht.search(1));
        ht.printTable();

        // Test Case 5: Delete a key and verify
        System.out.println("\nTest Case 5: Deleting key 11");
        ht.delete(11);
        System.out.println("Searching for key 11 after deletion...");
        value = ht.search(11);
        System.out.println("Value for key 11: " + (value != null ? value : "Not Found"));
        ht.printTable();
    }
}