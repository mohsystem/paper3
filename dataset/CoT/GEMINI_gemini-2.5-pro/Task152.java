import java.util.Objects;

class Task152 {

    // Node for storing key-value pairs in the hash table
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

    private final HashNode[] buckets;
    private final int capacity;

    // Constructor
    public Task152(int capacity) {
        if (capacity <= 0) {
            throw new IllegalArgumentException("Capacity must be a positive integer.");
        }
        this.capacity = capacity;
        this.buckets = new HashNode[capacity];
    }

    // Hash function to find index for a key
    private int getBucketIndex(int key) {
        // Ensure index is non-negative
        return Math.abs(key % capacity);
    }

    // Method to search for a key
    public Integer search(int key) {
        int bucketIndex = getBucketIndex(key);
        HashNode head = buckets[bucketIndex];

        // Search key in the chain
        while (head != null) {
            if (head.key == key) {
                return head.value;
            }
            head = head.next;
        }
        // If key not found
        return -1;
    }

    // Method to insert a key-value pair
    public void insert(int key, int value) {
        int bucketIndex = getBucketIndex(key);
        HashNode head = buckets[bucketIndex];

        // Check if key is already present
        while (head != null) {
            if (head.key == key) {
                head.value = value; // Update value
                return;
            }
            head = head.next;
        }

        // Insert key in the chain
        head = buckets[bucketIndex];
        HashNode newNode = new HashNode(key, value);
        newNode.next = head;
        buckets[bucketIndex] = newNode;
    }

    // Method to delete a key
    public void delete(int key) {
        int bucketIndex = getBucketIndex(key);
        HashNode head = buckets[bucketIndex];

        // If key is at the head of the chain
        if (head != null && head.key == key) {
            buckets[bucketIndex] = head.next;
            return;
        }

        // Search for the key to be deleted, keep track of the previous node
        HashNode prev = null;
        while (head != null && head.key != key) {
            prev = head;
            head = head.next;
        }

        // If key was not present
        if (head == null) {
            return;
        }

        // Unlink the node from the linked list
        if (prev != null) {
           prev.next = head.next;
        }
    }

    public static void main(String[] args) {
        Task152 hashTable = new Task152(10);
        System.out.println("Java Hash Table Implementation Test");

        // Test Case 1: Insert key-value pairs. (5 and 15 will collide with capacity 10)
        hashTable.insert(5, 50);
        hashTable.insert(15, 150);
        hashTable.insert(2, 20);
        System.out.println("Test 1: Insert (5, 50), (15, 150), (2, 20)");
        System.out.println("Value for key 15: " + hashTable.search(15));
        System.out.println("Value for key 5: " + hashTable.search(5));
        System.out.println("Value for key 2: " + hashTable.search(2));
        System.out.println("--------------------");

        // Test Case 2: Search for an existing key.
        System.out.println("Test 2: Search for existing key 15");
        System.out.println("Value: " + hashTable.search(15));
        System.out.println("--------------------");

        // Test Case 3: Search for a non-existing key.
        System.out.println("Test 3: Search for non-existing key 25");
        System.out.println("Value: " + hashTable.search(25));
        System.out.println("--------------------");
        
        // Test Case 4: Update an existing key's value.
        System.out.println("Test 4: Update value for key 5 to 55");
        hashTable.insert(5, 55);
        System.out.println("New value for key 5: " + hashTable.search(5));
        System.out.println("--------------------");

        // Test Case 5: Delete a key and verify deletion.
        System.out.println("Test 5: Delete key 15");
        hashTable.delete(15);
        System.out.println("Searching for deleted key 15: " + hashTable.search(15));
        System.out.println("Searching for key 5 to ensure it's still there: " + hashTable.search(5));
        System.out.println("--------------------");
    }
}