import java.util.LinkedList;
import java.util.NoSuchElementException;

public class Task149 {
    private LinkedList<Integer> queue;

    // Constructor
    public Task149() {
        queue = new LinkedList<>();
    }

    // Enqueue: Add an element to the end of the queue
    public void enqueue(int item) {
        queue.addLast(item);
    }

    // Dequeue: Remove and return the element from the front of the queue
    public int dequeue() {
        if (isEmpty()) {
            throw new NoSuchElementException("Queue is empty");
        }
        return queue.removeFirst();
    }

    // Peek: Return the element at the front of the queue without removing it
    public int peek() {
        if (isEmpty()) {
            throw new NoSuchElementException("Queue is empty");
        }
        return queue.getFirst();
    }

    // Check if the queue is empty
    public boolean isEmpty() {
        return queue.isEmpty();
    }

    public static void main(String[] args) {
        Task149 q = new Task149();

        System.out.println("--- Test Case 1: Enqueue and Peek ---");
        q.enqueue(10);
        q.enqueue(20);
        q.enqueue(30);
        System.out.println("Peek: " + q.peek()); // Expected: 10

        System.out.println("\n--- Test Case 2: Dequeue and Peek ---");
        System.out.println("Dequeued: " + q.dequeue()); // Expected: 10
        System.out.println("Peek: " + q.peek()); // Expected: 20

        System.out.println("\n--- Test Case 3: Dequeue multiple ---");
        System.out.println("Dequeued: " + q.dequeue()); // Expected: 20
        System.out.println("Dequeued: " + q.dequeue()); // Expected: 30

        System.out.println("\n--- Test Case 4: Operations on empty queue ---");
        try {
            System.out.println("Peek: " + q.peek());
        } catch (NoSuchElementException e) {
            System.out.println("Peek error: " + e.getMessage()); // Expected
        }
        try {
            System.out.println("Dequeued: " + q.dequeue());
        } catch (NoSuchElementException e) {
            System.out.println("Dequeue error: " + e.getMessage()); // Expected
        }

        System.out.println("\n--- Test Case 5: Enqueue after empty ---");
        q.enqueue(40);
        q.enqueue(50);
        System.out.println("Peek: " + q.peek()); // Expected: 40
        System.out.println("Dequeued: " + q.dequeue()); // Expected: 40
        System.out.println("Peek: " + q.peek()); // Expected: 50
    }
}