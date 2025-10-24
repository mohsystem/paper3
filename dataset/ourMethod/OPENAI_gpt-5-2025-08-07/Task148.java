import java.util.*;

public class Task148 {
    static final class IntStack {
        private final int[] data;
        private int top;

        public IntStack(int capacity) {
            if (capacity <= 0 || capacity > 1_000_000) {
                throw new IllegalArgumentException("Invalid capacity");
            }
            this.data = new int[capacity];
            this.top = 0;
        }

        public boolean push(int value) {
            if (top >= data.length) {
                return false;
            }
            data[top++] = value;
            return true;
        }

        public Integer pop() {
            if (top == 0) {
                return null;
            }
            return data[--top];
        }

        public Integer peek() {
            if (top == 0) {
                return null;
            }
            return data[top - 1];
        }

        public int size() {
            return top;
        }

        public boolean isEmpty() {
            return top == 0;
        }

        public int capacity() {
            return data.length;
        }
    }

    public static void main(String[] args) {
        // Test 1: Basic push/pop/peek
        IntStack s1 = new IntStack(3);
        System.out.println("T1 push 10: " + s1.push(10));
        System.out.println("T1 push 20: " + s1.push(20));
        System.out.println("T1 peek: " + s1.peek());
        System.out.println("T1 pop: " + s1.pop());
        System.out.println("T1 pop: " + s1.pop());
        System.out.println("T1 pop empty: " + s1.pop());

        // Test 2: Overflow behavior
        IntStack s2 = new IntStack(1);
        System.out.println("T2 push 5: " + s2.push(5));
        System.out.println("T2 push 6 overflow: " + s2.push(6));
        System.out.println("T2 peek: " + s2.peek());
        System.out.println("T2 size: " + s2.size());

        // Test 3: Empty stack operations
        IntStack s3 = new IntStack(2);
        System.out.println("T3 empty peek: " + s3.peek());
        System.out.println("T3 empty pop: " + s3.pop());
        System.out.println("T3 isEmpty: " + s3.isEmpty());

        // Test 4: Fill and partial pop
        IntStack s4 = new IntStack(4);
        System.out.println("T4 push 1: " + s4.push(1));
        System.out.println("T4 push 2: " + s4.push(2));
        System.out.println("T4 push 3: " + s4.push(3));
        System.out.println("T4 size: " + s4.size());
        System.out.println("T4 pop: " + s4.pop());
        System.out.println("T4 peek: " + s4.peek());

        // Test 5: Mixed operations
        IntStack s5 = new IntStack(5);
        System.out.println("T5 push 7: " + s5.push(7));
        System.out.println("T5 push 8: " + s5.push(8));
        System.out.println("T5 push 9: " + s5.push(9));
        System.out.println("T5 peek: " + s5.peek());
        System.out.println("T5 pop: " + s5.pop());
        System.out.println("T5 push 10: " + s5.push(10));
        System.out.println("T5 size: " + s5.size());
    }
}