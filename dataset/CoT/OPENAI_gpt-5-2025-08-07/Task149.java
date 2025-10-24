import java.util.NoSuchElementException;

public class Task149 {
    // Secure linked-list based queue for Integer values
    private static final class Node {
        final Integer value;
        Node next;
        Node(Integer v) { this.value = v; }
    }

    public static final class SafeQueue {
        private Node head;
        private Node tail;
        private int size;

        // Security: prevent integer overflow on size
        private boolean canIncrementSize() {
            return size < Integer.MAX_VALUE;
        }

        public SafeQueue() {
            this.head = null;
            this.tail = null;
            this.size = 0;
        }

        // Enqueue operation: returns true on success, false if null input or size overflow
        public boolean enqueue(Integer value) {
            if (value == null) {
                return false; // disallow nulls to keep semantics clear
            }
            if (!canIncrementSize()) {
                return false; // prevent overflow
            }
            Node n = new Node(value);
            if (tail == null) {
                head = tail = n;
            } else {
                tail.next = n;
                tail = n;
            }
            size++;
            return true;
        }

        // Dequeue operation: returns value or null if empty
        public Integer dequeue() {
            if (head == null) {
                return null;
            }
            Integer v = head.value;
            Node old = head;
            head = head.next;
            // Help GC
            old.next = null;
            if (head == null) {
                tail = null;
            }
            size--;
            return v;
        }

        // Peek operation: returns value or null if empty
        public Integer peek() {
            return head == null ? null : head.value;
        }

        public boolean isEmpty() {
            return head == null;
        }

        public int size() {
            return size;
        }
    }

    // Demonstration with 5 test cases
    public static void main(String[] args) {
        // Test 1: Basic enqueue, peek, dequeue
        SafeQueue q1 = new SafeQueue();
        q1.enqueue(1);
        q1.enqueue(2);
        q1.enqueue(3);
        System.out.println("Test1 Peek (expect 1): " + q1.peek());
        System.out.println("Test1 Dequeue (expect 1): " + q1.dequeue());
        System.out.println("Test1 Peek (expect 2): " + q1.peek());
        System.out.println("Test1 Size (expect 2): " + q1.size());

        // Test 2: Operations on empty queue
        SafeQueue q2 = new SafeQueue();
        System.out.println("Test2 Dequeue empty (expect null): " + q2.dequeue());
        System.out.println("Test2 Peek empty (expect null): " + q2.peek());
        System.out.println("Test2 Size (expect 0): " + q2.size());

        // Test 3: Mixed operations
        SafeQueue q3 = new SafeQueue();
        q3.enqueue(10);
        System.out.println("Test3 Dequeue (expect 10): " + q3.dequeue());
        q3.enqueue(20);
        q3.enqueue(30);
        System.out.println("Test3 Peek (expect 20): " + q3.peek());
        System.out.println("Test3 Size (expect 2): " + q3.size());

        // Test 4: Negative and zero values
        SafeQueue q4 = new SafeQueue();
        q4.enqueue(-5);
        q4.enqueue(0);
        System.out.println("Test4 Dequeue (expect -5): " + q4.dequeue());
        System.out.println("Test4 Peek (expect 0): " + q4.peek());
        System.out.println("Test4 Size (expect 1): " + q4.size());

        // Test 5: FIFO property with sequence
        SafeQueue q5 = new SafeQueue();
        for (int i = 100; i < 105; i++) q5.enqueue(i);
        System.out.println("Test5 Dequeue1 (expect 100): " + q5.dequeue());
        System.out.println("Test5 Dequeue2 (expect 101): " + q5.dequeue());
        System.out.println("Test5 Dequeue3 (expect 102): " + q5.dequeue());
        System.out.println("Test5 Peek (expect 103): " + q5.peek());
        System.out.println("Test5 Size (expect 2): " + q5.size());
    }
}