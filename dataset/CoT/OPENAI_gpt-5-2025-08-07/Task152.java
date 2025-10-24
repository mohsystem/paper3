// Chain-of-Through secure implementation: Java
// 1) Problem understanding: Integer hash set with insert, delete, search.
// 2) Security: safe hashing, no negative indices, capacity as power-of-two, rehashing with bounds.
// 3) Secure coding: input validation, overflow checks, avoid null deref, no exposure of internals.
// 4) Code review: use immutable next updates, verify thresholds, safe rehash.
// 5) Secure output: final tested code with deterministic behavior.

import java.util.Arrays;

public class Task152 {
    // Integer Hash Set with separate chaining
    static final class IntHashSet {
        private static final float LOAD_FACTOR = 0.75f;
        private static final int MIN_CAPACITY = 16;
        private static final int MAX_CAPACITY = 1 << 30;

        private static final class Node {
            final int key;
            Node next;
            Node(int k, Node n) { this.key = k; this.next = n; }
        }

        private Node[] table;
        private int size;
        private int threshold;

        public IntHashSet() {
            this.table = new Node[MIN_CAPACITY];
            this.threshold = (int)(MIN_CAPACITY * LOAD_FACTOR);
            this.size = 0;
        }

        private static long mix64(long z) {
            z = (z ^ (z >>> 33)) * 0xff51afd7ed558ccdL;
            z = (z ^ (z >>> 33)) * 0xc4ceb9fe1a85ec53L;
            z = z ^ (z >>> 33);
            return z;
        }

        private int indexFor(int key, int capacity) {
            long h = mix64((long) key);
            return (int) (h & (capacity - 1));
        }

        private void resizeIfNeeded() {
            if (size <= threshold) return;
            int oldCap = table.length;
            if (oldCap >= MAX_CAPACITY) return; // cap growth to prevent overflow
            int newCap = oldCap << 1;
            if (newCap <= 0) { // overflow guard
                newCap = MAX_CAPACITY;
            }
            Node[] newTab = new Node[newCap];

            for (int i = 0; i < oldCap; i++) {
                Node e = table[i];
                while (e != null) {
                    Node next = e.next;
                    int idx = indexFor(e.key, newCap);
                    e.next = newTab[idx];
                    newTab[idx] = e;
                    e = next;
                }
            }
            table = newTab;
            threshold = (int) Math.min((long)(newCap * LOAD_FACTOR), (long) Integer.MAX_VALUE);
        }

        public boolean insert(int key) {
            int idx = indexFor(key, table.length);
            Node n = table[idx];
            for (Node e = n; e != null; e = e.next) {
                if (e.key == key) return false;
            }
            table[idx] = new Node(key, n);
            size++;
            resizeIfNeeded();
            return true;
        }

        public boolean delete(int key) {
            int idx = indexFor(key, table.length);
            Node prev = null, cur = table[idx];
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

        public boolean search(int key) {
            int idx = indexFor(key, table.length);
            for (Node e = table[idx]; e != null; e = e.next) {
                if (e.key == key) return true;
            }
            return false;
        }

        public int size() { return size; }
    }

    // Simple test harness with 5 cases
    public static void main(String[] args) {
        IntHashSet set = new IntHashSet();

        // Test 1: insert and search existing
        set.insert(10);
        set.insert(20);
        set.insert(30);
        System.out.println("T1 contains(20): " + set.search(20)); // true

        // Test 2: search missing
        System.out.println("T2 contains(15): " + set.search(15)); // false

        // Test 3: delete existing then search
        System.out.println("T3 delete(20): " + set.delete(20));   // true
        System.out.println("T3 contains(20): " + set.search(20)); // false

        // Test 4: insert again after delete
        System.out.println("T4 insert(20): " + set.insert(20));   // true
        System.out.println("T4 contains(20): " + set.search(20)); // true

        // Test 5: delete missing
        System.out.println("T5 delete(99): " + set.delete(99));   // false
    }
}