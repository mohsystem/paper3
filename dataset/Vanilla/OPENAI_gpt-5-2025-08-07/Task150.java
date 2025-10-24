import java.util.*;

public class Task150 {

    static class SinglyLinkedList {
        private static class Node {
            int val;
            Node next;
            Node(int v) { this.val = v; }
        }

        private Node head;

        // Insert at the end
        public void insert(int value) {
            if (head == null) {
                head = new Node(value);
                return;
            }
            Node cur = head;
            while (cur.next != null) cur = cur.next;
            cur.next = new Node(value);
        }

        // Delete first occurrence of value, return true if deleted
        public boolean delete(int value) {
            Node cur = head, prev = null;
            while (cur != null) {
                if (cur.val == value) {
                    if (prev == null) head = cur.next;
                    else prev.next = cur.next;
                    return true;
                }
                prev = cur;
                cur = cur.next;
            }
            return false;
        }

        // Search for value
        public boolean search(int value) {
            Node cur = head;
            while (cur != null) {
                if (cur.val == value) return true;
                cur = cur.next;
            }
            return false;
        }

        public int[] toArray() {
            int n = 0;
            Node cur = head;
            while (cur != null) { n++; cur = cur.next; }
            int[] arr = new int[n];
            cur = head;
            int i = 0;
            while (cur != null) {
                arr[i++] = cur.val;
                cur = cur.next;
            }
            return arr;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("[");
            Node cur = head;
            while (cur != null) {
                sb.append(cur.val);
                if (cur.next != null) sb.append(", ");
                cur = cur.next;
            }
            sb.append("]");
            return sb.toString();
        }
    }

    public static void main(String[] args) {
        // Test 1: Insert sequence
        SinglyLinkedList list = new SinglyLinkedList();
        list.insert(10);
        list.insert(20);
        list.insert(30);
        System.out.println("Test1 list after inserts: " + list);

        // Test 2: Search existing
        System.out.println("Test2 search 20: " + list.search(20));

        // Test 3: Delete middle
        System.out.println("Test3 delete 20: " + list.delete(20) + ", list: " + list);

        // Test 4: Delete head and tail
        System.out.println("Test4 delete 10: " + list.delete(10) + ", delete 30: " + list.delete(30) + ", list: " + list);

        // Test 5: Delete on empty and search non-existing
        System.out.println("Test5 delete 99 on empty: " + list.delete(99) + ", search 99: " + list.search(99));
    }
}