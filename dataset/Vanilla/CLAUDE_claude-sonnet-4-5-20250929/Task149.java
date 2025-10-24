
import java.util.ArrayList;
import java.util.List;

class Task149 {
    private List<Integer> queue;
    
    public Task149() {
        queue = new ArrayList<>();
    }
    
    public void enqueue(int value) {
        queue.add(value);
    }
    
    public Integer dequeue() {
        if (queue.isEmpty()) {
            return null;
        }
        return queue.remove(0);
    }
    
    public Integer peek() {
        if (queue.isEmpty()) {
            return null;
        }
        return queue.get(0);
    }
    
    public boolean isEmpty() {
        return queue.isEmpty();
    }
    
    public int size() {
        return queue.size();
    }
    
    public static void main(String[] args) {
        // Test Case 1: Basic enqueue and dequeue
        System.out.println("Test Case 1: Basic enqueue and dequeue");
        Task149 q1 = new Task149();
        q1.enqueue(10);
        q1.enqueue(20);
        q1.enqueue(30);
        System.out.println("Dequeue: " + q1.dequeue()); // 10
        System.out.println("Dequeue: " + q1.dequeue()); // 20
        System.out.println();
        
        // Test Case 2: Peek operation
        System.out.println("Test Case 2: Peek operation");
        Task149 q2 = new Task149();
        q2.enqueue(5);
        q2.enqueue(15);
        System.out.println("Peek: " + q2.peek()); // 5
        System.out.println("Peek: " + q2.peek()); // 5 (still 5)
        System.out.println("Dequeue: " + q2.dequeue()); // 5
        System.out.println("Peek: " + q2.peek()); // 15
        System.out.println();
        
        // Test Case 3: Empty queue operations
        System.out.println("Test Case 3: Empty queue operations");
        Task149 q3 = new Task149();
        System.out.println("Dequeue from empty: " + q3.dequeue()); // null
        System.out.println("Peek from empty: " + q3.peek()); // null
        q3.enqueue(100);
        System.out.println("Dequeue: " + q3.dequeue()); // 100
        System.out.println("Dequeue from empty: " + q3.dequeue()); // null
        System.out.println();
        
        // Test Case 4: Multiple operations
        System.out.println("Test Case 4: Multiple operations");
        Task149 q4 = new Task149();
        q4.enqueue(1);
        q4.enqueue(2);
        System.out.println("Dequeue: " + q4.dequeue()); // 1
        q4.enqueue(3);
        q4.enqueue(4);
        System.out.println("Peek: " + q4.peek()); // 2
        System.out.println("Dequeue: " + q4.dequeue()); // 2
        System.out.println("Dequeue: " + q4.dequeue()); // 3
        System.out.println();
        
        // Test Case 5: Size check
        System.out.println("Test Case 5: Size check");
        Task149 q5 = new Task149();
        System.out.println("Size: " + q5.size()); // 0
        q5.enqueue(50);
        q5.enqueue(60);
        q5.enqueue(70);
        System.out.println("Size: " + q5.size()); // 3
        q5.dequeue();
        System.out.println("Size after dequeue: " + q5.size()); // 2
        System.out.println("Is empty: " + q5.isEmpty()); // false
    }
}
