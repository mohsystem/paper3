import java.util.NoSuchElementException;

public class Task149 {

    /**
     * A simple Queue implementation using a linked list for integer data.
     */
    static class Queue {
        private static class Node {
            int data;
            Node next;

            Node(int data) {
                this.data = data;
                this.next = null;
            }
        }

        private Node front;
        private Node rear;

        /**
         * Constructs an empty queue.
         */
        public Queue() {
            this.front = null;
            this.rear = null;
        }

        /**
         * Checks if the queue is empty.
         * @return true if the queue is empty, false otherwise.
         */
        public boolean isEmpty() {
            return front == null;
        }

        /**
         * Adds an element to the rear of the queue.
         * @param data The integer to be added.
         */
        public void enqueue(int data) {
            Node newNode = new Node(data);
            if (isEmpty()) {
                front = newNode;
                rear = newNode;
            } else {
                rear.next = newNode;
                rear = newNode;
            }
        }

        /**
         * Removes and returns the element at the front of the queue.
         * @return The element at the front of the queue.
         * @throws NoSuchElementException if the queue is empty.
         */
        public int dequeue() {
            if (isEmpty()) {
                throw new NoSuchElementException("Queue is empty. Cannot dequeue.");
            }
            int data = front.data;
            front = front.next;
            if (front == null) {
                rear = null; // The queue is now empty
            }
            return data;
        }

        /**
         * Returns the element at the front of the queue without removing it.
         * @return The element at the front of the queue.
         * @throws NoSuchElementException if the queue is empty.
         */
        public int peek() {
            if (isEmpty()) {
                throw new NoSuchElementException("Queue is empty. Cannot peek.");
            }
            return front.data;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Queue Test Cases ---");

        // Test Case 1: Basic enqueue and dequeue
        System.out.println("\n--- Test Case 1: Basic Operations ---");
        Queue q1 = new Queue();
        q1.enqueue(10);
        q1.enqueue(20);
        System.out.println("Dequeued: " + q1.dequeue()); // Expected: 10
        System.out.println("Peek: " + q1.peek());       // Expected: 20
        System.out.println("Dequeued: " + q1.dequeue()); // Expected: 20
        System.out.println("Is empty: " + q1.isEmpty()); // Expected: true

        // Test Case 2: Dequeue from an empty queue
        System.out.println("\n--- Test Case 2: Dequeue from Empty Queue ---");
        Queue q2 = new Queue();
        try {
            q2.dequeue();
        } catch (NoSuchElementException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }

        // Test Case 3: Peek at an empty queue
        System.out.println("\n--- Test Case 3: Peek at Empty Queue ---");
        try {
            q2.peek();
        } catch (NoSuchElementException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }

        // Test Case 4: Enqueue after emptying the queue
        System.out.println("\n--- Test Case 4: Enqueue after Emptying ---");
        Queue q4 = new Queue();
        q4.enqueue(1);
        q4.enqueue(2);
        q4.dequeue();
        q4.dequeue();
        q4.enqueue(3);
        System.out.println("Peek: " + q4.peek()); // Expected: 3
        System.out.println("Is empty: " + q4.isEmpty()); // Expected: false

        // Test Case 5: Multiple operations
        System.out.println("\n--- Test Case 5: Multiple Operations ---");
        Queue q5 = new Queue();
        q5.enqueue(100);
        q5.enqueue(200);
        q5.enqueue(300);
        System.out.println("Dequeued: " + q5.dequeue()); // Expected: 100
        q5.enqueue(400);
        System.out.println("Peek: " + q5.peek());       // Expected: 200
        System.out.println("Dequeued: " + q5.dequeue()); // Expected: 200
        System.out.println("Dequeued: " + q5.dequeue()); // Expected: 300
        System.out.println("Peek: " + q5.peek());       // Expected: 400
    }
}