import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;

public class Task152 {
    static final class HashTable {
        private static final int DEFAULT_CAPACITY = 16;
        private static final double LOAD_FACTOR = 0.75;
        private Node[] buckets;
        private int size;
        private int threshold;
        private final long salt;

        private static final class Node {
            final String key;
            int value;
            Node next;
            Node(String key, int value, Node next) {
                this.key = key;
                this.value = value;
                this.next = next;
            }
        }

        public HashTable() {
            this(DEFAULT_CAPACITY);
        }

        public HashTable(int capacity) {
            if (capacity <= 0) {
                capacity = DEFAULT_CAPACITY;
            }
            capacity = nextPowerOfTwo(capacity);
            this.buckets = new Node[capacity];
            this.threshold = (int) Math.floor(capacity * LOAD_FACTOR);
            SecureRandom sr = new SecureRandom();
            this.salt = sr.nextLong();
            this.size = 0;
        }

        private static int nextPowerOfTwo(int n) {
            int c = 1;
            while (c < n) c <<= 1;
            return c;
        }

        private long fnv1a64(byte[] data) {
            long hash = 0xcbf29ce484222325L; // 14695981039346656037
            final long prime = 0x100000001b3L; // 1099511628211
            for (byte b : data) {
                hash ^= (b & 0xffL);
                hash *= prime;
            }
            return hash;
        }

        private int index(String key) {
            byte[] bytes = key.getBytes(StandardCharsets.UTF_8);
            long h = fnv1a64(bytes) ^ salt;
            // Java's modulo for negative numbers can be negative; use mask by power-of-two
            return (int) (h & (buckets.length - 1));
        }

        private void resizeIfNeeded() {
            if (size <= threshold) return;
            int newCap = buckets.length << 1;
            Node[] newBuckets = new Node[newCap];
            for (Node head : buckets) {
                Node curr = head;
                while (curr != null) {
                    Node next = curr.next;
                    byte[] bytes = curr.key.getBytes(StandardCharsets.UTF_8);
                    long h = fnv1a64(bytes) ^ salt;
                    int idx = (int) (h & (newCap - 1));
                    curr.next = newBuckets[idx];
                    newBuckets[idx] = curr;
                    curr = next;
                }
            }
            buckets = newBuckets;
            threshold = (int) Math.floor(newCap * LOAD_FACTOR);
        }

        // Inserts or updates. Returns true if new key inserted, false if updated.
        public boolean put(String key, int value) {
            if (key == null) throw new IllegalArgumentException("key must not be null");
            int idx = index(key);
            Node curr = buckets[idx];
            while (curr != null) {
                if (curr.key.equals(key)) {
                    curr.value = value;
                    return false;
                }
                curr = curr.next;
            }
            buckets[idx] = new Node(key, value, buckets[idx]);
            size++;
            resizeIfNeeded();
            return true;
        }

        // Deletes key. Returns true if deleted, false if not found.
        public boolean delete(String key) {
            if (key == null) throw new IllegalArgumentException("key must not be null");
            int idx = index(key);
            Node curr = buckets[idx];
            Node prev = null;
            while (curr != null) {
                if (curr.key.equals(key)) {
                    if (prev == null) buckets[idx] = curr.next;
                    else prev.next = curr.next;
                    size--;
                    return true;
                }
                prev = curr;
                curr = curr.next;
            }
            return false;
        }

        // Returns value if present, else null.
        public Integer get(String key) {
            if (key == null) throw new IllegalArgumentException("key must not be null");
            int idx = index(key);
            Node curr = buckets[idx];
            while (curr != null) {
                if (curr.key.equals(key)) return curr.value;
                curr = curr.next;
            }
            return null;
        }

        public int size() {
            return size;
        }
    }

    public static void main(String[] args) {
        HashTable ht = new HashTable();

        // Test case 1: insert and search
        ht.put("apple", 1);
        ht.put("banana", 2);
        ht.put("cherry", 3);
        System.out.println("T1 banana=" + ht.get("banana")); // expect 2

        // Test case 2: update existing
        ht.put("banana", 20);
        System.out.println("T2 banana=" + ht.get("banana")); // expect 20

        // Test case 3: delete and search
        boolean deleted = ht.delete("apple");
        System.out.println("T3 apple_deleted=" + deleted + " val=" + ht.get("apple")); // expect true, null

        // Test case 4: search non-existent
        System.out.println("T4 durian=" + ht.get("durian")); // expect null

        // Test case 5: insert more and verify
        ht.put("elderberry", 5);
        ht.put("fig", 6);
        System.out.println("T5 elderberry=" + ht.get("elderberry") + " size=" + ht.size()); // expect 5 and size
    }
}