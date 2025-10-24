
import java.util.EmptyStackException;

public class Task148 {
    private static class Stack {
        private Node top;
        private int size;
        
        private static class Node {
            int data;
            Node next;
            
            Node(int data) {
                this.data = data;
                this.next = null;
            }
        }
        
        public Stack() {
            this.top = null;
            this.size = 0;
        }
        
        public void push(int data) {
            Node newNode = new Node(data);
            newNode.next = top;
            top = newNode;
            size++;
        }
        
        public int pop() {
            if (isEmpty()) {
                throw new EmptyStackException();
            }
            int data = top.data;
            top = top.next;
            size--;
            return data;
        }
        
        public int peek() {
            if (isEmpty()) {
                throw new EmptyStackException();
            }
            return top.data;
        }
        
        public boolean isEmpty() {
            return top == null;
        }
        
        public int getSize() {
            return size;
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Basic push and pop operations
        System.out.println("Test Case 1: Basic push and pop");
        Stack stack1 = new Stack();
        stack1.push(10);
        stack1.push(20);
        stack1.push(30);
        System.out.println("Peek: " + stack1.peek());
        System.out.println("Pop: " + stack1.pop());
        System.out.println("Pop: " + stack1.pop());
        System.out.println("Size: " + stack1.getSize());
        System.out.println();
        
        // Test Case 2: Push multiple elements and peek
        System.out.println("Test Case 2: Multiple pushes and peek");
        Stack stack2 = new Stack();
        for (int i = 1; i <= 5; i++) {
            stack2.push(i * 10);
        }
        System.out.println("Peek: " + stack2.peek());
        System.out.println("Size: " + stack2.getSize());
        System.out.println();
        
        // Test Case 3: Pop all elements
        System.out.println("Test Case 3: Pop all elements");
        Stack stack3 = new Stack();
        stack3.push(100);
        stack3.push(200);
        stack3.push(300);
        while (!stack3.isEmpty()) {
            System.out.println("Popped: " + stack3.pop());
        }
        System.out.println("Is empty: " + stack3.isEmpty());
        System.out.println();
        
        // Test Case 4: Mixed operations
        System.out.println("Test Case 4: Mixed operations");
        Stack stack4 = new Stack();
        stack4.push(5);
        stack4.push(15);
        System.out.println("Peek: " + stack4.peek());
        stack4.pop();
        stack4.push(25);
        stack4.push(35);
        System.out.println("Peek: " + stack4.peek());
        System.out.println("Size: " + stack4.getSize());
        System.out.println();
        
        // Test Case 5: Empty stack exception handling
        System.out.println("Test Case 5: Empty stack exception");
        Stack stack5 = new Stack();
        try {
            stack5.pop();
        } catch (EmptyStackException e) {
            System.out.println("Exception caught: Cannot pop from empty stack");
        }
        try {
            stack5.peek();
        } catch (EmptyStackException e) {
            System.out.println("Exception caught: Cannot peek empty stack");
        }
    }
}
