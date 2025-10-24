// Chain-of-Through process in code generation:
// 1) Problem understanding: Implement a singly linked list with insert, delete, and search operations.
// 2) Security requirements: Validate indices, avoid null dereferences, and ensure robust handling of edge cases.
// 3) Secure coding generation: Defensive checks on indices and nulls, no external inputs, clear ownership of objects.
// 4) Code review: Checked for off-by-one errors, null checks, and consistent size updates.
// 5) Secure code output: Final code includes safe operations and comprehensive tests.

public class Task150 {
    static final class SinglyLinkedList {
        private static final class Node {
            int val;
            Node next;
            Node(int v) { this.val = v; }
        }
        private Node head;
        private int size;

        public int size() { return size; }

        public void insertAtHead(int value) {
            Node n = new Node(value);
            n.next = head;
            head = n;
            size++;
        }

        public void insertAtTail(int value) {
            Node n = new Node(value);
            if (head == null) {
                head = n;
            } else {
                Node cur = head;
                while (cur.next != null) cur = cur.next;
                cur.next = n;
            }
            size++;
        }

        public boolean insertAtIndex(int index, int value) {
            if (index < 0 || index > size) return false;
            if (index == 0) {
                insertAtHead(value);
                return true;
            }
            Node prev = head;
            for (int i = 0; i < index - 1; i++) {
                if (prev == null) return false; // defensive
                prev = prev.next;
            }
            Node n = new Node(value);
            n.next = prev.next;
            prev.next = n;
            size++;
            return true;
        }

        public boolean deleteByValue(int value) {
            Node prev = null;
            Node cur = head;
            while (cur != null && cur.val != value) {
                prev = cur;
                cur = cur.next;
            }
            if (cur == null) return false;
            if (prev == null) head = cur.next;
            else prev.next = cur.next;
            size--;
            return true;
        }

        public boolean deleteAtIndex(int index) {
            if (index < 0 || index >= size) return false;
            if (index == 0) {
                if (head != null) {
                    head = head.next;
                    size--;
                    return true;
                }
                return false;
            }
            Node prev = head;
            for (int i = 0; i < index - 1; i++) {
                if (prev == null) return false;
                prev = prev.next;
            }
            if (prev == null || prev.next == null) return false;
            prev.next = prev.next.next;
            size--;
            return true;
        }

        public int indexOf(int value) {
            int idx = 0;
            Node cur = head;
            while (cur != null) {
                if (cur.val == value) return idx;
                cur = cur.next;
                idx++;
            }
            return -1;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("[");
            Node cur = head;
            while (cur != null) {
                sb.append(cur.val);
                if (cur.next != null) sb.append(" -> ");
                cur = cur.next;
            }
            sb.append("]");
            return sb.toString();
        }
    }

    public static void main(String[] args) {
        // 5 Test cases
        SinglyLinkedList list = new SinglyLinkedList();

        // Test 1: Insert at head and tail
        list.insertAtHead(3);
        list.insertAtTail(5);
        list.insertAtTail(7);
        System.out.println("Test1: " + list + " | size=" + list.size());

        // Test 2: Insert at index
        boolean insIdx = list.insertAtIndex(1, 4); // [3,4,5,7]
        System.out.println("Test2: insertAtIndex(1,4)=" + insIdx + " -> " + list + " | size=" + list.size());

        // Test 3: Search
        System.out.println("Test3: indexOf(5)=" + list.indexOf(5) + ", indexOf(9)=" + list.indexOf(9));

        // Test 4: Delete by value
        boolean delVal1 = list.deleteByValue(4); // remove 4
        boolean delVal2 = list.deleteByValue(42); // not present
        System.out.println("Test4: deleteByValue(4)=" + delVal1 + ", deleteByValue(42)=" + delVal2 + " -> " + list + " | size=" + list.size());

        // Test 5: Delete at index
        boolean delIdx = list.deleteAtIndex(1); // remove element at index 1
        System.out.println("Test5: deleteAtIndex(1)=" + delIdx + " -> " + list + " | size=" + list.size());
    }
}