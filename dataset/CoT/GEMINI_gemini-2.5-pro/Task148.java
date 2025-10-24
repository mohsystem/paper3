public class Task148 {

    // Stack implementation class
    static class Stack {
        private int maxSize;
        private int[] stackArray;
        private int top;

        // Constructor
        public Stack(int size) {
            maxSize = size;
            stackArray = new int[maxSize];
            top = -1; // Stack is initially empty
        }

        // Push an item onto the top of the stack
        public void push(int value) {
            if (isFull()) {
                System.err.println("Error: Stack overflow. Cannot push " + value);
                return;
            }
            stackArray[++top] = value;
            System.out.println("Pushed " + value + " to stack.");
        }

        // Pop an item from the top of the stack
        public int pop() {
            if (isEmpty()) {
                System.err.println("Error: Stack underflow. Cannot pop.");
                return -1; // Return a sentinel value for error
            }
            int value = stackArray[top--];
            System.out.println("Popped " + value + " from stack.");
            return value;
        }

        // Peek at the top item of the stack without removing it
        public int peek() {
            if (isEmpty()) {
                System.err.println("Error: Stack is empty. Cannot peek.");
                return -1; // Return a sentinel value for error
            }
            return stackArray[top];
        }

        // Check if the stack is empty
        public boolean isEmpty() {
            return (top == -1);
        }

        // Check if the stack is full
        public boolean isFull() {
            return (top == maxSize - 1);
        }
    }

    public static void main(String[] args) {
        System.out.println("Initializing a stack with capacity 5.");
        Stack stack = new Stack(5);

        // Test Case 1: Push items and peek
        System.out.println("\n--- Test Case 1: Push and Peek ---");
        stack.push(10);
        stack.push(20);
        stack.push(30);
        System.out.println("Top element is (peek): " + stack.peek());
        
        // Test Case 2: Pop all items
        System.out.println("\n--- Test Case 2: Pop All Items ---");
        stack.pop();
        stack.pop();
        stack.pop();

        // Test Case 3: Pop from an empty stack (underflow)
        System.out.println("\n--- Test Case 3: Pop from Empty Stack ---");
        System.out.println("Is stack empty? " + stack.isEmpty());
        stack.pop(); // This should cause an underflow error

        // Test Case 4: Push until the stack is full
        System.out.println("\n--- Test Case 4: Fill the Stack ---");
        stack.push(11);
        stack.push(22);
        stack.push(33);
        stack.push(44);
        stack.push(55);
        System.out.println("Is stack full? " + stack.isFull());

        // Test Case 5: Push to a full stack (overflow)
        System.out.println("\n--- Test Case 5: Push to Full Stack ---");
        stack.push(66); // This should cause an overflow error
        System.out.println("Top element is (peek): " + stack.peek());
    }
}