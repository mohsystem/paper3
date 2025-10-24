import java.util.ArrayList;
import java.util.EmptyStackException;

public class Task148 {

    /**
     * A secure Stack implementation for integers.
     * It uses ArrayList, which dynamically resizes, preventing overflow unless
     * out of memory. It explicitly checks for underflow on pop and peek.
     */
    static class Stack {
        private final ArrayList<Integer> stackList;

        /**
         * Constructs an empty stack.
         */
        public Stack() {
            stackList = new ArrayList<>();
        }

        /**
         * Pushes an element onto the top of the stack.
         * @param item The integer element to be pushed onto the stack.
         */
        public void push(int item) {
            stackList.add(item);
        }

        /**
         * Removes and returns the element at the top of the stack.
         * @return The element at the top of the stack.
         * @throws EmptyStackException if the stack is empty.
         */
        public int pop() {
            if (isEmpty()) {
                // Secure: Prevent stack underflow.
                throw new EmptyStackException();
            }
            return stackList.remove(stackList.size() - 1);
        }

        /**
         * Returns the element at the top of the stack without removing it.
         * @return The element at the top of the stack.
         * @throws EmptyStackException if the stack is empty.
         */
        public int peek() {
            if (isEmpty()) {
                // Secure: Prevent stack underflow.
                throw new EmptyStackException();
            }
            return stackList.get(stackList.size() - 1);
        }

        /**
         * Tests if this stack is empty.
         * @return true if and only if this stack contains no items; false otherwise.
         */
        public boolean isEmpty() {
            return stackList.isEmpty();
        }
    }

    /**
     * Main method with test cases for the Stack implementation.
     */
    public static void main(String[] args) {
        System.out.println("Java Stack Test Cases:");
        
        // Test Case 1: Basic push and pop
        System.out.println("\n--- Test Case 1: Basic Operations ---");
        Stack s1 = new Stack();
        s1.push(10);
        s1.push(20);
        System.out.println("Popped: " + s1.pop()); // Should be 20
        System.out.println("Peeked: " + s1.peek()); // Should be 10
        System.out.println("Popped: " + s1.pop()); // Should be 10
        
        // Test Case 2: Popping from an empty stack
        System.out.println("\n--- Test Case 2: Pop from Empty Stack ---");
        Stack s2 = new Stack();
        try {
            s2.pop();
        } catch (EmptyStackException e) {
            System.out.println("Caught expected exception: Stack is empty.");
        }

        // Test Case 3: Peeking from an empty stack
        System.out.println("\n--- Test Case 3: Peek from Empty Stack ---");
        Stack s3 = new Stack();
        try {
            s3.peek();
        } catch (EmptyStackException e) {
            System.out.println("Caught expected exception: Stack is empty.");
        }

        // Test Case 4: A sequence of operations
        System.out.println("\n--- Test Case 4: Sequence of Operations ---");
        Stack s4 = new Stack();
        s4.push(1);
        s4.push(2);
        s4.push(3);
        System.out.println("Popped: " + s4.pop()); // 3
        s4.push(4);
        System.out.println("Peeked: " + s4.peek()); // 4
        System.out.println("Popped: " + s4.pop()); // 4
        System.out.println("Popped: " + s4.pop()); // 2

        // Test Case 5: Check isEmpty
        System.out.println("\n--- Test Case 5: isEmpty Check ---");
        Stack s5 = new Stack();
        System.out.println("Is stack empty? " + s5.isEmpty()); // true
        s5.push(100);
        System.out.println("Is stack empty? " + s5.isEmpty()); // false
        s5.pop();
        System.out.println("Is stack empty? " + s5.isEmpty()); // true
    }
}