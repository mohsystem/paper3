
import java.util.Objects;

public class Task150 {
    private static class Node {
        private final int value;
        private Node next;

        public Node(int value) {
            this.value = value;
            this.next = null;
        }

        public int getValue() {
            return value;
        }

        public Node getNext() {
            return next;
        }

        public void setNext(Node next) {
            this.next = next;
        }
    }

    private static class SinglyLinkedList {
        private Node head;
        private int size;
        private static final int MAX_SIZE = 100000;

        public SinglyLinkedList() {
            this.head = null;
            this.size = 0;
        }

        public boolean insert(int value) {
            if (size >= MAX_SIZE) {
                return false;
            }
            try {
                Node newNode = new Node(value);
                if (head == null) {
                    head = newNode;
                } else {
                    Node current = head;
                    while (current.getNext() != null) {
                        current = current.getNext();
                    }
                    current.setNext(newNode);
                }
                size++;
                return true;
            } catch (OutOfMemoryError e) {
                return false;
            }
        }

        public boolean delete(int value) {
            if (head == null) {
                return false;
            }

            if (head.getValue() == value) {
                head = head.getNext();
                size--;
                return true;
            }

            Node current = head;
            while (current.getNext() != null) {
                if (current.getNext().getValue() == value) {
                    current.setNext(current.getNext().getNext());
                    size--;
                    return true;
                }
                current = current.getNext();
            }
            return false;
        }

        public boolean search(int value) {
            Node current = head;
            while (current != null) {
                if (current.getValue() == value) {
                    return true;
                }
                current = current.getNext();
            }
            return false;
        }

        public int getSize() {
            return size;
        }
    }

    public static void main(String[] args) {
        try {
            // Test case 1: Basic insert and search
            SinglyLinkedList list1 = new SinglyLinkedList();
            list1.insert(10);
            list1.insert(20);
            list1.insert(30);
            System.out.println("Test 1 - Search 20: " + list1.search(20));
            System.out.println("Test 1 - Search 40: " + list1.search(40));

            // Test case 2: Delete operation
            SinglyLinkedList list2 = new SinglyLinkedList();
            list2.insert(5);
            list2.insert(15);
            list2.insert(25);
            System.out.println("Test 2 - Delete 15: " + list2.delete(15));
            System.out.println("Test 2 - Search 15: " + list2.search(15));

            // Test case 3: Delete head
            SinglyLinkedList list3 = new SinglyLinkedList();
            list3.insert(100);
            list3.insert(200);
            System.out.println("Test 3 - Delete 100: " + list3.delete(100));
            System.out.println("Test 3 - Search 100: " + list3.search(100));

            // Test case 4: Empty list operations
            SinglyLinkedList list4 = new SinglyLinkedList();
            System.out.println("Test 4 - Search in empty: " + list4.search(50));
            System.out.println("Test 4 - Delete from empty: " + list4.delete(50));

            // Test case 5: Multiple operations
            SinglyLinkedList list5 = new SinglyLinkedList();
            list5.insert(1);
            list5.insert(2);
            list5.insert(3);
            list5.delete(2);
            list5.insert(4);
            System.out.println("Test 5 - Search 2: " + list5.search(2));
            System.out.println("Test 5 - Search 4: " + list5.search(4));
            System.out.println("Test 5 - Size: " + list5.getSize());
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
