
import java.util.ArrayList;
import java.util.List;
import java.util.NoSuchElementException;

public class Task148 {
    private static class Stack<T> {
        private final List<T> items;
        private final int maxCapacity;
        
        public Stack(int capacity) {
            if (capacity <= 0 || capacity > 100000) {
                throw new IllegalArgumentException("Capacity must be between 1 and 100000");
            }
            this.maxCapacity = capacity;
            this.items = new ArrayList<>(capacity);
        }
        
        public void push(T item) {
            if (item == null) {
                throw new IllegalArgumentException("Cannot push null item");
            }
            if (items.size() >= maxCapacity) {
                throw new IllegalStateException("Stack overflow: capacity exceeded");
            }
            items.add(item);
        }
        
        public T pop() {
            if (items.isEmpty()) {
                throw new NoSuchElementException("Stack underflow: cannot pop from empty stack");
            }
            return items.remove(items.size() - 1);
        }
        
        public T peek() {
            if (items.isEmpty()) {
                throw new NoSuchElementException("Stack is empty: cannot peek");
            }
            return items.get(items.size() - 1);
        }
        
        public boolean isEmpty() {
            return items.isEmpty();
        }
        
        public int size() {
            return items.size();
        }
    }
    
    public static void main(String[] args) {
        try {
            // Test case 1: Basic push, peek, and pop operations
            System.out.println("Test 1: Basic operations");
            Stack<Integer> stack1 = new Stack<>(10);
            stack1.push(10);
            stack1.push(20);
            stack1.push(30);
            System.out.println("Peek: " + stack1.peek());
            System.out.println("Pop: " + stack1.pop());
            System.out.println("Pop: " + stack1.pop());
            System.out.println("Size: " + stack1.size());
            System.out.println();
            
            // Test case 2: String stack
            System.out.println("Test 2: String stack");
            Stack<String> stack2 = new Stack<>(5);
            stack2.push("Hello");
            stack2.push("World");
            System.out.println("Peek: " + stack2.peek());
            System.out.println("Pop: " + stack2.pop());
            System.out.println("Peek: " + stack2.peek());
            System.out.println();
            
            // Test case 3: Empty stack operations
            System.out.println("Test 3: Empty stack check");
            Stack<Double> stack3 = new Stack<>(10);
            System.out.println("Is empty: " + stack3.isEmpty());
            stack3.push(3.14);
            System.out.println("Is empty: " + stack3.isEmpty());
            stack3.pop();
            System.out.println("Is empty after pop: " + stack3.isEmpty());
            System.out.println();
            
            // Test case 4: Capacity limit
            System.out.println("Test 4: Capacity limit");
            Stack<Integer> stack4 = new Stack<>(3);
            stack4.push(1);
            stack4.push(2);
            stack4.push(3);
            try {
                stack4.push(4);
            } catch (IllegalStateException e) {
                System.out.println("Caught overflow: " + e.getMessage());
            }
            System.out.println();
            
            // Test case 5: Exception handling for underflow
            System.out.println("Test 5: Underflow handling");
            Stack<Integer> stack5 = new Stack<>(10);
            stack5.push(100);
            stack5.pop();
            try {
                stack5.pop();
            } catch (NoSuchElementException e) {
                System.out.println("Caught underflow: " + e.getMessage());
            }
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
