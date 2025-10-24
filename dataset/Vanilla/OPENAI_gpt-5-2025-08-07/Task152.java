import java.util.*;

public class Task152 {
    static class Node {
        int key, value;
        Node next;
        Node(int k, int v, Node n) { key = k; value = v; next = n; }
    }

    static class HashTable {
        private Node[] table;
        private int capacity;
        private int size;

        public HashTable(int capacity) {
            this.capacity = Math.max(4, capacity);
            this.table = new Node[this.capacity];
            this.size = 0;
        }

        private int index(int key) {
            int h = Integer.hashCode(key);
            h ^= (h >>> 16);
            if (h < 0) h = -h;
            return h % capacity;
        }

        public void put(int key, int value) {
            int idx = index(key);
            Node cur = table[idx];
            while (cur != null) {
                if (cur.key == key) {
                    cur.value = value;
                    return;
                }
                cur = cur.next;
            }
            table[idx] = new Node(key, value, table[idx]);
            size++;
        }

        public Integer get(int key) {
            int idx = index(key);
            Node cur = table[idx];
            while (cur != null) {
                if (cur.key == key) return cur.value;
                cur = cur.next;
            }
            return null;
        }

        public boolean remove(int key) {
            int idx = index(key);
            Node cur = table[idx], prev = null;
            while (cur != null) {
                if (cur.key == key) {
                    if (prev == null) table[idx] = cur.next;
                    else prev.next = cur.next;
                    size--;
                    return true;
                }
                prev = cur;
                cur = cur.next;
            }
            return false;
        }

        public int size() {
            return size;
        }
    }

    public static void main(String[] args) {
        HashTable ht = new HashTable(16);

        // Test 1: Insert and search
        ht.put(1, 10);
        System.out.println("Test1 get(1): " + ht.get(1)); // 10

        // Test 2: Collision insert and search
        ht.put(17, 20); // likely collides with 1 if capacity 16
        System.out.println("Test2 get(17): " + ht.get(17)); // 20

        // Test 3: Update existing key
        ht.put(1, 15);
        System.out.println("Test3 get(1): " + ht.get(1)); // 15

        // Test 4: Delete existing key and verify
        System.out.println("Test4 remove(17): " + ht.remove(17)); // true
        System.out.println("Test4 get(17): " + ht.get(17)); // null

        // Test 5: Delete non-existing key and size check
        System.out.println("Test5 remove(99): " + ht.remove(99)); // false
        System.out.println("Test5 size(): " + ht.size()); // should be 1
    }
}