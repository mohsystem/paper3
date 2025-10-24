import java.util.*;

public class Task149 {
    static class IntQueue {
        private final int[] data;
        private int head = 0;
        private int tail = 0;
        private int size = 0;
        private final int capacity;

        public IntQueue(int capacity) {
            this.capacity = capacity;
            this.data = new int[capacity];
        }

        public boolean enqueue(int x) {
            if (size == capacity) return false;
            data[tail] = x;
            tail = (tail + 1) % capacity;
            size++;
            return true;
        }

        public Integer dequeue() {
            if (size == 0) return null;
            int val = data[head];
            head = (head + 1) % capacity;
            size--;
            return val;
        }

        public Integer peek() {
            if (size == 0) return null;
            return data[head];
        }

        public boolean isEmpty() {
            return size == 0;
        }

        public boolean isFull() {
            return size == capacity;
        }
    }

    public static void main(String[] args) {
        // Test 1: Basic enqueue, peek, and dequeue
        IntQueue q1 = new IntQueue(5);
        q1.enqueue(1);
        q1.enqueue(2);
        q1.enqueue(3);
        System.out.println("Test1 Peek: " + q1.peek());         // Expect 1
        System.out.println("Test1 Dequeue: " + q1.dequeue());   // Expect 1
        System.out.println("Test1 Peek after dequeue: " + q1.peek()); // Expect 2

        // Test 2: Peek on empty queue
        IntQueue q2 = new IntQueue(3);
        System.out.println("Test2 Peek on empty: " + q2.peek()); // Expect null

        // Test 3: Dequeue on empty queue
        IntQueue q3 = new IntQueue(2);
        System.out.println("Test3 Dequeue on empty: " + q3.dequeue()); // Expect null

        // Test 4: Fill to capacity and attempt extra enqueue
        IntQueue q4 = new IntQueue(3);
        System.out.println("Test4 Enqueue 1: " + q4.enqueue(1)); // true
        System.out.println("Test4 Enqueue 2: " + q4.enqueue(2)); // true
        System.out.println("Test4 Enqueue 3: " + q4.enqueue(3)); // true
        System.out.println("Test4 Enqueue 4 (should fail): " + q4.enqueue(4)); // false

        // Test 5: Wrap-around behavior
        IntQueue q5 = new IntQueue(3);
        q5.enqueue(10);
        q5.enqueue(20);
        q5.enqueue(30);
        System.out.println("Test5 Dequeue: " + q5.dequeue()); // 10
        System.out.println("Test5 Dequeue: " + q5.dequeue()); // 20
        System.out.println("Test5 Enqueue 40: " + q5.enqueue(40)); // true
        System.out.println("Test5 Enqueue 50: " + q5.enqueue(50)); // true
        System.out.println("Test5 Dequeue: " + q5.dequeue()); // 30
        System.out.println("Test5 Dequeue: " + q5.dequeue()); // 40
        System.out.println("Test5 Dequeue: " + q5.dequeue()); // 50
        System.out.println("Test5 Dequeue on empty: " + q5.dequeue()); // null
    }
}