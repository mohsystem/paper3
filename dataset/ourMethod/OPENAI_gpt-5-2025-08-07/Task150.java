import java.util.Arrays;

public final class Task150 {
    public static void main(String[] args) {
        SinglyLinkedList list = new SinglyLinkedList();

        // Test 1: Insert at tail and basic search/delete
        list.insertAtTail(1);
        list.insertAtTail(2);
        list.insertAtTail(3);
        System.out.println("Test1 list: " + list);
        int idx2 = list.search(2);
        System.out.println("Test1 search(2): " + idx2);
        boolean del2 = list.deleteValue(2);
        System.out.println("Test1 deleteValue(2): " + del2 + " => " + list + " search(2): " + list.search(2));

        // Test 2: Insert at head
        boolean headOk = list.insertAtHead(10);
        System.out.println("Test2 insertAtHead(10): " + headOk + " => " + list);

        // Test 3: Insert at index valid and invalid
        boolean insIdxOk = list.insertAtIndex(1, 99);
        boolean insIdxBad = list.insertAtIndex(list.size() + 2, 5);
        System.out.println("Test3 insertAtIndex(1,99): " + insIdxOk + " => " + list);
        System.out.println("Test3 insertAtIndex(size+2,5): " + insIdxBad + " => " + list);

        // Test 4: Delete at index valid and invalid
        Integer rem0 = list.deleteAtIndex(0);
        Integer remBad = list.deleteAtIndex(list.size()); // out of bounds
        System.out.println("Test4 deleteAtIndex(0): " + rem0 + " => " + list);
        System.out.println("Test4 deleteAtIndex(size): " + remBad + " => " + list);

        // Test 5: Search non-existing
        System.out.println("Test5 search(100): " + list.search(100));
    }

    static final class SinglyLinkedList {
        private static final class Node {
            final int value;
            Node next;
            Node(int value) { this.value = value; }
        }

        private Node head;
        private int size;

        public int size() {
            return size;
        }

        public boolean insertAtHead(int value) {
            Node n = new Node(value);
            n.next = head;
            head = n;
            size++;
            return true;
        }

        public boolean insertAtTail(int value) {
            Node n = new Node(value);
            if (head == null) {
                head = n;
                size = 1;
                return true;
            }
            Node cur = head;
            while (cur.next != null) {
                cur = cur.next;
            }
            cur.next = n;
            size++;
            return true;
        }

        public boolean insertAtIndex(int index, int value) {
            if (index < 0 || index > size) {
                return false;
            }
            if (index == 0) {
                return insertAtHead(value);
            }
            Node prev = head;
            for (int i = 0; i < index - 1; i++) {
                prev = prev.next;
            }
            Node n = new Node(value);
            n.next = prev.next;
            prev.next = n;
            size++;
            return true;
        }

        public boolean deleteValue(int value) {
            Node prev = null;
            Node cur = head;
            while (cur != null && cur.value != value) {
                prev = cur;
                cur = cur.next;
            }
            if (cur == null) {
                return false;
            }
            if (prev == null) {
                head = cur.next;
            } else {
                prev.next = cur.next;
            }
            size--;
            return true;
        }

        public Integer deleteAtIndex(int index) {
            if (index < 0 || index >= size) {
                return null;
            }
            Node prev = null;
            Node cur = head;
            for (int i = 0; i < index; i++) {
                prev = cur;
                cur = cur.next;
            }
            int val = cur.value;
            if (prev == null) {
                head = cur.next;
            } else {
                prev.next = cur.next;
            }
            size--;
            return val;
        }

        public int search(int value) {
            Node cur = head;
            int idx = 0;
            while (cur != null) {
                if (cur.value == value) return idx;
                idx++;
                cur = cur.next;
            }
            return -1;
        }

        public int[] toArray() {
            int[] arr = new int[size];
            Node cur = head;
            int i = 0;
            while (cur != null) {
                arr[i++] = cur.value;
                cur = cur.next;
            }
            return arr;
        }

        @Override
        public String toString() {
            return Arrays.toString(toArray());
        }
    }
}