import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

public class Task152 {

    private static final class Node {
        final int key;
        int value;
        Node(int k, int v) { this.key = k; this.value = v; }
    }

    public static final class HashTable {
        private final int capacity;
        private final List<LinkedList<Node>> buckets;

        public HashTable(int capacity) {
            if (capacity <= 0) {
                throw new IllegalArgumentException("Capacity must be positive");
            }
            this.capacity = capacity;
            this.buckets = new ArrayList<>(capacity);
            for (int i = 0; i < capacity; i++) {
                buckets.add(new LinkedList<>());
            }
        }

        private int index(int key) {
            int h = key * 0x9E3779B9; // multiplicative hashing
            h ^= (h >>> 16);
            if (h < 0) h = -h;
            return h % capacity;
        }

        // Inserts key->value. Returns true if inserted new, false if updated existing.
        public boolean insert(int key, int value) {
            int idx = index(key);
            LinkedList<Node> bucket = buckets.get(idx);
            for (Node n : bucket) {
                if (n.key == key) {
                    n.value = value;
                    return false;
                }
            }
            bucket.add(new Node(key, value));
            return true;
        }

        // Deletes key. Returns true if existed and removed, false otherwise.
        public boolean delete(int key) {
            int idx = index(key);
            LinkedList<Node> bucket = buckets.get(idx);
            var it = bucket.iterator();
            while (it.hasNext()) {
                Node n = it.next();
                if (n.key == key) {
                    it.remove();
                    return true;
                }
            }
            return false;
        }

        // Searches key. Returns Integer value if found, otherwise null.
        public Integer search(int key) {
            int idx = index(key);
            LinkedList<Node> bucket = buckets.get(idx);
            for (Node n : bucket) {
                if (n.key == key) {
                    return n.value;
                }
            }
            return null;
        }
    }

    public static void main(String[] args) {
        HashTable ht = new HashTable(17);

        // Test 1: Insert and search basic
        System.out.println("Test1 Insert new keys: "
                + (ht.insert(1, 100) && ht.insert(2, 200) && ht.insert(3, 300)));
        System.out.println("Test1 Search keys: "
                + (Integer.valueOf(100).equals(ht.search(1)) && Integer.valueOf(200).equals(ht.search(2))
                && Integer.valueOf(300).equals(ht.search(3))));

        // Test 2: Update existing key
        boolean updated = !ht.insert(2, 250); // should update
        Integer v2 = ht.search(2);
        System.out.println("Test2 Update existing: " + (updated && Integer.valueOf(250).equals(v2)));

        // Test 3: Delete existing key
        boolean del3 = ht.delete(3);
        boolean notFound3 = (ht.search(3) == null);
        System.out.println("Test3 Delete existing: " + (del3 && notFound3));

        // Test 4: Delete non-existing key
        boolean del99 = ht.delete(99);
        System.out.println("Test4 Delete non-existing: " + (!del99));

        // Test 5: Search non-existing key after sequence
        boolean searchNF = (ht.search(42) == null);
        System.out.println("Test5 Search non-existing: " + searchNF);
    }
}