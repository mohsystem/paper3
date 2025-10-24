import java.util.NoSuchElementException;

public class Task149 {

    // Node class for the linked list
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
    private int size;

    public Task149() {
        this.front = null;
        this.rear = null;
        this.size = 0;
    }

    /**
     * Checks if the queue is empty.
     * @return true if the queue is empty, false otherwise.
     */
    public boolean isEmpty() {
        return size == 0;
    }

    /**
     * Adds an element to the rear of the queue.
     * @param data The integer to add to the queue.
     */
    public void enqueue(int data) {
        Node newNode = new Node(data);
        if (isEmpty()) {
            front = newNode;
        } else {
            rear.next = newNode;
        }
        rear = newNode;
        size++;
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
            rear = null;
        }
        size--;
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

    public static void main(String[] args) {
        Task149 queue = new Task149();

        // Test Case 1: Enqueue elements
        System.out.println("Test Case 1: Enqueue 10, 20, 30");
        queue.enqueue(10);
        queue.enqueue(20);
        queue.enqueue(30);
        System.out.println("Enqueue successful.");
        System.out.println("--------------------");

        // Test Case 2: Peek at the front element
        System.out.println("Test Case 2: Peek front element");
        System.out.println("Front element is: " + queue.peek()); // Expected: 10
        System.out.println("--------------------");

        // Test Case 3: Dequeue an element
        System.out.println("Test Case 3: Dequeue an element");
        System.out.println("Dequeued element: " + queue.dequeue()); // Expected: 10
        System.out.println("Front element after dequeue is: " + queue.peek()); // Expected: 20
        System.out.println("--------------------");

        // Test Case 4: Dequeue all elements
        System.out.println("Test Case 4: Dequeue all elements");
        System.out.println("Dequeued: " + queue.dequeue()); // Expected: 20
        System.out.println("Dequeued: " + queue.dequeue()); // Expected: 30
        System.out.println("Is queue empty? " + queue.isEmpty()); // Expected: true
        System.out.println("--------------------");

        // Test Case 5: Operations on an empty queue (handle exceptions)
        System.out.println("Test Case 5: Operations on empty queue");
        try {
            System.out.println("Attempting to peek...");
            queue.peek();
        } catch (NoSuchElementException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
        try {
            System.out.println("Attempting to dequeue...");
            queue.dequeue();
        } catch (NoSuchElementException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
        System.out.println("--------------------");
    }
}