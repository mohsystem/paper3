import java.util.Objects;

public class Task152 {

    static class HashTable<K, V> {
        private static class Node<K, V> {
            final K key;
            V value;
            Node<K, V> next;

            Node(K key, V value, Node<K, V> next) {
                this.key = key;
                this.value = value;
                this.next = next;
            }
        }

        private Node<K, V>[] table;
        private int size;
        private int capacity;
        private static final int INITIAL_CAPACITY = 16;
        private static final float LOAD_FACTOR_THRESHOLD = 0.75f;

        @SuppressWarnings("unchecked")
        public HashTable() {
            this.capacity = INITIAL_CAPACITY;
            this.table = (Node<K, V>[]) new Node[capacity];
            this.size = 0;
        }

        private int hash(K key) {
            if (key == null) {
                return 0;
            }
            // A secondary hash function to protect against poor hashCode() implementations.
            int h = key.hashCode();
            h ^= (h >>> 20) ^ (h >>> 12);
            return (h ^ (h >>> 7) ^ (h >>> 4)) & (capacity - 1);
        }

        private void rehash() {
            int oldCapacity = this.capacity;
            Node<K, V>[] oldTable = this.table;

            this.capacity = oldCapacity * 2;
            this.size = 0;
            @SuppressWarnings("unchecked")
            Node<K, V>[] newTable = (Node<K, V>[]) new Node[this.capacity];
            this.table = newTable;

            for (int i = 0; i < oldCapacity; i++) {
                Node<K, V> current = oldTable[i];
                while (current != null) {
                    insert(current.key, current.value);
                    current = current.next;
                }
            }
        }

        public void insert(K key, V value) {
            if (key == null) {
                // Disallow null keys for simplicity and security
                return;
            }
            
            if ((float) size / capacity >= LOAD_FACTOR_THRESHOLD) {
                rehash();
            }

            int index = hash(key);
            Node<K, V> head = table[index];

            Node<K, V> current = head;
            while (current != null) {
                if (Objects.equals(current.key, key)) {
                    current.value = value; // Update existing key
                    return;
                }
                current = current.next;
            }

            // Insert new node at the beginning of the chain
            Node<K, V> newNode = new Node<>(key, value, head);
            table[index] = newNode;
            size++;
        }

        public V search(K key) {
            if (key == null) {
                return null;
            }
            int index = hash(key);
            Node<K, V> current = table[index];
            while (current != null) {
                if (Objects.equals(current.key, key)) {
                    return current.value;
                }
                current = current.next;
            }
            return null; // Not found
        }

        public void delete(K key) {
             if (key == null) {
                return;
            }
            int index = hash(key);
            Node<K, V> current = table[index];
            Node<K, V> prev = null;

            while (current != null) {
                if (Objects.equals(current.key, key)) {
                    if (prev == null) { // Node to delete is the head
                        table[index] = current.next;
                    } else {
                        prev.next = current.next;
                    }
                    size--;
                    return;
                }
                prev = current;
                current = current.next;
            }
        }
    }

    public static void main(String[] args) {
        HashTable<String, Integer> ht = new HashTable<>();

        // Test Case 1: Basic insertion and search
        System.out.println("--- Test Case 1: Insert and Search ---");
        ht.insert("apple", 10);
        ht.insert("banana", 20);
        System.out.println("Value for 'apple': " + ht.search("apple")); // Expected: 10
        System.out.println("Value for 'banana': " + ht.search("banana")); // Expected: 20
        System.out.println();

        // Test Case 2: Search for a non-existent key
        System.out.println("--- Test Case 2: Search Non-existent Key ---");
        System.out.println("Value for 'cherry': " + ht.search("cherry")); // Expected: null
        System.out.println();

        // Test Case 3: Update an existing key
        System.out.println("--- Test Case 3: Update Key ---");
        System.out.println("Value for 'apple' before update: " + ht.search("apple")); // Expected: 10
        ht.insert("apple", 15);
        System.out.println("Value for 'apple' after update: " + ht.search("apple")); // Expected: 15
        System.out.println();
        
        // Test Case 4: Deletion
        System.out.println("--- Test Case 4: Deletion ---");
        System.out.println("Value for 'banana' before deletion: " + ht.search("banana")); // Expected: 20
        ht.delete("banana");
        System.out.println("Value for 'banana' after deletion: " + ht.search("banana")); // Expected: null
        System.out.println();

        // Test Case 5: Trigger rehash
        System.out.println("--- Test Case 5: Trigger Rehash ---");
        System.out.println("Initial capacity: " + ht.capacity);
        // Initial capacity is 16, threshold is 0.75 * 16 = 12.
        // We have 1 element ('apple'). Let's add 11 more to trigger rehash.
        for (int i = 0; i < 12; i++) {
            ht.insert("key" + i, i);
        }
        System.out.println("Size: " + ht.size);
        System.out.println("Capacity after rehash: " + ht.capacity); // Expected: 32
        System.out.println("Value for 'key5': " + ht.search("key5")); // Expected: 5
        System.out.println("Value for 'apple': " + ht.search("apple")); // Expected: 15
    }
}